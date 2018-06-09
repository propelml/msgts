/*
Copyright 2018 Ryan Dahl <ry@tinyclouds.org>. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

#include "binding.h"

using namespace v8;

// TODO use V8's CHECK.
#define CHECK(x) \
  if (!(x)) {    \
    exit(42);    \
  }

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

static inline v8::Local<v8::String> v8_str(const char* x) {
  return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), x,
                                 v8::NewStringType::kNormal)
      .ToLocalChecked();
}

// Exits the process.
void HandleException(Deno* d, Local<Value> exception) {
  HandleScope handle_scope(d->isolate);
  auto context = d->context.Get(d->isolate);
  Context::Scope context_scope(context);

  auto message = Exception::CreateMessage(d->isolate, exception);
  auto onerrorStr = String::NewFromUtf8(d->isolate, "onerror");
  auto onerror = context->Global()->Get(onerrorStr);

  if (onerror->IsFunction()) {
    Local<Function> func = Local<Function>::Cast(onerror);
    Local<Value> args[5];
    auto origin = message->GetScriptOrigin();
    args[0] = exception->ToString();
    args[1] = message->GetScriptResourceName();
    args[2] = origin.ResourceLineOffset();
    args[3] = origin.ResourceColumnOffset();
    args[4] = exception;
    func->Call(context->Global(), 5, args);
    /* message, source, lineno, colno, error */
  } else {
    String::Utf8Value exceptionStr(d->isolate, exception);
    printf("Unhandled Exception %s\n", ToCString(exceptionStr));
    message->PrintCurrentStackTrace(d->isolate, stdout);
  }

  exit(1);
}

/*
bool AbortOnUncaughtExceptionCallback(Isolate* isolate) {
  return true;
}

void MessageCallback2(Local<Message> message, Local<Value> data) {
  printf("MessageCallback2\n\n");
}

void FatalErrorCallback2(const char* location, const char* message) {
  printf("FatalErrorCallback2\n");
}
*/

void ExitOnPromiseRejectCallback(PromiseRejectMessage promise_reject_message) {
  auto* isolate = Isolate::GetCurrent();
  Deno* d = static_cast<Deno*>(isolate->GetData(0));
  assert(d->isolate == isolate);
  HandleScope handle_scope(d->isolate);
  auto exception = promise_reject_message.GetValue();
  HandleException(d, exception);
}

void Print(const FunctionCallbackInfo<Value>& args) {
  bool first = true;
  auto* isolate = args.GetIsolate();
  for (int i = 0; i < args.Length(); i++) {
    HandleScope handle_scope(isolate);
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    String::Utf8Value str(isolate, args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

// Sets the recv callback.
void Recv(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Deno* d = (Deno*)isolate->GetData(0);
  assert(d->isolate == isolate);

  HandleScope handle_scope(isolate);

  Local<Value> v = args[0];
  assert(v->IsFunction());
  Local<Function> func = Local<Function>::Cast(v);

  d->recv.Reset(isolate, func);
}

// Called from JavaScript, routes message to golang.
void Send(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Deno* d = static_cast<Deno*>(isolate->GetData(0));
  assert(d->isolate == isolate);

  Locker locker(d->isolate);
  EscapableHandleScope handle_scope(isolate);

  Local<Value> v = args[0];
  assert(v->IsArrayBuffer());

  auto ab = Local<ArrayBuffer>::Cast(v);
  auto contents = ab->GetContents();

  void* buf = contents.Data();
  int buflen = static_cast<int>(contents.ByteLength());

  auto retbuf = d->cb(d, DenoBuf{buf, buflen});
  if (retbuf.data) {
    auto ab = ArrayBuffer::New(d->isolate, retbuf.data, retbuf.len,
                               ArrayBufferCreationMode::kInternalized);
    /*
    // I'm slightly worried the above ArrayBuffer construction leaks memory
    // the following might be a safer way to do it.
    auto ab = ArrayBuffer::New(d->isolate, retbuf.len);
    auto contents = ab->GetContents();
    memcpy(contents.Data(), retbuf.data, retbuf.len);
    free(retbuf.data);
    */
    args.GetReturnValue().Set(handle_scope.Escape(ab));
  }
}

intptr_t external_references[] = {reinterpret_cast<intptr_t>(Print),
                                  reinterpret_cast<intptr_t>(Recv),
                                  reinterpret_cast<intptr_t>(Send), 0};

extern "C" {

const char* v8_version() { return V8::GetVersion(); }

void v8_set_flags(int* argc, char** argv) {
  V8::SetFlagsFromCommandLine(argc, argv, true);
}

const char* deno_last_exception(Deno* d) { return d->last_exception.c_str(); }

int deno_load(Deno* d, const char* name_s, const char* source_s) {
  Locker locker(d->isolate);
  Isolate::Scope isolate_scope(d->isolate);
  HandleScope handle_scope(d->isolate);

  auto context = d->context.Get(d->isolate);
  Context::Scope context_scope(context);

  TryCatch try_catch(d->isolate);

  Local<String> name = String::NewFromUtf8(d->isolate, name_s);
  Local<String> source = String::NewFromUtf8(d->isolate, source_s);

  ScriptOrigin origin(name);

  MaybeLocal<Script> script = Script::Compile(context, source, &origin);

  if (script.IsEmpty()) {
    assert(try_catch.HasCaught());
    HandleException(d, try_catch.Exception());
    assert(false);
    return 1;
  }

  MaybeLocal<Value> result = script.ToLocalChecked()->Run(context);

  if (result.IsEmpty()) {
    assert(try_catch.HasCaught());
    HandleException(d, try_catch.Exception());
    assert(false);
    return 2;
  }

  return 0;
}

// Called from golang. Must route message to javascript lang.
// non-zero return value indicates error. check deno_last_exception().
int deno_send(Deno* d, DenoBuf buf) {
  Locker locker(d->isolate);
  Isolate::Scope isolate_scope(d->isolate);
  HandleScope handle_scope(d->isolate);

  auto context = d->context.Get(d->isolate);
  Context::Scope context_scope(context);

  TryCatch try_catch(d->isolate);

  Local<Function> recv = Local<Function>::New(d->isolate, d->recv);
  if (recv.IsEmpty()) {
    d->last_exception = "V8Deno2.recv has not been called.";
    return 1;
  }

  Local<Value> args[1];
  args[0] = ArrayBuffer::New(d->isolate, buf.data, buf.len,
                             ArrayBufferCreationMode::kInternalized);
  assert(!args[0].IsEmpty());
  assert(!try_catch.HasCaught());

  recv->Call(context->Global(), 1, args);

  if (try_catch.HasCaught()) {
    HandleException(d, try_catch.Exception());
    return 2;
  }

  return 0;
}

void v8_init() {
  // V8::InitializeICUDefaultLocation(argv[0]);
  // V8::InitializeExternalStartupData(argv[0]);
  auto p = platform::CreateDefaultPlatform();
  V8::InitializePlatform(p);
  V8::Initialize();
}

Deno* deno_new(void* data, RecvCallback cb) {
  Deno* d = new Deno;
  d->cb = cb;
  d->data = data;
  Isolate::CreateParams params;
  params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate* isolate = Isolate::New(params);
  deno_add_isolate(d, isolate);
  return d;
}

Deno* deno_from_snapshot(v8::StartupData* blob, void* data, RecvCallback cb) {
  Deno* d = new Deno;
  d->cb = cb;
  d->data = data;
  Isolate::CreateParams params;
  params.snapshot_blob = blob;
  params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
  params.external_references = external_references;
  Isolate* isolate = Isolate::New(params);
  deno_add_isolate(d, isolate);

  Isolate::Scope isolate_scope(isolate);
  {
    HandleScope handle_scope(isolate);
    auto context = v8::Context::New(isolate);
    d->context.Reset(d->isolate, context);
  }

  return d;
}

void deno_add_isolate(Deno* d, Isolate* isolate) {
  d->isolate = isolate;
  // Leaving this code here because it will probably be useful later on, but
  // disabling it now as I haven't got tests for the desired behavior.
  // d->isolate->SetCaptureStackTraceForUncaughtExceptions(true);
  // d->isolate->SetAbortOnUncaughtExceptionCallback(AbortOnUncaughtExceptionCallback);
  // d->isolate->AddMessageListener(MessageCallback2);
  // d->isolate->SetFatalErrorHandler(FatalErrorCallback2);
  d->isolate->SetPromiseRejectCallback(ExitOnPromiseRejectCallback);
  d->isolate->SetData(0, d);
}

v8::StartupData SerializeInternalField(v8::Local<v8::Object> holder, int index,
                                       void* data) {
  printf("SerializeInternalField %d\n", index);
  v8::StartupData sd;
  sd.data = "a";
  sd.raw_size = 1;
  return sd;
}

v8::StartupData deno_make_snapshot(const char* js_filename,
                                   const char* js_source) {
  auto creator = new v8::SnapshotCreator(external_references);
  auto* isolate = creator->GetIsolate();

  Deno* d = new Deno;
  deno_add_isolate(d, isolate);

  Isolate::Scope isolate_scope(isolate);
  {
    HandleScope handle_scope(isolate);

    Local<Context> context = Context::New(d->isolate);
    Context::Scope context_scope(context);

    d->context.Reset(d->isolate, context);

    auto global = context->Global();

    auto print_tmpl = v8::FunctionTemplate::New(isolate, Print);
    auto print_val = print_tmpl->GetFunction(context).ToLocalChecked();
    CHECK(global->Set(context, v8_str("deno_print"), print_val).FromJust());

    auto recv_tmpl = v8::FunctionTemplate::New(isolate, Recv);
    auto recv_val = recv_tmpl->GetFunction(context).ToLocalChecked();
    CHECK(global->Set(context, v8_str("deno_recv"), recv_val).FromJust());

    auto send_tmpl = v8::FunctionTemplate::New(isolate, Send);
    auto send_val = send_tmpl->GetFunction(context).ToLocalChecked();
    CHECK(global->Set(context, v8_str("deno_send"), send_val).FromJust());

    creator->SetDefaultContext(context);
  }

  int r = deno_load(d, js_filename, js_source);
  assert(r == 0);

  d->context.Reset();  // Delete persistant handles.
  d->recv.Reset();     // Delete persistant handles.

  auto snapshot_blob =
      creator->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kKeep);

  return snapshot_blob;
}

void deno_dispose(Deno* d) {
  d->isolate->Dispose();
  delete (d);
}

void deno_terminate_execution(Deno* d) { d->isolate->TerminateExecution(); }
}

#include <assert.h>
#include <stdio.h>
#include "binding.h"
#include "v8/include/v8.h"

#include <fstream>
#include <iterator>
#include <vector>

// Caller must free returned value.
v8::StartupData ReadFile(const char* fn) {
  std::ifstream input(fn, std::ios::binary);
  if (!input) {
    printf("Error reading %s\n", fn);
    exit(1);
  }
  std::vector<char> buffer((std::istreambuf_iterator<char>(input)),
                           (std::istreambuf_iterator<char>()));
  v8::StartupData sd;
  sd.data = strdup(reinterpret_cast<char*>(buffer.data()));
  sd.raw_size = static_cast<int>(buffer.size());
  return sd;
}

void WriteFile(const char* fn, v8::StartupData startup_data) {
  std::ofstream output(fn, std::ios::binary);
  output.write(startup_data.data, startup_data.raw_size);
  output.close();
  if (output.bad()) {
    printf("Error writing %s\n", fn);
    exit(1);
  }
}

int main(int argc, char** argv) {
  const char* gen_dir = argv[1];
  const char* js_fn = argv[2];
  const char* out_fn = argv[3];

  auto js_data = ReadFile(js_fn);

  v8_init();

  v8::V8::InitializeExternalStartupData(gen_dir);

  const intptr_t* external_references = nullptr;
  auto ssc = new v8::SnapshotCreator(external_references);
  auto* isolate = ssc->GetIsolate();
  Worker* w = worker_from_isolate(isolate, nullptr, nullptr);

  int r = worker_load(w, js_fn, js_data.data);
  assert(r == 0);

  {
    v8::HandleScope handle_scope(w->isolate);
    auto context = w->context.Get(w->isolate);
    ssc->SetDefaultContext(context);

    // Delete our persistant handles.
    w->context.Reset();
  }

  auto startup_data =
      ssc->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kKeep);
  WriteFile(out_fn, startup_data);
  printf("Wrote snapshot %d %s \n", startup_data.raw_size, out_fn);
}

// Copyright 2018 Ryan Dahl <ry@tinyclouds.org>
// All rights reserved. MIT License.
#ifndef BINDING_H
#define BINDING_H
#ifdef __cplusplus
extern "C" {
#endif

//void v8_init();
void v8_init(int argc, char** argv);
const char* v8_version();
void v8_set_flags(int* argc, char** argv);

// Worker = Wrapped Isolate.
struct worker_s;
typedef struct worker_s Worker;

// Data that gets transmitted.
struct buf_s {
  void* data;
  size_t len;
};
typedef struct buf_s WorkerBuf;

// The callback from V8 when data is sent.
typedef WorkerBuf (*RecvCallback)(Worker* w, WorkerBuf buf);

Worker* worker_new(void* data, RecvCallback cb);
void* worker_get_data();

// Returns nonzero on error.
// Get error text with worker_last_exception().
int worker_load(Worker* w, const char* name_s, const char* source_s);

// Returns nonzero on error.
int worker_send(Worker* w, WorkerBuf buf);

const char* worker_last_exception(Worker* w);

void worker_dispose(Worker* w);
void worker_terminate_execution(Worker* w);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // BINDING_H

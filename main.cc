#include <assert.h>
#include <stdio.h>
#include "binding.h"
#include "v8/include/v8.h"

#include "natives_deno.cc"
#include "snapshot_deno.cc"

int main(int argc, char** argv) {
  v8_init();

  auto natives_blob = *StartupBlob_natives();
  printf("natives_blob %d bytes\n", natives_blob.raw_size);

  auto snapshot_blob = *StartupBlob_snapshot();
  printf("snapshot_blob %d bytes\n", snapshot_blob.raw_size);

  v8::V8::SetNativesDataBlob(&natives_blob);
  v8::V8::SetSnapshotDataBlob(&snapshot_blob);

  Worker* w = worker_from_snapshot(&snapshot_blob, NULL, NULL);
  int r = worker_load(w, "main2.js", "foo();");
  if (r != 0) {
    printf("Error! %s\n", worker_last_exception(w));
    exit(1);
  }

  const char* v = v8::V8::GetVersion();
  printf("Hello World. V8 version %s\n", v);
}

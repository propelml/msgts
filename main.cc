#include <assert.h>
#include <stdio.h>
#include "binding.h"
#include "v8/include/v8.h"

int main(int argc, char** argv) {
  v8_init();

  Worker* w = worker_new(NULL, NULL);
  int r = worker_load(w, "main.js", "1 + 2;");
  if (r != 0) {
    printf("Error!");
    exit(1);
  }

  const char* v = v8::V8::GetVersion();
  printf("Hello World. V8 version %s\n", v);
}

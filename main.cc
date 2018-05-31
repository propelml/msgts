#include <assert.h>
#include <stdio.h>
#include "binding.h"
#include "v8/include/v8.h"

int main() {
  v8_init();

  printf("after v8_init \n");
  Worker* w = worker_new(NULL, NULL);
  printf("after worker new \n");
  int r = worker_load(w, "main.js", "1 + 2;");
  printf("after worker load \n");
  if (r != 0) {
    printf("Error!");
    exit(1);
  }

  const char* v = v8::V8::GetVersion();
  printf("Hello World. V8 version %s\n", v);
}

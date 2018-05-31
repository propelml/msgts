#include <stdio.h>
#include "v8/include/v8.h"

int main() {
	const char* v = v8::V8::GetVersion();
	printf("Hello World. V8 version %s\n", v);
}

#include <stdio.h>

__attribute__((constructor))
void load(void) {
  puts("Loaded dylib");
}

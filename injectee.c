#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

int main() {
  pid_t pid = getpid();
  printf("PID: %d\n", pid);
  while(true) {
    puts("running...");
    sleep(30);
  }
}

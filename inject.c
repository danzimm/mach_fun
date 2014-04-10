#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <mach/mach.h>
#include <stdbool.h>
#include <pthread.h>

#define ENSURE_SUCCESS(c) \
    if ((ret = c) != KERN_SUCCESS) { \
      mach_error( # c , ret); \
      return ret; \
    }

int main(int argc, const char *argv[]) {
  if (argc < 3)
    return -1;
  kern_return_t ret;
  vm_address_t r_libname;
  vm_address_t stack;
  vm_address_t code;
  thread_t thread;
  x86_thread_state64_t state;
  mach_port_t task;
  const char *libname = argv[1];
  unsigned long long stackContents[5], stack_size, i; // the stack contents has to be an odd number of ull's for some reason (some aligning issue) in dlopen
  unsigned char codeContents[38];
  
  bzero(codeContents, sizeof(codeContents));
  bzero(stackContents, sizeof(stackContents));
  codeContents[0] = 0x55; // push rbp
  codeContents[1] = 0x48;
  codeContents[2] = 0x89;
  codeContents[3] = 0xe5; // mov %rsp, %rbp
  codeContents[4] = 0x48;
  codeContents[5] = 0xbf; // mov r_libname, %rdi
  stackContents[1] = (unsigned long long)dlopen;
  stackContents[2] = (unsigned long long)mach_thread_self;
  stackContents[4] = (unsigned long long)thread_suspend;
  stack_size = 65536;
  
  if (strcmp(argv[2], "self") == 0)
    task = mach_task_self();
  else
    ENSURE_SUCCESS(task_for_pid(mach_task_self(), atoi(argv[2]), &task));
  ENSURE_SUCCESS(vm_allocate(task, &r_libname, strlen(libname) + 1, true));
  ENSURE_SUCCESS(vm_allocate(task, &stack, stack_size, true));
  ENSURE_SUCCESS(vm_allocate(task, &code, sizeof(codeContents), true));
  stackContents[0] = code;
  stackContents[3] = (unsigned long long)code + 27;
  ENSURE_SUCCESS(vm_write(task, r_libname, (vm_offset_t)libname, strlen(libname) + 1));
  ENSURE_SUCCESS(vm_write(task, stack + stack_size-sizeof(stackContents), (vm_offset_t)stackContents, sizeof(stackContents)));
  
  memcpy(&codeContents[6], &r_libname, sizeof(unsigned long long));
  codeContents[14] = 0x48;
  codeContents[15] = 0xbe;
  codeContents[16] = 0x2;  // mov 0x2, %rsi
  codeContents[24] = 0x5d; // pop %rbp
  codeContents[25] = 0x90; // nop / int 3 depending if im debugging
  codeContents[26] = 0xc3; // ret
  codeContents[27] = 0x48;
  codeContents[28] = 0x89;
  codeContents[29] = 0xc7;
  codeContents[30] = 0xc3;

  ENSURE_SUCCESS(vm_write(task, code, (vm_offset_t)codeContents, sizeof(codeContents)));
  ENSURE_SUCCESS(vm_protect(task, code, sizeof(codeContents), false, VM_PROT_EXECUTE | VM_PROT_READ));
  printf("Created code region at %p:\n", (void *)code);
  for (i = 0; i < sizeof(codeContents); i++) {
    printf("0x%02x ", codeContents[i]);
  }
  puts("");
  printf("Created stack at %p with top of stack at %p\n", (void*)stack, (void*)(stack + stack_size));
  for (i = 0; i < sizeof(stackContents) / sizeof(stackContents[0]); i++) {
    printf("0x%02llx:\t0x%02llx\n", (stack + stack_size - sizeof(stackContents) + (i * sizeof(unsigned long long))), stackContents[i]);
  }
  bzero(&state, sizeof(state));
  state.__rip = (uint64_t)dlsym(RTLD_DEFAULT, "_pthread_set_self");
  state.__rdi = stack;
  state.__rsp = stack + stack_size-sizeof(stackContents); // end of stack minus returns
  state.__rbp = state.__rsp;
  printf("Found _pthread_set_self at %p\n", (void *)state.__rip);

  ENSURE_SUCCESS(thread_create_running(task, x86_THREAD_STATE64, (thread_state_t)(&state), x86_THREAD_STATE64_COUNT, &thread));
  
  if (strcmp(argv[2], "self") == 0) {
    int rv = pthread_join(*(pthread_t *)stack, NULL);
    if (rv) {
      fprintf(stderr, "pthread_join: (%d) %s\n", rv, strerror(rv));
      sleep(1); // let the dylib actually load in the other thread, it wouldn't appear that there exists mach thread waiting, and I'm too lazy to create a semaphore and using the value of `stack` for a pthread_t in pthread_join doesn't work
    }
  }
}


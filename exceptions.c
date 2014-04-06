#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <pthread.h>
#include <mach/i386/thread_status.h>
#include <capstone/capstone.h>

boolean_t mach_exc_server(mach_msg_header_t *, mach_msg_header_t *);

int instruction_length_at_address(uint64_t address) {
  csh handle;
  cs_insn *insn;
  size_t count;
  int retval = 0;

  if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
    return -1;
  count = cs_disasm_ex(handle, (uint8_t *)address, 12, 0x1000, 0, &insn);
  if (count > 0) {
    retval = insn[0].size;
    cs_free(insn, count);
  }
  cs_close(&handle);
  return retval;
}

kern_return_t catch_mach_exception_raise(mach_port_t exception_port, mach_port_t thread, mach_port_t task, exception_type_t type, exception_data_t code, mach_msg_type_number_t code_count) {
  x86_thread_state_t state;
  kern_return_t kr;
  mach_msg_type_number_t count = x86_THREAD_STATE_COUNT;
  int insn_len = 0;

  printf("%#02x\n", thread);
  if ((kr = thread_get_state(thread, x86_THREAD_STATE, (thread_state_t)&state, &count)) != KERN_SUCCESS) {
    mach_error("thread_get_state", kr);
    goto bad;
  }
  insn_len = instruction_length_at_address(state.uts.ts64.__rip);
  if (insn_len == 0 || insn_len == -1)
    goto bad;
  printf("Got exception type %d at %#02llx insn_len: %d\n", type, state.uts.ts64.__rip, insn_len);
  state.uts.ts64.__rip += insn_len;
  count = x86_THREAD_STATE_COUNT;
  if ((kr = thread_set_state(thread, x86_THREAD_STATE, (thread_state_t)&state, count)) != KERN_SUCCESS) {
    mach_error("thread_set_state", kr);
    goto bad;
  }
  return KERN_SUCCESS;
bad:
  return KERN_FAILURE;
}


kern_return_t catch_mach_exception_raise_state(mach_port_t exception_port, exception_type_t exception, exception_data_t code, mach_msg_type_number_t code_count, int *flavor, thread_state_t in_state, mach_msg_type_number_t in_state_count, thread_state_t out_state, mach_msg_type_number_t *out_state_count) {
  printf("noooooo\n");
  exit(1);
  return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state_identity(mach_port_t exception_port, mach_port_t thread, mach_port_t task, exception_type_t exception, exception_data_t code, mach_msg_type_number_t code_count, int *flavor, thread_state_t in_state, mach_msg_type_number_t in_state_count, thread_state_t out_state, mach_msg_type_number_t *out_state_count) {
  printf("nooooo\n");
  exit(1);
  return KERN_FAILURE;
}

void *server_thread(void *arg) {
  mach_port_t exc_port = *(mach_port_t *)arg;
	kern_return_t kr;

	while(1) {
		if ((kr = mach_msg_server_once(mach_exc_server, 4096, exc_port, 0)) != KERN_SUCCESS) {
			fprintf(stderr, "mach_msg_server_once: error %#x\n", kr);
			exit(1);
		}
	}
	return (NULL);
}

int main() {
  kern_return_t kr = 0;
  mach_port_t exc_port;
  mach_port_t task = mach_task_self();
  pthread_t exception_thread;
  int err;
  
	mach_msg_type_number_t maskCount = 1;
	exception_mask_t mask;
	exception_handler_t handler;
	exception_behavior_t behavior;
	thread_state_flavor_t flavor;

	if ((kr = mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &exc_port)) != KERN_SUCCESS) {
		fprintf(stderr, "mach_port_allocate: %#x\n", kr);
		exit(1);
	}

	if ((kr = mach_port_insert_right(task, exc_port, exc_port, MACH_MSG_TYPE_MAKE_SEND)) != KERN_SUCCESS) {
		fprintf(stderr, "mach_port_allocate: %#x\n", kr);
		exit(1);
	}

	if ((kr = task_get_exception_ports(task, EXC_MASK_ALL, &mask, &maskCount, &handler, &behavior, &flavor)) != KERN_SUCCESS) {
		fprintf(stderr,"task_get_exception_ports: %#x\n", kr);
		exit(1);
	}

	if ((err = pthread_create(&exception_thread, NULL, server_thread, &exc_port)) != 0) {
		fprintf(stderr, "pthread_create server_thread: %s\n", strerror(err));
		exit(1);
	}
  
	pthread_detach(exception_thread);

  if ((kr = task_set_exception_ports(task, EXC_MASK_ALL, exc_port, EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES, flavor)) != KERN_SUCCESS) {
    fprintf(stderr, "task_set_exception_ports: %#x\n", kr);
    exit(1);
  }
  puts("Starting exception stuff");
  int a = 1;
  int b = 0;
  float c = a / b;
  int cat = *(int *)(0x13371337);
  printf("Have c %f and cat %d\n", c, cat);
  puts("End.");

}

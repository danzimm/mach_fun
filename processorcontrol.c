#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <unistd.h>

void usage(void) {
  printf("to be doc'd...\n");
  exit(0);
}

#define STRINGIFY(a) #a

#define CHECKRET(r) \
  if (r) { \
    mach_error(__FILE__ ":" STRINGIFY(__LINE__), ret); \
    return r; \
  }

int main(int argc, const char *argv[]) {
  if (seteuid(0)) {
    perror("seteuid");
    return -1;
  }
  int i, ch, index = -1, enabled = -1, all = 0, nprocs = 0;
  host_t host;
  processor_array_t processors;
  mach_msg_type_number_t nprocessors, info_count;
  kern_return_t ret;
  struct processor_basic_info basic;

  while ((ch = getopt(argc, (char *const *)argv, "nai:e:")) != -1) {
    switch(ch) {
      case 'a':
        all = 1;
        break;
      case 'i':
        index = atoi(optarg);
        break;
      case 'e':
        enabled = optarg[0] == '1' ? 1 : 0;
        break;
      case 'n':
        nprocs = 1;
        break;
      case '?':
      default:
        usage();
        break;
    }
  }

  host = mach_host_self();
  ret = host_processors(host, &processors, &nprocessors);
  CHECKRET(ret);
  if (nprocs)
    printf("Number of Processors: %d\n", nprocessors);
  if (all) {
    for (i = 0; i < nprocessors; i++) {
      bzero(&basic, sizeof(struct processor_basic_info));
      info_count = PROCESSOR_BASIC_INFO_COUNT;
      ret = processor_info(processors[i], PROCESSOR_BASIC_INFO, &host, (processor_info_t)&basic, &info_count);
      CHECKRET(ret);
      if (!basic.is_master) {
        if (enabled == 0) {
          processor_exit(processors[i]);
        } else if (enabled == 1) {
          processor_start(processors[i]);
        }
      }
      bzero(&basic, sizeof(struct processor_basic_info));
      info_count = PROCESSOR_BASIC_INFO_COUNT;
      ret = processor_info(processors[i], PROCESSOR_BASIC_INFO, &host, (processor_info_t)&basic, &info_count);
      CHECKRET(ret);
      printf("Processor %d: %d:%d %s %d %s\n", i, basic.cpu_type, basic.cpu_subtype, basic.running ? "Running" : "Not Running", basic.slot_num, basic.is_master ? "Master" : "Slave");
    }
  } else {
    if (index != -1 && index < nprocessors) {
      bzero(&basic, sizeof(struct processor_basic_info));
      info_count = PROCESSOR_BASIC_INFO_COUNT;
      ret = processor_info(processors[index], PROCESSOR_BASIC_INFO, &host, (processor_info_t)&basic, &info_count);
      CHECKRET(ret);
      if (!basic.is_master) {
        if (enabled == 0) {
          processor_exit(processors[index]);
        } else if (enabled == 1) {
          processor_start(processors[index]);
        }
      }
      bzero(&basic, sizeof(struct processor_basic_info));
      info_count = PROCESSOR_BASIC_INFO_COUNT;
      ret = processor_info(processors[index], PROCESSOR_BASIC_INFO, &host, (processor_info_t)&basic, &info_count);
      CHECKRET(ret);
      printf("Processor %d: %d:%d %s %d %s\n", index, basic.cpu_type, basic.cpu_subtype, basic.running ? "Running" : "Not Running", basic.slot_num, basic.is_master ? "Master" : "Slave");
    }
  }
}


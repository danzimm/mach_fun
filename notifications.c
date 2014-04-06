#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach.h>
#include <bootstrap_priv.h>
#include "UNDRequest.h"

void usage() {
  puts("TODO: usage.");
  exit(0);
}

int main(int argc, const char *argv[]) {
  int timeout = 0, ch;
  unsigned flags = 0;
  char *iconPath = NULL, *soundPath = NULL, *localizationPath = NULL, *header = NULL, *message = NULL, *defaultButtonTitle = NULL;
  kern_return_t ret;
  UNDServerRef UNDServer;
  
  while ((ch = getopt(argc, (char *const *)argv, "t:f:i:s:l:h:m:d:")) != -1) {
    switch(ch) {
      case 't':
        timeout = atoi(optarg);
        break;
      case 'f':
        flags = (unsigned)atoi(optarg);
        break;
      case 'i':
        iconPath = optarg;
        break;
      case 's':
        soundPath = optarg;
        break;
      case 'l':
        localizationPath = optarg;
        break;
      case 'h':
        header = optarg;
        break;
      case 'm':
        message = optarg;
        break;
      case 'd':
        defaultButtonTitle = optarg;
        break;
      case '?':
      default:
        usage();
        break;
    }
  }
  

  ret = bootstrap_look_up2(bootstrap_port, "com.apple.system.Kernel[UNC]Notifications", &UNDServer, 0, 0);
  if (ret) {
    mach_error("bootstrap_look_up2", ret);
    return ret;
  }
  ret = UNDDisplayNoticeSimple_rpc(UNDServer, timeout, flags, iconPath ?: "", soundPath ?: "", localizationPath ?: "", header ?: "", message ?: "", defaultButtonTitle ?: "");
  if (ret) {
    mach_error("UNDDisplayNoticeSimple_rpc", ret);
    return ret;
  }
  mach_port_deallocate(mach_task_self(), UNDServer);
  return 0;
}


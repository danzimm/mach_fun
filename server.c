#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include "message.h"

kern_return_t start_server(void) {
  mach_port_t server_port = MACH_PORT_NULL;
  kern_return_t ret = 0;
  message_receive_t *message = NULL;
  unsigned long size = 0;

  ret = bootstrap_check_in(bootstrap_port, (char *)server_name, &server_port);
  CHECKBAD(ret, bad);
  
  message = malloc(sizeof(message_receive_t));
  while (true) {
    puts("Listening...");
    size = sizeof(message_receive_t);
    bzero(message, sizeof(message_receive_t));
    message->hdr.msgh_size = size;
    ret = mach_msg((mach_msg_header_t *)message, MACH_RCV_MSG, 0, size, server_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    if (ret) {
      mach_error("mach_msg", ret);
      continue;
    }
    printf("Got message: %s\n", message->data);
    if (strcmp(message->data, "zimm_bye") == 0)
      break;
  }

  mach_port_deallocate(mach_task_self(), server_port);

bad:
  return ret;
}

#ifndef MACH_SERVER_LIB

int main() {
  kern_return_t ret = start_server();
  if (ret) {
    mach_error("start_server", ret);
  }
  return ret;
}

#endif

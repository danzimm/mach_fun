#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include "message.h"

int main() {
  message_send_t *message = NULL;
  unsigned long size = 0;
  kern_return_t ret = 0;
  mach_port_t server_port = MACH_PORT_NULL;
  
  ret = bootstrap_look_up(bootstrap_port, server_name, &server_port);
  if (ret && ret != 0x44e) {
    mach_error("bootstrap_loop_up", ret);
    goto bad;
  }
  if (server_port == MACH_PORT_NULL || ret == 0x44e) {
    printf("Failed to find %s\n", server_name);
    goto bad;
  }
  
  message = malloc(sizeof(message_send_t));

  while (server_port) {
    bzero(message, sizeof(message_send_t));
    size = sizeof(message_send_t);
    printf("Enter data: ");
    fgets(message->data, sizeof(message->data) - 1, stdin);
    if (feof(stdin))
      break;
    if (message->data[strlen(message->data)-1] == '\n')
      message->data[strlen(message->data)-1] = '\0';
    message->hdr.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    message->hdr.msgh_remote_port = server_port;
    message->hdr.msgh_id = 0;
    message->hdr.msgh_size = size;
    ret = mach_msg((mach_msg_header_t *)message, MACH_SEND_MSG, size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    if (ret) {
      mach_error("mach_msg", ret);
      goto bad;
    }
    puts("Sent.");
  }
  puts("Done.");
bad:
  return 0;
}

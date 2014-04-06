#include <mach/mach.h>

#ifndef __MESSAGE_H
#define __MESSAGE_H

#define CHECKBAD(r, b) if (r != KERN_SUCCESS) goto b
#define MESSAGEPAIR(name, body) \
  typedef struct name ## _send { \
    mach_msg_header_t hdr; \
    body; \
  } name ## _send_t; \
  typedef struct name ## _receive { \
    mach_msg_header_t hdr; \
    body; \
    mach_msg_trailer_t trailer; \
  } name ## _receive_t

MESSAGEPAIR(message, char data[256]);

const char *server_name = "zimm_server";

kern_return_t start_server(void);

#endif

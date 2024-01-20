#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H
#include "globals.h"
#include <linux/aio_abi.h>
#include <arpa/inet.h>

unsigned char multiplexer(struct iocb_ptrs_struct *iocb_ptrs,struct io_event *io_events, struct sockaddr_in6 *address); 

#endif

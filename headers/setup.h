#ifndef SETUP_H
#define SETUP_H
#include <netinet/in.h>
#include "globals.h"
#include <linux/aio_abi.h>

unsigned int create_and_setup_server_socket(struct sockaddr_in6 *addresss);

#endif

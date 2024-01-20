/* main.c */
/* TODO 
 * create an array of timers of connections waiting for reads or writes
 * and check them after select, to close any
 * before reading a socket shutdown them being able to write */
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <poll.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/aio_abi.h>
#include "headers/tools.h"
#include "headers/globals.h"
#include "headers/cmdline.h"
#include "headers/setup.h"
#include "headers/multiplexer.h"

/* Program start */
int 
main(int argc, char *argv[]) {

	/* Init variables */
	unsigned int main_socket;
	unsigned char retval;
	struct iocb_ptrs_struct iocb_ptrs;
	struct sockaddr_in6 address;
	struct io_event io_events[MAX_CONTROL_BLOCKS];
	int i;
	/* Fill structs with 0 */
	memset(&address,0,sizeof(address));
	memset(&iocb_ptrs,0,sizeof(struct iocb_ptrs_struct));
	memset(&io_events,0,sizeof(struct io_event) * MAX_CONTROL_BLOCKS);

	/* Make sure unsigned int is 4 bytes */
	assert(sizeof(unsigned int) == 4);
	/* Make sure unsigned long is 8 bytes */
	assert(sizeof(unsigned long) == 8);
	/* Make sure stdin file descriptor is 0 */
	assert(STDIN_FILENO == 0);
	/* Make sure ULONG max is 18446744073709551615 */
	assert(ULONG_MAX == 18446744073709551615UL);

	/* Find and save command line arguments in 
	 * file_locations and program_modes variables  */
	find_and_save_cmdline_args(argc,argv);
	if(program_modes)
		execute_informational_program_modes();

	/* Set up main network socket */
	main_socket = create_and_setup_server_socket(&address);

	/* Tie control block pointer array to control block array */
	for(i=0;i < MAX_CONTROL_BLOCKS; i++) {
		iocb_ptrs.ptrs[i] = &(control_blocks_for_queue[i]);
	}

	/* Add main network socket to control block queue */
	iocb_ptrs.ptrs[0]->aio_lio_opcode = IOCB_CMD_POLL;
	iocb_ptrs.ptrs[0]->aio_buf = POLLIN;
	iocb_ptrs.ptrs[0]->aio_fildes = main_socket;
	iocb_ptrs.ptrs[0]->aio_data = ULONG_MAX;
	
	/* Main loop ------------------------------- */
	for(;;) { 

	/* Go to multiplexer and block until a list of io_event structs
	 * (ie new data on sockets) are ready to process */
	retval = multiplexer(&iocb_ptrs,io_events,&address); 
	if(retval == CONTINUE)
		continue;

	/* If main socket is ready, accept new connection, and give it a new time */

	/* Iterate the file descriptors and handle their connections appropriately */
	} 

	/* Exit entire program */
	close(main_socket);
	output_log("exiting...",STDOUT_LOG,0);
	/* TODO close all connections by iterating iocb_ptrs.ptrs */
	return 0;

} 

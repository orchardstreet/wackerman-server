#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#define DEBUG
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/aio_abi.h>
#include "headers/tools.h"
#include "headers/multiplexer.h"
#include "headers/globals.h"

static int 
io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp)
{
	return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

static int 
io_getevents(aio_context_t ctx, long min_nr, long max_nr, struct io_event *events, struct timespec *timeout)
{
	return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

static int 
io_setup(unsigned nr, aio_context_t *ctxp)
{
	return syscall(__NR_io_setup, nr, ctxp);
}

static int 
io_destroy(aio_context_t ctx)
{
	return syscall(__NR_io_destroy, ctx);
}

unsigned char 
multiplexer(struct iocb_ptrs_struct *iocb_ptrs, struct io_event *io_events, struct sockaddr_in6 *address)  {

	/* Init variables */ 
	int errno_save, new_connection_fd, num_events, i, socket_flags;
	socklen_t addr_len = sizeof(struct sockaddr_in6);
	aio_context_t ctx = 0;

	/* Call io_setup(), which is necessary to use io_submit() */
	if(io_setup(MAX_CONTROL_BLOCKS,&ctx) == -1) {
		output_log("error with io_setup()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}

	/* Submit control block pointers to kernel ring buffer in ctx for processing */
	errno = 0;
#ifdef DEBUG
	printf("\nsubmitting %u pointers to I/O control blocks\n",iocb_ptrs->index + 1);
#endif
	if(io_submit(ctx,iocb_ptrs->index + 1,iocb_ptrs->ptrs) < 0) {
		errno_save = errno;
		output_log("error with io_sumit()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		if(errno_save == EAGAIN) {
			fprintf(stderr,"Not enough resources, looping back to io_submit\n");
			io_destroy(ctx);
			return CONTINUE;
		}
		exit(EXIT_FAILURE);
	}
	/* Block entire program until at least one I/O control block event is finished, and return the events in 
	 * io_events and the number of events in num_events */
	num_events = io_getevents(ctx,1,MAX_CONTROL_BLOCKS,io_events,NULL);
	if(num_events < 0) {
		output_log("error with io_getevents()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	printf("I/O events received: %d\n",num_events);
	usleep(1000000);
#endif

	/* Iterate the io_events returned from io_submit above */
	for(i=0; i < num_events; i++) {
		/* If there is a new connection on main_socket, then accept it and save the file descriptor in new_connection_df */
		if((uint64_t)io_events[i].obj == (uint64_t)iocb_ptrs->ptrs[0]) { 
/*		if(io_events[i].data == ULONG_MAX) {  */
#ifdef DEBUG
			printf("new connection request\n");
			fflush(stdout);
			usleep(1000000);
#endif
			errno=0;
			new_connection_fd = accept(iocb_ptrs->ptrs[0]->aio_fildes,(struct sockaddr *)address,&addr_len);
			/* Check for accept() errors */
			if(new_connection_fd < 0) {
				if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EBADF 
						   || errno == ECONNABORTED || errno == EPERM) {
					output_log("accept() couldn't get connection (not necessarily an error):",SYSTEM_STDERR_LOG|ERROR_LOG_FILE,0);
					/* ignore failed connection attempt and process other file descriptors */
					continue;
				}
				if(errno == EINTR) {
					output_log("accept() interrupted by EINTR, trying again:",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
					/* try accept() on main_socket again */
					i = -1;
					continue;
				}
				/* else there was a fatal error on accept(), exit entire program */
				output_log("accept() fatal error:",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
				exit(EXIT_FAILURE);
			}
			/* Make new connection non-blocking */
			socket_flags = fcntl(new_connection_fd,F_GETFL);
			if(fcntl(new_connection_fd,F_SETFL,socket_flags | O_NONBLOCK ) == -1) {
				output_log("error with fcntl()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
				exit(EXIT_FAILURE);
			}
			/* If control block queue is full then close the last connection.
			 * There should only be 1 slot in the control block queue
			 * for each connection. 
			 * TODO: Check for file descriptor collisions right above this comment 
			 * and close last duplicate FD if one exists with a function using memmove(). 
			 * Then create a code section right below to simply replace that slot and return
			 * before the rest of the connection accept code */
			iocb_ptrs->index = iocb_ptrs->index + 1;
			if(iocb_ptrs->index == MAX_CONTROL_BLOCKS) {
				iocb_ptrs->index = iocb_ptrs->index - 1;
				if((iocb_ptrs->ptrs[iocb_ptrs->index]->aio_fildes) != 0) {
					close(iocb_ptrs->ptrs[iocb_ptrs->index]->aio_fildes);
					memset(iocb_ptrs->ptrs[iocb_ptrs->index],0,sizeof(struct iocb));
				}
			}
			/* Add new connection to control block queue as a read request */
			iocb_ptrs->ptrs[iocb_ptrs->index]->aio_lio_opcode = IOCB_CMD_PREAD;
			iocb_ptrs->ptrs[iocb_ptrs->index]->aio_buf = (uint64_t)read_buffers[iocb_ptrs->index];
			iocb_ptrs->ptrs[iocb_ptrs->index]->aio_nbytes = READ_BUFFER_SIZE;
			iocb_ptrs->ptrs[iocb_ptrs->index]->aio_fildes = new_connection_fd;
/*			iocb_ptrs->ptrs[iocb_ptrs->index]->aio_rw_flags = RWF_HIPRI; */
			iocb_ptrs->ptrs[iocb_ptrs->index]->aio_data = iocb_ptrs->index;
#ifdef DEBUG
			printf("index: %u\n",iocb_ptrs->index);
#endif

			output_log("accepted new connection",STDOUT_LOG,0);
			fflush(stdout);
			continue;
		} /* end of accepting new connection */
		/* else if file descriptor is not main_socket */
#ifdef DEBUG
		printf("data from %dth io_event: %lu\n",i,(unsigned long)io_events[i].data);
		printf("res: %ld\n",(long int)io_events[i].res);
		printf("res2: %ld\n",(long int)io_events[i].res2);
#endif
		if(io_events[i].res == 0) {
#ifdef DEBUG
			printf("closing fd %d...\n",iocb_ptrs->ptrs[io_events[i].data]->aio_fildes);
#endif
			close(iocb_ptrs->ptrs[io_events[i].data]->aio_fildes);
		}

		/* TODO: process reads and writes */

	} /* end of iterating new io_events */

	/* Clearing the io_events array, might not be necessary 
	 * but probably is. */
	memset(io_events,0,num_events * sizeof(struct io_event));
	io_destroy(ctx);

	return SUCCESS;

}

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "headers/setup.h"
#include "headers/globals.h"
#include "headers/tools.h"
#define MAX_PORT_DIGITS 5
#define NULL_CHARACTER 1

static const char success_log_intro[] = "Listening on 0.0.0.0, port: ";
#define SUCCESS_LOG_SIZE sizeof(success_log_intro) - 1 + MAX_PORT_DIGITS + NULL_CHARACTER

unsigned int 
create_and_setup_server_socket(struct sockaddr_in6 *address)
{

	char success_log[SUCCESS_LOG_SIZE]; 
	int on = 1; 
	int socket_flags;
	int main_socket;

	/* Check if server port provided is out of range */
	if(SERVER_PORT <= 0 || SERVER_PORT > 65535) {
		output_log("Port provided is out of range\n"
				"should be between 0 and 65535"
				,STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}
	sprintf(success_log,"%s%d",success_log_intro,SERVER_PORT);

	/* Configure main socket address info */
	address->sin6_addr = in6addr_any;
	address->sin6_family = AF_INET6;
	address->sin6_port = htons(SERVER_PORT);

	/* Create socket for both IPV6 and IPV4 */
	if((main_socket = socket(AF_INET6,SOCK_STREAM,0)) < 0) {
		output_log("error with socket()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}

	/* Make socket non-blocking */
	socket_flags = fcntl(main_socket,F_GETFL);
	if(fcntl(main_socket,F_SETFL,socket_flags | O_NONBLOCK) == -1) {
		output_log("error with fcntl()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}

	/* Make address re-usable */
	if(setsockopt(main_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on)) == 1) {
		output_log("error with setsockopt(SO_REUSEADDR)",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}

	/* Bind socket to address */
	if(bind(main_socket,(const struct sockaddr *)address,(socklen_t)sizeof(struct sockaddr_in6)) == -1) {
		output_log("error with bind()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}

	/* Start listening on socket */
	if(listen(main_socket,LISTEN_BACKLOG_MAX) == -1) {
		output_log("error with listen()",SYSTEM_STDERR_LOG | ERROR_LOG_FILE,0);
		exit(EXIT_FAILURE);
	}

	/* Output success notice (listening on ...) */
	output_log(success_log,STDOUT_LOG,0);

	return (unsigned int) main_socket;

}

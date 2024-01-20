#include "headers/globals.h"
#include <linux/aio_abi.h>

/* Initialize globals from globals.h */

/* Program mode data (global) --------------------------------------------------------------------*/
/* Program_modes variable contains Program mode flags. It contains 8 bits which are 8 flags that 
 * define which program modes are on. Uses the defines in cmdline.h associated with 
 * the program_mode_strings array below for the order of flags */
unsigned char program_modes;
/* Global array of program mode string data for parsing command line arguments 
 * should be in order of defines in globals.h */
/* Cannot exceed 8 items in array */
char *program_mode_strings[] = {
	"daemon",
	"silent",
	"version",
	"help",
};
/* Cannot exceed 8 items in array */
char *program_mode_string_explanations[] = {
	"Runs program as a daemon",
	"Runs program without printing to stdout or stderr",
	"Displays version number and license",
	"Runs this prompt",
};
/* Global number of program modes */
const unsigned char number_of_program_modes = sizeof(program_mode_strings) / sizeof(char *);


/* File location data (global) ---------------------------------------------------------------------*/
/* Array of file location string data for parsing command line arguments */
/* should be in order of defines in globals.h */
char *file_location_strings[] = {
	"accesslog",
	"errorlog",
	"conf"
};
char *file_location_string_explanations[] = {
	"Specify an access log file location,\n"
	"		eg 'http_server -a /var/log/http_server_access.log'",
	"Specify an error log file location,\n"
	"		eg 'http_server -e /var/log/http_server_error.log'",
	"Specify a configuration file location,\n"
	"		eg 'http_server -c /etc/http_server/http_server.conf'",
};
/* A global array with the file location data */
char *file_locations[sizeof(file_location_strings) / sizeof(char *)];
/* Global number of file locations */
const unsigned char number_of_file_locations = sizeof(file_location_strings) / sizeof(char *);

/* Buffers */
struct iocb control_blocks_for_queue[MAX_CONTROL_BLOCKS];
char read_buffers[MAX_CONTROL_BLOCKS][READ_BUFFER_SIZE];

/* custom errno */
int errno_custom;

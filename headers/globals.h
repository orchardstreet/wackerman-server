/*TODO progam_modes to flag_program_modes
 * and file_locations to value_program_modes 
 * and create new function called in main which parses
 * integer based value program modes from value_program_modes
 * and stores them in integer_value_program_modes, also create
 * MAX_FLAG_PROGRAM_MODES and MAX_VALUE_PROGRAM_MODES and use them
 * as appropriate to replace magic numbers in cmdline.c */
#ifndef GLOBALS_H
#define GLOBALS_H
#define ISSET(X,Y) X & Y
#include <time.h>
#include <linux/aio_abi.h>

#define SERVER_PORT 8000
#define MAX_CONNECTIONS 3000 /* MAX_CONNECTIONS can't be more than UINT_MAX - 1 */
#define READ_BUFFER_SIZE 100000
#define MAX_CONTROL_BLOCKS MAX_CONNECTIONS + 1 /* Can't be equal or longer than ULONG_MAX */
#define LISTEN_BACKLOG_MAX 4096

enum exit_codes{SUCCESS,FAILURE,CONTINUE};

/* Defines for program_modes
 * to add a new program mode, add a new define below
 * and a new, corresponding entry in progam_mode_strings in main.c, and that's it! */
#define DAEMON_MODE 1 << 0
#define SILENT_MODE 1 << 1
#define VERSION_MODE 1 << 2
#define HELP_MODE 1 << 3
/* Holds the strings which correspond to the defines above, initiated in main.c */
extern char *program_mode_strings[];
extern char *program_mode_string_explanations[];
/* Global program mode flags. Global 8 bits which are 8 flags that define which program modes are on.  
 * Uses the defines in cmdline.h associated with the program_mode_strings array below 
 * for the order of flags */
extern unsigned char program_modes; /* stores program modes as bits */
/* number of program modes */
extern const unsigned char number_of_program_modes;

/* Defines for command-line-defined file locations
 * To add a new command line option for a file location: add a new define
 * and then a new entry in file_location_strings in main.c, and that's it! */
#define RESOURCE_ACCESS_LOG_FILE_LOCATION_INDEX 0
#define ERROR_LOG_FILE_LOCATION_INDEX 1
#define CONF_FILE_LOCATION_INDEX 2
/* Holds the strings which correspond to the defines above, initiated in main.c */
extern char *file_location_strings[];
extern char *file_location_string_explanations[];
/* A global pointer array with the same number of elements as the array above */
extern char *file_locations[];
/* Number of file locations */
extern const unsigned char number_of_file_locations;

struct iocb_ptrs_struct {
	struct iocb *ptrs[MAX_CONTROL_BLOCKS];
	unsigned int index;
};

/* Custom errno value */
extern int errno_custom;
extern struct iocb control_blocks_for_queue[MAX_CONTROL_BLOCKS];
extern char read_buffers[MAX_CONTROL_BLOCKS][READ_BUFFER_SIZE];

#endif

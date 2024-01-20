#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include "headers/tools.h"
#include "headers/globals.h"


/* Function for exiting program as failure */
void die(const char *str) {
	fputs(str,stderr);
	exit(EXIT_FAILURE);
}

void output_log(char *str, unsigned char log_types, unsigned char log_level)
{

	(void)log_level;
	
	if(file_locations[RESOURCE_ACCESS_LOG_FILE_LOCATION_INDEX] 
			&& ISSET(log_types,RESOURCE_ACCESS_LOG_FILE)) {
		return;
	}

	if(file_locations[ERROR_LOG_FILE_LOCATION_INDEX] 
			&& ISSET(log_types,ERROR_LOG_FILE)) {
		return;
	}

	if(ISSET(program_modes,SILENT_MODE)) {
		return;
	}

	if(ISSET(log_types,STDOUT_LOG)) {
		puts(str);
		return;
	}

	if(ISSET(log_types,STDERR_LOG)) {
		fputs(str,stderr);
		return;
	}

	if(ISSET(log_types,SYSTEM_STDERR_LOG)) {
		perror(str);
		return;
	}

}

unsigned int strnlen_custom(char *str, unsigned int max_size)
{
	/* This returns the amount of characters in str, not including null characters, 
	 * up to and including max_size.
	 * If the string length exceeds max_size then errno_custom is to ERANGE and the function returns 0. 
	 * Caller should manually sey errno_custom to 0 before calling strlen_custom().
	 * If errno_custom is ERANGE after calling the function then don't use the return value. */

	/* Note that this function is significantly different than the GNU strnlen. Because you 
	 * can use the max_size if max_size is the return value and the function doesn't return ERANGE 
	 * in errno_custom.  This is more simple than the off-by-one errors that can be easy to make when 
	 * GNU strnlen() */

	unsigned int i;

	/* max_size can't be greater than UINT_MAX - 1 */
	if(max_size > UINT_MAX - 1)
		die("'max_size parameter to strlen_custom() can't be greater than UINT_MAX - 1'");

	/* count characters */
	for(i=0; i < max_size && str[i]; i++) { }

	/* if size is equal to max size */
	if(i == max_size && str[i]) {
		errno_custom = ERANGE;
		return 0;
	}

	return i;
}


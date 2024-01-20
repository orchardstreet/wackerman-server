#ifndef TOOLS_H
#define TOOLS_H

void die(const char *str); 

void output_log(char *str, unsigned char log_types, unsigned char log_level);

/* Log types */
#define	STDOUT_LOG 1 << 0
#define	STDERR_LOG  1 << 1
#define	SYSTEM_STDERR_LOG 1 << 2 
#define	RESOURCE_ACCESS_LOG_FILE 1 << 3
#define	ERROR_LOG_FILE 1 << 4


#endif

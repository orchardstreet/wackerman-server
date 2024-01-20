/* cmdline.c */
/* Functions in file: 
 * 1) static signed char save_single_dash_arg(int *i, int argc, char **argv, char *single_dash_arg_ptr)
 * 2) static signed char find_and_save_single_dash_args(int *i, int argc, char **argv) 
 * 3) static signed char find_and_save_double_dash_arg(int *i, int argc, char **argv) 
 * 4) void find_and_save_cmdline_args(int argc, char **argv) */
#include "headers/cmdline.h"
#include "headers/globals.h"
#include "headers/tools.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Include a global variable to indicate a full iocb_ptrs buffer.  
 * Once this state is reached then enter DDOS_MITIGATION_MODE.
 * Close all sockets belonging to IPs that exceed rate limit
 * and add then to UFW blacklist. */

static unsigned char number_of_possible_args;

/* Error and informational strings for command line argument parsing */
static const char *bad_syntax = "Bad syntax...\n"
			  "Run --help for full parameter list\n";

static const char *version = "Copyright William Lupinacci 2023\n"
		       "Licensed under GPL v3\n";

static signed char 
save_single_dash_arg(unsigned char *i, int argc, char **argv, char *single_dash_arg_ptr)
{

	/* init variables */
	unsigned char p;

	/* Iterate program_mode_strings array to find match */
	for(p=0; p < number_of_program_modes && p < 8; ++p) {
		if(*single_dash_arg_ptr == *(program_mode_strings[p])) {
			/* Match found */
			/* die as bad syntax if already set */
			if(ISSET(program_modes,1 << p))
				return FAILURE;
			/* set flag as bit in program_modes */
			program_modes |= 1 << p;
			return SUCCESS;
		}
	}
	/* Iterate file_location_strings array to find match */
	for(p=0; p < number_of_file_locations && p < 8; ++p) {
		if(*single_dash_arg_ptr == *(file_location_strings[p])) {
			/* Make sure single dash arg is single character */
			++single_dash_arg_ptr;
			if(*single_dash_arg_ptr)
				return FAILURE;
			/* Iterate index of argv from find_and_save_cmdline_args()
			 * should probably rename this to main_argv_iterator */
			++*i;
			/* If argv strings or max number of args run out, then exit as bad syntax */
			if(*i >= argc || *i > number_of_possible_args)
				return FAILURE;
			/* Else save file location string location
			 * in file_locations as pointer to char */
			file_locations[p] = argv[*i];
			return SUCCESS;
		}
	}

	/* If user provided character didn't match any arg option, exit as FAILURE */
	return FAILURE;

} /* end of save_single_dash_arg() function */


static signed char
find_and_save_single_dash_args(unsigned char *i, int argc, char **argv)
{

	/* init variables */
	unsigned char p;	
	
	/* Store number of characters after first dash as args_length
	 * eg -vh would make args_length the value of 2 */
	char *single_dash_arg_ptr = argv[*i] + 1;

	/* If no options provided, then return as failure */
	if(!(*single_dash_arg_ptr)) 
		return FAILURE;

	/* Iterate every character after the first dash of eg -vhs 
	 * then check if it is a valid arg in save_single_dash_arg() 
	 * and if it is valid, then save in a respective global variable
	 * if not a match then exit as failure */
	for(p=0; *single_dash_arg_ptr; single_dash_arg_ptr++, ++p) {
		/* if number of single dash options exceed total possible options, return as failure */
		if(p == number_of_possible_args)
			return FAILURE;
		if(save_single_dash_arg(i, argc, argv,single_dash_arg_ptr) == FAILURE)
			return FAILURE;
	}

	return SUCCESS; 

} /* end of find_and_save_single_dash_args() */


static signed char 
find_and_save_double_dash_arg(unsigned char *i, int argc, char **argv)
{
	/* init variables */
	unsigned char p;
	char *double_dash_arg = argv[*i] + 2;

	/* If *argv + 2 is a valid double dash command line option which doesn't
	 * require a separate value, then save it in program_modes as a flag, then 
	 * exit function as success */
	for(p=0; p < number_of_program_modes && p < 8; ++p) {
		if(!strcmp(double_dash_arg,program_mode_strings[p])) {
			/* die if flag already set */
			if(ISSET(program_modes,1 << p))
				return FAILURE;
			/* set flag */
			program_modes |= 1 << p;
			return SUCCESS;
		}
	}

	/* If *argv + 2 is a valid double dash command line option which requires a file location
	 * as a separate value, then iterate argv index and check if it exists.  If it exists
	 * then save it's location if file_locations array */
	for(p=0; p < number_of_file_locations && p < 8; ++p) {
		if(!strcmp(double_dash_arg,file_location_strings[p])) {
			/* Iterate main argv iterator, should probably rename as main_argv_iterator */
			++*i;
			/* If argv strings or options run out, then exit as bad syntax */
			if(*i >= argc || *i > number_of_possible_args)
				return FAILURE;
			/* Save file location string location as pointer to char in 
			 * file_locations array */
			file_locations[p] = argv[*i];
			return SUCCESS;
		}
	}

	/* If we never returned as SUCCESS in previous two for loops, then no valid double dash argument 
	 * was found. Exit as bad syntax. */
	return FAILURE;
} /* end of find_and_save_double_dash_arg() */


void 
find_and_save_cmdline_args(int argc, char **argv)
{
	/* init variables */
	unsigned char i;
	number_of_possible_args = number_of_program_modes + (number_of_file_locations * 2);

	/* Save any flags from the command line in the program_modes variable as individual bits 
	 * or the value of any command line flags in any respective pointer to char variables */
	for(i = 1; i < argc && i <= number_of_possible_args; i++) {
		/* If there in an unaccounted argument with no dash, exit program as failure */
		if(*(argv[i]) != '-') 
			die(bad_syntax);
		/* If argument leads with two dashes */
		if(!strncmp(argv[i],"--",2)) {
			if(find_and_save_double_dash_arg(&i,argc,argv) == FAILURE)
				die(bad_syntax);
		} else { /* else leads with a single dash */
			if(find_and_save_single_dash_args(&i,argc,argv) == FAILURE)
				die(bad_syntax);
		}
	}
	/* If user entered more space-separated cmdline startup options than there are arg options available,
	 * then exit as failure.  We wanted to limit how much we iterate i and this is a sensible amount to
	 * limit it to.  It's not necessary to die here but we do so to not give the user the impression that
	 * unparsed argv arguments were saved. */
	if(i > number_of_possible_args && argv[i] != NULL ) {
		die(bad_syntax);
	}

} /* end of void function find_and_save_cmdline_args() */

void execute_informational_program_modes(void)
{
	/* init variables */
	unsigned char i;
	/* informational mode flags mask */
	unsigned char only_informational_bits_mask = program_modes & (HELP_MODE | VERSION_MODE);

	/* If no informational mode flags then return */
	if(!only_informational_bits_mask)
		return;

	/* If VERSION_MODE is the only informational mode flag set in program_modes */
	if((only_informational_bits_mask & program_modes) == VERSION_MODE) {
		printf("%s",version);
		exit(EXIT_SUCCESS);
	}

	/* If HELP_MODE is the only informational mode flag set in program_modes */
	if((only_informational_bits_mask & program_modes) == HELP_MODE) {
		printf("http_server [OPTION]... [FILE]...\n\n"
				"List of valid arguments:\n");
		for(i=0;i < number_of_program_modes && i < 8;i++) {
			printf("-%c, --%s	%s\n",
					*(program_mode_strings[i]),
					program_mode_strings[i],
					program_mode_string_explanations[i]
					);
		}
		for(i=0;i < number_of_file_locations && i < 8;i++) {
			printf("-%c, --%s	%s\n",
					*(file_location_strings[i]),
					file_location_strings[i],
					file_location_string_explanations[i]
					);
		}
		exit(EXIT_SUCCESS);
	}

	/* Multiple informational arguments given, die as bad syntax */
	die(bad_syntax);

} /* end of execute_informational_program_modes */
/* End of cmdline.c */

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
#include	"smsh.h"
#include	"splitline2.h"
#include	"varlib.h"
#include	"process.h"

/**
 **	small-shell version 5
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **
 **     hist: 2017-04-12: changed comment to say "version 5"
 **/

#define	DFL_PROMPT	"> "

static int last_result = 17;                            // to save exit statuses 

void	setup();

int main()
{
	char	*cmdline, *prompt, **arglist;
	int	result;

	prompt = DFL_PROMPT ;
	setup();

	while ( (cmdline = next_cmd(prompt, stdin)) != NULL ) {
		if ( (arglist = splitline(cmdline)) != NULL  ) {
			result = process(arglist);
            last_result = result;                                 // save result
			freelist(arglist);
		}
		free(cmdline);
	}
	return result;
}

int get_last_result()
{
    return last_result;
}

void setup()
/*
 * purpose: initialize shell
 * returns: nothing. calls fatal() if trouble
 */
{
	extern char **environ;

	VLenviron2table(environ);
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n)
{
	fprintf(stderr,"Error: %s,%s\n", s1, s2);
	exit(n);
}

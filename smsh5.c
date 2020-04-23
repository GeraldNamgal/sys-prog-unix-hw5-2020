// Gerald Arocena
// CSCI E-28, Spring 2020
// 4-22-2020
// hw 5

/* *
 * smallsh
 * Implements a version of the Unix shell with a subset of features typically
 * offered in Unix shells. 
 * note: This assignment uses starter code from the hw instructions
 * usage: ./smsh [SCRIPT FILE]
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
#include	"smsh.h"
#include	"splitline2.h"
#include	"varlib.h"
#include	"process.h"
#include    "builtin.h"
#include    <string.h>
#include    "controlflow.h"

/**
 **	small-shell version 5
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **
 **     hist: 2017-04-12: changed comment to say "version 5"
 **/

#define	DFL_PROMPT	"> "
#define BUFF_SIZE 4096

static char* cmdline;

void	setup();

/* *
 * main( int ac, char *av[] )
 * purpose: starts up shell to accept and process commands. If a filename arg is
 * given then runs the commands in the file
 * args: en optional filename
 * rets: exit status code
 */
int main( int ac, char *av[] ) {
	char	*prompt, **arglist;
	int	result;
    FILE *fp = stdin;
    void varsub(char **);

	prompt = DFL_PROMPT ;
	setup();
    init_while_struct();                        // init FLEXLIST in while struct

    if ( *++av != NULL ) {                       // if has argument, script file
        fp = fopen( *av, "r" );                                     // change fp
        if (fp == NULL) {
            fprintf( stderr, "smallsh: error opening file\n" );
            exit(1);
        }
        prompt = "";                                            // change prompt
    }    
    while ( (cmdline = next_cmd(prompt, fp)) != NULL ) {
        check_for_while(cmdline);                                       
        varsub(&cmdline);
        if ( (arglist = splitline(cmdline)) != NULL  ) {
            result = process(arglist);
            freelist(arglist);
        }
        free(cmdline);
    }    
    if ( strcmp( prompt, "" ) == 0 )
        fclose(fp);                          // close file pointer if file input
	
    free_while_struct();
    free_table();
    return result;
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

/* *
 * free_cmdline()
 * purpose: frees the command line. Useful external processes to use
 * args: none
 * rets: none
 */
void free_cmdline()
{
    free(cmdline);
}
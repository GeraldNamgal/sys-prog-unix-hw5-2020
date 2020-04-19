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

static int last_result = 0;                             // to save exit statuses
static int check_for_while(char*);

void	setup();

int main( int ac, char *av[] )
{
	char	*cmdline, *prompt, **arglist;
	int	result;
    FILE *fp = stdin;
    void varsub(char **);

	prompt = DFL_PROMPT ;
	setup();

    if ( *++av != NULL ) {                             // if argument, open file
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
            last_result = result;                                 // save result
            freelist(arglist);
        }
        free(cmdline);
    }    
    if ( strcmp( prompt, "" ) == 0 )
        fclose(fp);                          // close file pointer if file input
	
    return result;
}

int get_last_exit_stat()
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

/* *
 * TODO
 */
int check_for_while(char* cmdline)
{
    int i = 0;
    char while_letters[5] = { 'w', 'h', 'i', 'l', 'e' };

    if ( strlen(cmdline) < MIN_WHILE_CHARS ) 
        return 0;                                       // not even enough chars
    
    while ( cmdline[i] == ' ' || cmdline[i] == '\t' )     // skip leading spaces
        i++;

    for ( int j = 0; j < 5; j++ )                      // check for word 'while'
        if ( cmdline[i] != while_letters[j] )
            return 0;                                              // no 'while'

    if ( cmdline[++i] != ' ' || cmdline[i] != '\t' || cmdline[i] != '\n' )         
        return 0;                                         // no terminating char

    while ( cmdline[i] == ' ' || cmdline[i] == '\t' )       // skip to next char
        i++;

    if ( cmdline[i] == '\0' ) {                    // not at least one argument?
        fprintf(stderr, "smallsh: while: missing argument\n");
        return -1;
    }

    return 0;
}
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
#include	"smsh.h"
#include	"builtin.h"
#include	"varlib.h"
#include	"controlflow.h"
#include	"process.h"
#include    <string.h>
#include    <errno.h>

static int last_result = 0;                             // to save exit statuses

/* process.c
 * command processing layer: handles layers of processing
 * 
 * The process(char **arglist) function is called by the main loop
 * It sits in front of the do_command function which sits 
 * in front of the execute() function.  This layer handles
 * two main classes of processing:
 *	a) process - checks for flow control (if, while, for ...)
 * 	b) do_command - does the command by 
 *		         1. Is command built-in? (exit, set, read, cd, ...)
 *                       2. If not builtin, run the program (fork, exec...)
 *                    - also does variable substitution (should be earlier)
 */

/*
 * purpose: process user command: this level handles flow control
 * returns: result of processing command
 *  errors: arise from subroutines, handled there
 */
int process(char *args[])
{
	int		rv = 0;
    int  exclamation = 0;
    
	if ( args[0] == NULL )
		rv = 0;
    else if ( strcmp( args[0], "!" ) == 0 ) {
        exclamation = 1;
        args++;
    }
	else if ( is_control_command(args[0]) )
		rv = do_control_command(args);
	else if ( ok_to_execute() )
		rv = do_command(args);
    if ( exclamation == 1 ) {
        printf("%d\n", rv);
        rv = !rv;
        printf("%d\n", rv);
    }
    last_result = rv;

	return rv;                                  // return exit status of command
}

/*
 * do_command
 *   purpose: do a command - either builtin or external
 *   returns: result of the command
 *    errors: returned by the builtin command or from exec,fork,wait
 *      note: this version does variable substitution: not where sh does it
 *
 */
int do_command(char **args)
{
	int  is_builtin(char **, int *);
	int  rv;    
    
	if ( !is_builtin(args, &rv) )		/* if not done by shell	*/ 
		rv = execute(args);		/* fork and exec it	*/    

	return rv;				
}

/*
 * purpose: run a program passing it arguments
 * returns: status returned via wait, or -1 on error
 *  errors: -1 on fork() or wait() errors
 *    NOTE: this does not return the exit(n) status from child
 *	    Instead it returns the status word wait() receives
 *	    To get the exit(n) status, you must add some code. Also, referenced:
 *      https://www.geeksforgeeks.org/wait-system-call-c/
 */
int execute(char *argv[])
{
	extern char **environ;		/* note: declared in <unistd.h>	*/
	int	pid ;
	int	child_info = -1;
    int exit_status = -1;

	if ( argv[0] == NULL )		/* nothing succeeds		*/
		return 0;

	if ( (pid = fork())  == -1 )
		perror("fork");

	else if ( pid == 0 ) {                                              // child
		environ = VLtable2environ();
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execvp(argv[0], argv);                 // if successful, does not return
		perror("cannot execute command");
        if ( errno == ENOENT )
            _exit(127);
        else if ( errno == EACCES )
            _exit(126);
        else
		    _exit(1);
	}
	else {
		if ( wait( &child_info ) == -1 )
			perror("wait failed");
	}
    if ( WIFEXITED( child_info ) ) { 
        exit_status = WEXITSTATUS( child_info );
    }
    else if ( WIFSIGNALED( child_info ) ) {
        exit_status = WTERMSIG( child_info );
    }
	return exit_status;
}

int get_last_exit_stat()
{
    return last_result;
}
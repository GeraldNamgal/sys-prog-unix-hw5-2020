/* controlflow.c
 *
 * "if" processing is done with two state variables
 *    if_state and if_result
 */
#include	<stdio.h>
#include 	<string.h>
#include	"smsh.h"
#include	"process.h"
#include    "controlflow.h"
#include	"flexstr2.h"
#include    <stdlib.h>

static struct while_loop whileloop;

enum states   { NEUTRAL, WANT_THEN, THEN_BLOCK, ELSE_BLOCK, WANT_DO
                , WANT_BODY, WANT_DONE };
enum results  { SUCCESS, FAIL };

static int if_state  = NEUTRAL;
static int if_result = SUCCESS;
static int last_stat = 0;

int	syn_err(char *);
int is_while(char* cmdline, int*);
char* get_first_arg( char * );
int execute_while();
void free_while_struct();
void add_to_while( char * );

/*
 * purpose: determine the shell should execute a command
 * returns: 1 for yes, 0 for no
 * details: if in THEN_BLOCK and if_result was SUCCESS then yes
 *          if in THEN_BLOCK and if_result was FAIL    then no
 *          if in WANT_THEN  then syntax error (sh is different)
 */
int ok_to_execute()
{
	int	rv = 1;		/* default is positive */

	if ( if_state == WANT_THEN ) {
		syn_err("then expected");
		rv = 0;
	}
    else if ( if_state == WANT_DO ) {
        syn_err("do expected");
        rv = 0;
    }
    else if ( if_state == WANT_BODY ) {
        if_state = WANT_DONE;
        rv = 0;
    }
    else if ( if_state == WANT_DONE ) {
        rv = 0;
    }
	else if ( if_state == THEN_BLOCK && if_result == SUCCESS )
		rv = 1;
	else if ( if_state == THEN_BLOCK && if_result == FAIL )
		rv = 0;
    else if ( if_state == ELSE_BLOCK && if_result == FAIL )
        rv = 1;
    else if ( if_state == ELSE_BLOCK && if_result == SUCCESS )  
        rv = 0;
	
    return rv;
}

int is_control_command(char *s)
/*
 * purpose: boolean to report if the command is a shell control command
 * returns: 0 or 1
 */
{
    return (strcmp(s,"if")==0 || strcmp(s,"then")==0 || strcmp(s,"fi")==0
            || strcmp(s,"else")==0 || strcmp(s,"while")==0
            || strcmp(s,"do")==0 || strcmp(s,"done")==0);
}

int do_control_command(char **args)
/*
 * purpose: Process "if", "then", "fi" - change state or detect error
 * returns: 0 if ok, -1 for syntax error
 *   notes: I would have put returns all over the place, Barry says "no"
 */
{
	char	*cmd = args[0];
	int	rv = -1;

	if( strcmp(cmd,"if")==0 ){
		if ( if_state != NEUTRAL )
			rv = syn_err("if unexpected");
		else {
			last_stat = process(args+1);
			if_result = (last_stat == 0 ? SUCCESS : FAIL );            
			if_state = WANT_THEN;            
			rv = 0;
		}
	}
	else if ( strcmp(cmd,"then")==0 ){
		if ( if_state != WANT_THEN )
			rv = syn_err("then unexpected");
		else {
			if_state = THEN_BLOCK;
			rv = 0;
		}
	}
    else if ( strcmp(cmd, "else")==0 ) {
        if ( if_state != THEN_BLOCK )
            rv = syn_err("else unexpected");
        else {
            if_state = ELSE_BLOCK;
            rv = 0;
        }
    }
	else if ( strcmp(cmd,"fi")==0 ){
		if ( !( if_state == THEN_BLOCK || if_state == ELSE_BLOCK ) )
			rv = syn_err("fi unexpected");
		else {
			if_state = NEUTRAL;
			rv = 0;
		}
	}
    else if ( strcmp(cmd,"while")==0 ){
		if ( if_state != NEUTRAL )
			rv = syn_err("while unexpected");
		else {
			if_state = WANT_DO;
			rv = 0;
		}
	}
    else if ( strcmp(cmd, "do")==0 ) {
        if ( if_state != WANT_DO )
            rv = syn_err("do unexpected");
        else {
            if_state = WANT_BODY;
            rv = 0;
        }
    }
    else if ( strcmp(cmd, "done")==0 ) {
        if ( if_state != WANT_DONE )
            rv = syn_err("done unexpected");
        else {
            if_state = NEUTRAL;            
            rv = execute_while();
        }
    }
	else 
		fatal("internal error processing:", cmd, 2);
	return rv;
}

int syn_err(char *msg)
/* purpose: handles syntax errors in control structures
 * details: resets state to NEUTRAL
 * returns: -1 in interactive mode. Should call fatal in scripts
 */
{
	if_state = NEUTRAL;
	fprintf(stderr,"syntax error: %s\n", msg);
	return -1;
}

/* *
 * TODO
 */
void check_for_while(char* cmdline)
{
    int i = 0;

    if ( if_state == NEUTRAL ) {                      
       
        if ( is_while( cmdline, &i ) ) {                  // is command 'while'?
            free_while_struct();                                 // clear struct            
            if ( cmdline[i] == '\0' )                       // save condition...
                whileloop.condition = strdup(""); 
            else
                whileloop.condition = strdup(cmdline + i);
        }
    }
    else if ( if_state == WANT_BODY || if_state == WANT_DONE ) {

        char* first_arg = NULL;
        first_arg = get_first_arg( cmdline );
        if ( first_arg == NULL ) {
            fprintf( stderr, "smallsh: could not save while command\n" );
            return;
        }
        if (is_control_command(first_arg)) {
            // TODO: nested ifs and whiles
            
        }
        else                                       // else not a control command
            add_to_while( cmdline );
    }
}

/* *
 * 
 */
int is_while(char* cmdline, int* i)
{
    char while_letters[MIN_WHILE_CHARS] = { 'w', 'h', 'i', 'l', 'e' };

    if ( strlen(cmdline) < MIN_WHILE_CHARS ) 
        return 0;                                       // not even enough chars
    
    while ( cmdline[*i] == ' ' || cmdline[*i] == '\t' )   // skip leading spaces
        ++*i;

    for ( int j = 0; j < MIN_WHILE_CHARS; j++ ) {      // check for word 'while'
        if ( cmdline[*i] != while_letters[j] )
            return 0;                                              // no 'while'
        ++*i;                                              
    }

    if ( cmdline[*i] != ' ' && cmdline[*i] != '\t' && cmdline[*i] != '\n'
        && cmdline[*i] != '\0' && cmdline[*i] != '\r' )         
            return 0;                                     // no terminating char

    while ( cmdline[*i] == ' ' || cmdline[*i] == '\t' )     // skip to next char
        ++*i;  

    return 1;
}

/* *
 *
 */
char* get_first_arg( char *cmdline )
{
    int i = 0;
    FLEXSTR s;

    while ( cmdline[i] == ' ' || cmdline[i] == '\t' || cmdline[i] == '\n'
            || cmdline[i] == '\r' )                       // skip leading spaces
                i++;

    if ( cmdline[i] == '\0' )
        return strdup("");                                            // no args

    else {
        fs_init( &s, 0 );                                // getting first arg...
        while ( cmdline[i] != ' ' && cmdline[i] != '\t' && cmdline[i] != '\n'
                && cmdline[i] != '\0' && cmdline[i] != '\r' ) {      
                    fs_addch( &s, cmdline[i] );
                    i++;
        }
        fs_addch( &s, '\0' );
        
        return strdup( fs_getstr(&s) );
    }
    
    return NULL;
}

/* *
 *
 */
void free_while_struct()
{
    if ( whileloop.condition ) {
        free( whileloop.condition );
        whileloop.condition = NULL;
    }
    if ( whileloop.body ) {
        fl_free( &whileloop.body );
        whileloop.body = NULL;
    }
}

/* *
 * TODO
 */
void add_to_while( char* cmdline )
{
    if (!whileloop.body) {      // initialize if NULL
        // TODO
    }
    else (  )

}

/* *
 * TODO
 */
int execute_while()
{


    return 0;
}
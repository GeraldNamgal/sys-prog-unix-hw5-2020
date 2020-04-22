// Gerald Arocena
// CSCI E-28, Spring 2020
// 4-22-2020
// hw 5

/* controlflow.c
 *
 * "if" processing is done with two state variables
 *    if_state and if_result
 * "while" processing is done with 
 *    while_state 
 * Also inside_an_if and inside_a_while check if program is in the middle of
 * incomplete syntax
 * 
 */
#include	<stdio.h>
#include 	<string.h>
#include	"smsh.h"
#include	"process.h"
#include    "controlflow.h"
#include	"flexstr2.h"
#include    <stdlib.h>
#include    "builtin.h"
#include    "splitline2.h"

static struct while_loop whileloop;

enum states   { NEUTRAL, WANT_THEN, THEN_BLOCK, ELSE_BLOCK, WANT_DO
                , WANT_BODY, WANT_DONE };
enum results  { SUCCESS, FAIL };

static int if_state  = NEUTRAL;
static int while_state = NEUTRAL;
static int if_result = SUCCESS;
static int last_stat = 0;
static FLEXLIST strings;                   // to hold the body of the while loop
static int inside_a_while = 0;       // is process in unfinished 'while' syntax?
static int inside_an_if = 0;             // is process in unfinished 'if' syntax

int	syn_err(char *, int);
int is_while(char* cmdline, int*);
char* get_first_arg( char * );
int execute_while();
void add_to_while( char * );
int run_command(char *);

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
		syn_err("then expected", IF_ERROR);
		rv = 0;
	}
    else if ( while_state == WANT_DO ) {
        syn_err("do expected", WHILE_ERROR);
        rv = 0;
    }
    else if ( while_state == WANT_BODY ) {
        while_state = WANT_DONE;
        rv = 0;
    }
    else if ( while_state == WANT_DONE ) {
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
 * purpose: Process control commands - change state or detect error
 * returns: 0 if ok, -1 for syntax error
 *   notes: I would have put returns all over the place, Barry says "no"
 */
{
	char	*cmd = args[0];
	int	rv = -1;

	if( strcmp(cmd,"if")==0 ){
		if ( if_state != NEUTRAL || while_state == WANT_DO )
			rv = syn_err("if unexpected", IF_ERROR);
		else {
			last_stat = process(args+1);
			if_result = (last_stat == 0 ? SUCCESS : FAIL );            
			if_state = WANT_THEN;            
			rv = 0;
            inside_an_if = 1;                                            // flag              
		}
	}
	else if ( strcmp(cmd,"then")==0 ) {
		if ( if_state != WANT_THEN )
			rv = syn_err("then unexpected", IF_ERROR);
		else {
			if_state = THEN_BLOCK;
			rv = 0;
		}
	}
    else if ( strcmp(cmd, "else")==0 ) {
        if ( if_state != THEN_BLOCK )
            rv = syn_err("else unexpected", IF_ERROR);
        else {
            if_state = ELSE_BLOCK;
            rv = 0;
        }
    }
	else if ( strcmp(cmd,"fi")==0 ){
		if ( !( if_state == THEN_BLOCK || if_state == ELSE_BLOCK ) )
			rv = syn_err("fi unexpected", IF_ERROR);
		else {
			if_state = NEUTRAL;
			rv = 0;
            inside_an_if = 0;                                            // flag
		}
	}
    else if ( strcmp(cmd,"while")==0 ){
		if ( while_state != NEUTRAL || if_state == WANT_THEN )
			rv = syn_err("while unexpected", WHILE_ERROR);
		else {
		    while_state = WANT_DO;
			rv = 0;
            inside_a_while = 1;                                          // flag
		}
	}
    else if ( strcmp(cmd, "do")==0 ) {
        if ( while_state != WANT_DO )
            rv = syn_err("do unexpected", WHILE_ERROR);
        else {
            while_state = WANT_BODY;
            rv = 0;
        }
    }
    else if ( strcmp(cmd, "done")==0 ) {
        if ( while_state != WANT_DONE )
            rv = syn_err("done unexpected", WHILE_ERROR);
        else {
            while_state = NEUTRAL;            
            rv = execute_while();                    // while loop executed here
            inside_a_while = 0;                                          // flag
        }
    }
	else 
		fatal("internal error processing:", cmd, 2);
	return rv;
}

int syn_err(char *msg, int error)
/* purpose: handles syntax errors in control structures
 * details: resets state to NEUTRAL and not inside incomplete syntax anymore
 * returns: -1 in interactive mode. Should call fatal in scripts
 */
{
    if_state = NEUTRAL;    
    while_state = NEUTRAL;
    inside_a_while = 0;
    inside_an_if = 0;
    
	fprintf(stderr,"syntax error: %s\n", msg);

	return -1;
}

/* *
 * check_for_while(char* cmdline)
 * purpose: Checks if raw cmdline is a while command and saves it in struct if
 * so and if control state is appropriate
 * args: a command line (from main)
 * rets: none
 */
void check_for_while(char* cmdline)
{
    int i = 0;

    if ( while_state == NEUTRAL ) {                      
       
        if ( is_while( cmdline, &i ) ) {                  // is command 'while'?
            free_while_struct();                         // clear struct for new            
            if ( cmdline[i] == '\0' )                       // save condition...
                whileloop.condition = strdup(""); 
            else
                whileloop.condition = strdup(cmdline + i);
        }
    }
    else if ( while_state == WANT_BODY || while_state == WANT_DONE ) {

        char* first_arg = NULL;
        first_arg = get_first_arg( cmdline );
        if ( first_arg == NULL ) {
            fprintf( stderr, "smallsh: could not save while command\n" );
            return;
        }
        if ( is_control_command( first_arg ) ) {  // can have an 'if' in a while
            if (strcmp("if", first_arg)==0 || strcmp("then", first_arg) == 0
                || strcmp("else", first_arg)==0 || strcmp("fi", first_arg) == 0)
                    fl_append( whileloop.body , strdup(cmdline));         
        }
        else                                       // else not a control command
            fl_append( whileloop.body , strdup(cmdline) );
       
        free (first_arg);
    }
}

/* *
 * is_while(char* cmdline, int* i)
 * purpose: utility function for check_for_while(). Checks if command is a
 * 'while' command and moves a given index to its args if so
 * args: the command, the index
 * rets: 1 if true, 0 if false
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
 * get_first_arg( char *cmdline )
 * purpose: utility function for check_for_while(). Gets the first arg of a
 * string
 * args: the string / command 
 * rets: the first arg
 */
char* get_first_arg( char *cmdline )
{    
    int i = 0;
    FLEXSTR s;

    while ( cmdline[i] == ' ' || cmdline[i] == '\t' || cmdline[i] == '\n'
            || cmdline[i] == '\r' )                       // skip leading spaces
                i++;

    if ( cmdline[i] == '\0' )
        return strdup("");                      // no args, return emppty string

    else {
        fs_init( &s, 0 );                                // getting first arg...
        while ( cmdline[i] != ' ' && cmdline[i] != '\t' && cmdline[i] != '\n'
                && cmdline[i] != '\0' && cmdline[i] != '\r' ) {      
                    fs_addch( &s, cmdline[i] );
                    i++;
        }
        fs_addch( &s, '\0' );
        
        char* ret_value = strdup( fs_getstr(&s) );
        free( fs_getstr(&s) );
        return ret_value;
    }
    
    return NULL;
}

/* *
 * free_while_struct()
 * purpose: frees the while_loop struct
 * args: none
 * rets: none
 */
void free_while_struct()
{
    if ( whileloop.condition ) {
        free( whileloop.condition );
        whileloop.condition = NULL;
    }
    
    if ( fl_getlist( whileloop.body ) )
        fl_free( whileloop.body );
}

/* *
 *
 * purpose: initializes the while_loop struct's body which uses a FLEXLIST
 * args: none
 * rets: none
 */
void init_while_struct()
{
    fl_init( &strings, 0 );
    whileloop.body = &strings;
}

/* *
 * execute_while()
 * purpose: executes the while loop struct
 * args: none
 * rets: exit status of last command in while loop run
 */
int execute_while()
{
    char** while_body = fl_getlist( whileloop.body );
    int result = 0;

    while ( run_command( whileloop.condition ) == 0 ) {
        for ( int i = 0; i < fl_getcount( whileloop.body ); i++ )
            result = run_command( while_body[i] );        
    }

    return result; 
}

/* *
 * run_command(char* command)
 * purpose: utility function for execute_while(). Executes a cmd string
 * args: the command
 * rets: the exit status of the command
 */
int run_command(char* command)
{
    char    **arglist;
	int	result = 0;
    
    char* cmd = strdup( command );
    varsub(&cmd);
    if ( (arglist = splitline(cmd)) != NULL  ) {
        result = process(arglist);
        freelist(arglist);
    }
    free (cmd);

    return result;
}

/* *
 * get_inside_a_while()
 * purpose: returns if the program is in the middle of incomplete 'while' syntax
 * args: none
 * rets: inside_a_while which stores the status of the syntax
 */
int get_inside_a_while()
{
    return inside_a_while;
}

/* *
 * get_inside_an_if()
 * purpose: returns if the program is in the middle of incomplete 'if' syntax
 * args: none
 * rets: inside_an_if which stores the status of the syntax
 */
int get_inside_an_if()
{
    return inside_an_if;
}
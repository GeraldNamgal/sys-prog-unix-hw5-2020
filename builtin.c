/* builtin.c
 * contains the switch and the functions for builtin commands
 */

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	"smsh.h"
#include	"varlib.h"
#include	"builtin.h"
#include    <unistd.h>
#include    <stdbool.h>
#include	"flexstr2.h"

static bool is_number(char *);
static void handle_dollar(FLEXSTR*, char*,int *);
static char *is_var_name(char *, int *);

/*
 * purpose: run a builtin command 
 * returns: 1 if args[0] is builtin, 0 if not
 * details: test args[0] against all known builtins.  Call functions
 */
int is_builtin(char **args, int *resultp)
{
	if ( is_assign_var(args[0], resultp) )
		return 1;
	if ( is_list_vars(args[0], resultp) )
		return 1;
	if ( is_export(args, resultp) )
		return 1;
    if ( is_cd(args, resultp) )
        return 1;
    if ( is_exit(args, resultp) )
        return 1;
    if ( is_read(args, resultp) )
        return 1;
    if ( is_unset(args, resultp) ) {
        return 1;
    }
	return 0;
}

/* checks if a legal assignment cmd
 * if so, does it and retns 1
 * else return 0
 */
int is_assign_var(char *cmd, int *resultp)
{
	if ( strchr(cmd, '=') != NULL ){
		*resultp = assign(cmd);
		if ( *resultp != -1 )
			return 1;
	}
	return 0;
}

/* checks if command is "set" : if so list vars */
int is_list_vars(char *cmd, int *resultp)
{
	if ( strcmp(cmd,"set") == 0 ){	     /* 'set' command? */
		VLlist();
		*resultp = 0;
		return 1;
	}
	return 0;
}

int is_unset(char **args, int *resultp)
{
    if ( strcmp(args[0],"unset") == 0 ) {                 /* 'unset' command? */
        if ( args[1] != NULL )
            *resultp = VLdelete(args[1]);
		else
            *resultp = -1;                           // flag error (missing arg)
		return 1;                                  // return was 'unset' command
	}
    return 0;                                  // return was not 'unset' command
}

/*
 * if an export command, then export it and ret 1
 * else ret 0
 * note: the opengroup says
 *  "When no arguments are given, the results are unspecified."
 */
int is_export(char **args, int *resultp)
{
	if ( strcmp(args[0], "export") == 0 ) {
		if ( args[1] != NULL && okname(args[1]) )
			*resultp = VLexport(args[1]);
		else
			*resultp = 1;
		return 1;
	}
	return 0;
}

/* *
 * is_cd( char **args, int *resultp )
 */
int is_cd( char **args, int *resultp )
{
    if ( strcmp(args[0], "cd") == 0 ) {
        if ( args[1] != NULL ) {                      // there are args included
            if ( chdir( args[1] ) == 0 )
                *resultp = 0;                         // changed dir succesfully            
            else {
                *resultp = -1;                       // error changing directory
                fprintf( stderr, "smallsh: cd: can't cd to %s\n", args[1] );
            }
        }            
        else {                                     // else no args, just command
            if ( chdir( getenv( "HOME" ) ) == 0 )    // change to home directory
                *resultp = 0;                         // changed dir succesfully   
            else {
                *resultp = -1;                       // error changing directory
                fprintf( stderr, "smallsh: cd: can't cd to %s\n"
                        , getenv( "HOME" ) );
            }
        }
        return 1;                                    // return it's a cd command
    }
    return 0;                                    // return it's not a cd command
}

int is_exit( char **args, int *resultp )
{
    if ( strcmp( args[0], "exit" ) == 0 ) {
        if ( args[1] != NULL ) {                      // there are args included
            if ( is_number( args[1] ) ) {                          // valid num?
                *resultp = 0;                                     // flag sucess
                exit( atoi( args[1] ) );            // exit passing in first arg
            }           
            else {                                       // else not a valid num
                *resultp = -1;                                     // flag error 
                fprintf(stderr, "smallsh: exit: Illegal number: %s\n", args[1]);
            }
        }            
        else {                                     // else no args, just command
            *resultp = 0;                                        // flag success
            exit( get_last_exit_stat() );
        }
        return 1;                                 // return it's an exit command
    }
    return 0;                                 // return it's not an exit command
}

/* *
 *
 * purpose: utility function
 * note: code referenced
 * https://www.geeksforgeeks.org/program-check-input-integer-string/
 */
bool is_number(char *str)
{
    for ( int i = 0; i < strlen(str); i++ ) 
        if ( isdigit(str[i]) == false ) 
            return false; 
  
    return true;
}

int is_read( char **args, int *resultp ) {
    if ( strcmp( args[0], "read" ) == 0 ) {
        if ( args[1] == NULL ) {                          // required arg given?
            *resultp = -1;                                         // flag error
            fprintf( stderr, "smallsh: read: arg count\n" );
        }
        else {
            char buff[VAR_MAX_CHARS];            
            fgets(buff, VAR_MAX_CHARS, stdin);                 // get user input
            buff[strcspn ( buff, "\n" )] = '\0';    // remove newline from input
            
            char *name_val_pair;                             // concatenating...
            name_val_pair = malloc( strlen(buff) + strlen(args[1]) + 2 );
            if( name_val_pair == NULL ) {                    // if malloc failed
                *resultp = -1;                                     // flag error
                fprintf( stderr, "smallsh: read: malloc error\n" );
                return 1;
            }
            strcat( strcpy( name_val_pair, args[1] ), "=" );       // do concats 
            strcat( name_val_pair, buff );               

            int ret_value = assign( name_val_pair );      // assign input to var
            free(name_val_pair);                                  // free malloc
            if ( ret_value == -1 ) {
                *resultp = -1;                                     // flag error
                fprintf( stderr, "smallsh: read: %s: bad variable name\n"
                        , args[1] );                 
                return 1;
            }                                               
            *resultp = ret_value;                // flag success or not (0 or 1)
        }
        return 1;                                  // return it's a read command
    }
    return 0;                                  // return it's not a read command
}

/*
 * purpose: execute name=val AND ensure that name is legal
 * returns: -1 for illegal lval, or result of VLstore 
 * warning: modifies the string, but retores it to normal
 */
int assign(char *str)
{
	char	*cp;
	int	rv ;

	cp = strchr(str,'=');
	*cp = '\0';
	rv = ( okname(str) ? VLstore(str,cp+1) : -1 );
	*cp = '=';
	return rv;
}

int okname(char *str)
/*
 * purpose: determines if a string is a legal variable name
 * returns: 0 for no, 1 for yes
 */
{
	char	*cp;

	for(cp = str; *cp; cp++ ){
		if ( (isdigit(*cp) && cp==str) || !(isalnum(*cp) || *cp=='_' ))
			return 0;
	}
	return ( cp != str );	/* no empty strings, either */
}

/*
 * TODO
 * 
 * note: referenced
 *       https://stackoverflow.com/questions/9655202/how-to-convert-integer-to-
 *       string-in-c
 */
void varsub(char **args)
{
	int	i, j, dollar_found = 0;
	char *cmdline = *args;
    FLEXSTR s;
   
    for ( i = 0; cmdline[i] != '\0'; i++ ) {
        if ( cmdline[i] != '$' && dollar_found )       // new string being spun?
            fs_addch(&s, cmdline[i]);                       // add to new string
        else if ( cmdline[i] == '$' ) {
            if ( !dollar_found ) {                  // '$' not found previously?
                dollar_found = 1;                               // flag as found
                fs_init(&s, 0);                               // init new string
                for ( j = 0; j < i; j++ )            // fill new with old string
                    fs_addch(&s, cmdline[j]);   
            }            
            j = i + 1;                                // index to just after '$'         
            handle_dollar(&s, cmdline, &j);
            i = j - 1;                                  // update index position            
        }
    }
    if (dollar_found) {                        // '$' found (spun a new string)?
        free(*args);                             // change args to new string...
        *args = strdup( fs_getstr(&s) );              
        fs_free ( &s );                                       // free new string
    }    
}

static void handle_dollar(FLEXSTR *s, char *cmdline, int *j)
{
    if ( isdigit( cmdline[*j] ) )                            // a number?
        while ( isdigit( cmdline[++*j] ) ) {}         // skip over digits
    else if ( isalnum( cmdline[*j] ) || cmdline[*j] == '_' )  // var name?
        fs_addstr(s, is_var_name(cmdline, &*j) );  // put val in new str
    else if ( cmdline[*j] == '$' && ++*j ) {
        char num_string[MAX_PID_DIGITS];
        sprintf( num_string, "%d", getpid() );
        fs_addstr(s, num_string);              
    }
    else if ( cmdline[*j] == '?' && ++*j ) {
        char num_string[MAX_EXSTATUS_DIG];
        sprintf( num_string, "%d", get_last_exit_stat() );
        fs_addstr(s, num_string);       
    }
    else                                      // else just a dollar sign
        fs_addch(s, '$');              
}

/* *
 * 
 * purpose: utility function that varsub() uses
 */
static char* is_var_name( char* cmdline, int* j )
{
    FLEXSTR buff;
    char *newstr;
                
    fs_init(&buff, 0);                                    // init temporary buff
    fs_addch(&buff, cmdline[*j]);                  // adding var name to buff...
    while ( isalnum( cmdline[++*j] ) || cmdline[*j] == '_' )
        fs_addch(&buff, cmdline[*j]);
    fs_addch(&buff, '\0');
    
    newstr = VLlookup( fs_getstr(&buff) );                // looking up value...
    if ( newstr == NULL )
        newstr = "";                
    
    fs_free( &buff );                                     // free temporary buff

    return newstr;                                   // return value of var name
}


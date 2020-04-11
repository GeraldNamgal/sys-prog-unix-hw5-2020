/* splitline2.c - commmand reading and parsing functions for smsh
 *    
 *    char *next_cmd(char *prompt, FILE *fp) - get next command
 *    char **splitline(char *str);           - parse a string
 *    void freelist(char **);                - free list ret'ed by splitline
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"splitline2.h"
#include	"smsh.h"
#include	"flexstr2.h"
#include    <stdbool.h>

/*
 * purpose: read next command line from fp
 * returns: dynamically allocated string holding command line
 *  errors: NULL at EOF (not really an error)
 *          calls fatal from emalloc()
 *   notes: allocates space in BUFSIZ chunks.  
 */
char * next_cmd(char *prompt, FILE *fp) {
	int	c;				                                  /* input char		*/
	FLEXSTR	s;				                             /* the command		*/
	int	pos = 0;
    char prev_char = ' ';
    bool found_comment = false; 

	fs_init(&s, 0);				                     /* initialize the str	*/
	printf("%s", prompt);				                  /* prompt user	*/
	while( ( c = getc(fp)) != EOF ) {        
		if ( c == '\n' )                                 /* end of command? */
			break;

        if ( found_comment == true )
            continue;

        if ( c == '#' && ( prev_char == ' ' || prev_char == '\t' ) ) {
            found_comment = true;
            continue;
        } 
		
		fs_addch(&s, c);                                 /* no, add to buffer */
		pos++;

        prev_char = c;
	}

	if ( c == EOF && pos == 0 )		/* EOF and no input	*/
		return NULL;			/* say so		*/
	
    fs_addch(&s, '\0');			/* terminate string	*/
	
    return fs_getstr(&s);			/* get copy of string	*/
}

/**
 **	splitline ( parse a line into an array of strings )
 **/
#define	is_delim(x) ((x)==' '||(x)=='\t')

/*
 * purpose: split a line into array of white-space separated tokens
 * returns: a NULL-terminated array of pointers to copies of the tokens
 *          or NULL if line is NULL.
 *          (If no tokens on the line, then the array returned by splitline
 *           contains only the terminating NULL.)
 *  action: traverse the array, locate strings, make copies, add to FlexList
 *    note: strtok() could work, but we may want to add quotes later
 */
char ** splitline(char *line)
{
	char	*newstr();
	int	start;
	int	len;
	int	i=0;
	FLEXLIST strings;
	char	**parts;

	if ( line == NULL )			/* handle special case	*/
		return NULL;

	fl_init(&strings,0);

	while( line[i] != '\0' )
	{
		while ( is_delim(line[i]) )	/* skip leading spaces	*/
			i++;
		if ( line[i] == '\0' )		/* end of string? 	*/
			break;			/* yes, get out		*/

		/* mark start, then find end of word */
		start = i++;
		len   = 1;
		while ( line[i] != '\0' && !(is_delim(line[i])) )
			i++, len++;
		fl_append(&strings, newstr(&line[start], len));
	}
	parts = fl_getlist(&strings);	/* get array of those strings	*/
	return parts;
}

/*
 * purpose: constructor for strings
 * returns: a string, never NULL
 *    args: start of string, length to copy
 */
char *newstr(char *s, int l)
{
	char *rv = emalloc(l+1);

	rv[l] = '\0';
	strncpy(rv, s, l);
	return rv;
}

/*
 * purpose: free the list returned by splitline
 * returns: nothing
 *  action: free all strings in list and then free the list
 */
void freelist(char **list)
{
	fl_freelist(list);
}

void *emalloc(size_t n)
{
	void *rv ;
	if ( (rv = malloc(n)) == NULL )
		fatal("out of memory","",1);
	return rv;
}

void *erealloc(void *p, size_t n)
{
	void *rv;
	if ( (rv = realloc(p,n)) == NULL )
		fatal("realloc() failed","",1);
	return rv;
}


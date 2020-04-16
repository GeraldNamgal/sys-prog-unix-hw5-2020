#include	<stdlib.h>
#include	<stdio.h>
#include	"flexstr2.h"

/*
 * flexstr2: 2020-04-03
 * FS_2: make getlist work like getstr in that it automatically appends
 *	 a closing NULL so that freelist can be used safely.
 *	 It never hurts to have a NULL at the end of a list of pointers.
 */
#include	"splitline2.h"
/*
 * flexstr.c -- a set of functions for handling flexlists and flexstrings
 *
 *	a flexlist is an array of strings that grows as needed
 *	methods are:  
 *
 *	fl_init(FLEXLIST *p,int chunk)	- initialize a FLEXLIST
 *	fl_append(FLEXLIST *p, char *)	- add a string to an FLEXLIST
 *	char ** fl_getlist(FLEXLIST *p)	- return array of strings in the list
 *	                                  this array has a NULL at end
 *	fl_freelist( char ** )		- free what fl_getlist returns
 *	fl_free(FLEXLIST *p)		- dispose of all malloced data therein
 *	fl_getcount(FLEXLIST *p)		- return the number of items
 *
 *	note: you pass ptrs to add to the flexlist.  When you call getlist
 *	you get a dynamcally allocated array of those pointers.
 *	If the pointers point to malloc-ed memory, then you can use freelist
 *	to loop along until a NULL and free them.  
 *
 *      a flexstring a string that grows as needed
 *
 *	fs_init(FLEXSTR *p,int chunk)
 *	fs_addch(FLEXSTR *p, char c)
 *	fs_addstr(FLEXSTR *p, char *str)
 *	char *fs_getstr(FLEXSTR *p)	- need to free this value later
 *	fs_free(FLEXSTR *p)
 */



void 
fl_init(FLEXLIST *p, int amt)
{
	p->fl_list = NULL;
	p->fl_nslots = p->fl_nused = 0;
	p->fl_growby = ( amt > 0 ? amt : CHUNKSIZE );
}

int
fl_getcount(FLEXLIST *p)
{
	return p->fl_nused;
}

void
fl_free(FLEXLIST *p)
{
	int	i;

	for(i=0; i < p->fl_nused; i++)
		free(p->fl_list[i]);
	if ( p->fl_list )
		free(p->fl_list);
	fl_init(p,p->fl_growby);
}
/*
 * free the pointers and the list that contains them
 * assumes NULL at end of list.
 */
void
fl_freelist( char **lst_of_ptrs )
{
	int	i;

	if ( lst_of_ptrs != NULL ){
		for ( i = 0 ; lst_of_ptrs[i] != NULL ; i++ )
			free( lst_of_ptrs[i] );
		free( lst_of_ptrs );
	}
}

char **
fl_getlist(FLEXLIST *p)
{
	return p->fl_list;
}

/*
 * append string str to strlist, reallocing the array if needed
 * return 0 for ok dies on error
 */

int
fl_append(FLEXLIST *p, char *str)
{
	if ( p->fl_nslots == 0 ){
		p->fl_list  = emalloc(p->fl_growby * sizeof(char *));
		p->fl_nused = 0;
		p->fl_nslots= p->fl_growby;
	}
	else if ( p->fl_nused+1 == p->fl_nslots ){	/* +1 for NULL	*/
		p->fl_nslots += p->fl_growby;
		p->fl_list    = erealloc(p->fl_list, 
					 p->fl_nslots * sizeof(char *));
	}
	p->fl_list[p->fl_nused++] = str;
	p->fl_list[p->fl_nused  ] = NULL;	/* always add NULL ptr	*/
	return 0;
}


/************************************************************************
 *
 * flexstr functions
 *
 ************************************************************************/

void 
fs_init(FLEXSTR *p, int amt)
{
	p->fs_str = NULL;
	p->fs_space = p->fs_used = 0;
	p->fs_growby = ( amt > 0 ? amt : CHUNKSIZE );
}


void
fs_free(FLEXSTR *p)
{

	free(p->fs_str);
}

char *fs_getstr(FLEXSTR *p)
{
    /* nul-terminate the string before returning it*/

    /* First make sure there's room for the '\0' */
    if (p->fs_used == p->fs_space)
    { /* string is full, add room for one more char */
        p->fs_str = erealloc(p->fs_str, ++p->fs_space);
    }

    /* Add terminating '\0'.  Don't increment fs_used -- the '\0' is not
     * part of the string, and shouldn't be counted if someone wants to
     * continue adding characters to the string later.
     */
    p->fs_str[p->fs_used] = '\0';

    /* Now return the (terminated) string. */
	return p->fs_str;
}

/*
 * append char to flexstring, reallocing the array if needed
 * return 0 for ok dies on error
 */

int
fs_addch(FLEXSTR *p, char c)
{
	if ( p->fs_space == 0 ){
		p->fs_str  = emalloc(p->fs_growby);
		p->fs_used = 0;
		p->fs_space= p->fs_growby;
	}
	else if ( p->fs_used == p->fs_space ){
		p->fs_space += p->fs_growby;
		p->fs_str    = erealloc(p->fs_str, p->fs_space);
	}
	p->fs_str[p->fs_used++] = c;
	return 0;
}

int
fs_addstr(FLEXSTR *p, char *s)
{
	int	c;

	while( (c = *s++) != '\0' )
		fs_addch(p, c);
	return 0;
}

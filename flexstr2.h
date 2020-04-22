// Gerald Arocena
// CSCI E-28, Spring 2020
// 4-22-2020
// hw 5

#ifndef	FLEXSTR_H
#define	FLEXSTR_H

/*
 * flexstr2.h -- a set of functions for handling flexlists and flexstrings
 *
 *	a flexlist is an object that stores a list of strings
 *	methods are:  
 *
 *	fl_init(FLEXLIST *p,int amt)	- initialize a FLEXLIST
 *	fl_append(FLEXLIST *p, char *)	- add a str (no copy) to a FLEXLIST
 *					  string passed must be from malloc()
 *	char ** fl_getlist(FLEXLIST *p)	- return array of strings in the list
 *					  note: the array has a NULL at end
 *	fl_free(FLEXLIST *p)		- dispose of all malloced data therein
 *	fl_getcount(FLEXLIST *p)	- return the number of items
 *
 *
 */


#define	CHUNKSIZE	20

struct strlist {
			int	fl_nslots;
			int	fl_nused;
			char	**fl_list;
			int	fl_growby;
	};

typedef struct strlist FLEXLIST;

void fl_init(FLEXLIST *p,int amt);
int  fl_getcount(FLEXLIST *p);
void fl_free(FLEXLIST *p);
int fl_append(FLEXLIST *p, char *str);

char ** fl_getlist(FLEXLIST *p);
void fl_freelist(char **);

struct flexstring {
			int	fs_space;
			int	fs_used;
			char	*fs_str;
			int	fs_growby;

			/* methods here */
			
			int	(*addch)(struct flexstring *,char);
			int	(*addstr)(struct flexstring *,char *);
			char *	(*getstr)(struct flexstring *);
			void	(*free)(struct flexstring *);
};


typedef struct flexstring FLEXSTR;

void fs_init(FLEXSTR *p,int amt);
void fs_free(FLEXSTR *p);
char * fs_getstr(FLEXSTR *p);
char * fs_getstrd(FLEXSTR *p);
int fs_addch(FLEXSTR *p, char c);
int fs_addstr(FLEXSTR *p, char *s);
FLEXSTR *fso_new(int amt);

#endif

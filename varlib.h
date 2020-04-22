// Gerald Arocena
// CSCI E-28, Spring 2020
// 4-22-2020
// hw 5

#ifndef	VARLIB_H
#define	VARLIB_H
/*
 * header for varlib.c package
 */

int	VLexport(char *);
char	*VLlookup(char *);
void	VLlist();
int	VLstore( char *, char * );
char	**VLtable2environ();
int	VLenviron2table(char **);
int VLdelete(char *);

#endif

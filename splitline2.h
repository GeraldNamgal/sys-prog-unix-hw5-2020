#ifndef	SPLITLINE_H
#define	SPLITLINE_H

#include <stdio.h>

#define	YES	1
#define	NO	0

char	*next_cmd(char *, FILE *);
char	**splitline(char *);
void	freelist(char **);
void	*emalloc(size_t);
void	*erealloc(void *, size_t);

#endif

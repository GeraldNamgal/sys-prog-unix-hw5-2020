#ifndef	SMSH_H
#define	SMSH_H

#define MIN_WHILE_CHARS 5              // enough for 'while' command plus an arg

/* Put here things that need to be seen by all parts of the program */
void fatal(char *, char *, int);
int get_last_exit_stat();
void free_cmdline();

#endif

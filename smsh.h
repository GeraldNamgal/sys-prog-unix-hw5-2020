#ifndef	SMSH_H
#define	SMSH_H

#define MIN_WHILE_CHARS 7              // enough for 'while' command plus an arg

struct while_loop {
    char* condition;
    char** body;
};

/* Put here things that need to be seen by all parts of the program */
void fatal(char *, char *, int);
int get_last_exit_stat();

#endif

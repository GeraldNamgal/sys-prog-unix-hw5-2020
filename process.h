#ifndef	PROCESS_H
#define	PROCESS_H

int process(char **args);
int do_command(char **args);
int execute(char **args);
int get_last_exit_stat();

#endif

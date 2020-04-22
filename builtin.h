// Gerald Arocena
// CSCI E-28, Spring 2020
// 4-22-2020
// hw 5

/* *
 * Header for builtin.c
 * note: referenced
 *       https://stackoverflow.com/questions/6294133/maximum-pid-in-linux
 */

#ifndef	BUILTIN_H
#define	BUILTIN_H

#define VAR_MAX_CHARS   80                                    // max buffer size
#define MAX_PID_DIGITS   7
#define MAX_EXSTATUS_DIG 3                             // max exit status digits

int is_builtin(char **args, int *resultp);
int is_assign_var(char *cmd, int *resultp);
int is_list_vars(char *cmd, int *resultp);
void varsub(char **args);
int assign(char *);
int okname(char *);
int is_export(char **, int *);
int is_cd(char **, int *);
int is_exit(char **, int *);
int is_read(char **, int *);
int is_unset(char **, int *);

#endif

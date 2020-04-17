#ifndef	BUILTIN_H
#define	BUILTIN_H

#define VAR_MAX_CHARS   80     // TODO: max buffer size ok (not sure they care)?

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

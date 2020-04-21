/*
 * controlflow.h
 */

#include	"flexstr2.h"

#define IF_ERROR 0
#define WHILE_ERROR 1

struct while_loop {
    char* condition;
    FLEXLIST* body;
};

int is_control_command(char *);
int do_control_command(char **);
int ok_to_execute();
void check_for_while(char*);
void init_while_struct();
void free_while_struct();


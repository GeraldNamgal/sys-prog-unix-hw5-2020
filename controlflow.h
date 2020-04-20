/*
 * controlflow.h
 */

#include	"flexstr2.h"

struct while_loop {
    char* condition;
    FLEXLIST* body;
};

int is_control_command(char *);
int do_control_command(char **);
int ok_to_execute();
int get_if_state();
void check_for_while(char*);
void init_while_struct();
void free_while_struct();


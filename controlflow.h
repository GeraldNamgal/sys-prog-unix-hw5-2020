/*
 * controlflow.h
 */

struct while_loop {
    char* condition;
    char** body;
};

int is_control_command(char *);
int do_control_command(char **);
int ok_to_execute();
int get_if_state();
void check_for_while(char*);


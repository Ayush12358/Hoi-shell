// theme one
// char *c_one = "\033[0;32m"; // green 
// char *c_two = "\033[0;34m"; // blue
// char *c_three = "\033[0;35m"; // magenta
// char *c_four = "\033[0;31m"; // red
// char *c_reset = "\033[0m"; // white
// theme two
char *c_one = "\033[0;36m"; // cyan
char *c_two = "\033[0;35m"; // magenta
char *c_three = "\033[0;30m"; // grey
char *c_four = "\033[0;31m"; // red
char *c_reset = "\033[0m"; // white
// global variables
char *home;
char *wd;
char **history_wd;
int history_wd_size;
char **logs;
int log_size;
char *command_input;
int flag_time;
int log_flag;
int count_g;
struct termios orig_termios;
int fd;
char **aliases;
int alias_size;
char **functions;
int function_size;
int pipe_fd[2];
int saved_stdout;
int saved_stdin;
// Define maximum sizes
#define MAX_LINE_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <termios.h>
#include <curl/curl.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
// functions in order of call
int display_req(char *home);
int input_req(char *input, char *home);
int clean_c(char *command);
int redirection(char *command);
int execute(char *command);
int hop(char *command);
int reveal(char *command);
int loging(char *command);
int proclore(char *command);
int seek(char *command);
int activities(char *command);
int neonate(char *command);
// helper functions
char **search_dir(char *directory, int d, int f, int e, int h, char *search);
void reset_input_mode(void);
void set_input_mode(void);
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
void fetch_and_print_html(const char *url);
int compare_strings(const void *a, const void *b);

#include "init.c"
#include "header.h"
#include "definer.c"
#include "helper.c"
#include "execute.c"

int execute(char *command){ // forward to command, time taken, and logs
    // handle alias
    command = translate_myshrc(command);
    // execute command
    strcpy(command_input, command);
    char *token = __strtok_r(command, " ", &command);
    int error = 0;
    if(strcmp(token, "hop") == 0){
        error = hop(command);
    }else if(strcmp(token, "reveal") == 0){
        reveal(command);
    }else if(strcmp(token, "log") == 0){
        log_flag=1;
        error = loging(command);
    }else if(strcmp(token, "proclore") == 0){
        error = proclore(command);
    }else if(strcmp(token, "seek") == 0){
        error = seek(command);
    }else if(strcmp(token, "activities") == 0){
        error = activities(command);
    }else if(strcmp(token, "ping") == 0){
        error = pinging(command);
    }else if(strcmp(token, "fg") == 0){
        error = fg_bg(command, 0);
    }else if(strcmp(token, "bg") == 0){
        error = fg_bg(command, 1);
    }else if(strcmp(token, "neonate") == 0){
        error = neonate(command);
    }else if(strcmp(token, "iMan") == 0){
        error = iMan(command);
    }else if(strcmp(token, "hoi") == 0){
        printf("Aww you found me :D\nHere is a cookie for you\n'inserts cookie'\nNow get back to work silly!\n");
        return 0;
    }else{
        // execute command
        // start timer
        time_t start = time(NULL);
        pid_t pid = fork();
        if(pid == 0){ // child process
            char *args[MAX_LINE_SIZE];
            args[0] = token;
            int i = 1;
            while((token = __strtok_r(command, " ", &command)) != NULL){
                args[i] = token;
                i++;
            }
            args[i] = NULL;
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }else if(pid > 0){ // parent process
            int status;
            waitpid(pid, &status, 0);
            // end timer
            time_t end = time(NULL);
            // save integer time
            flag_time = (int)(end - start);
            if (WIFEXITED(status)){
                if(WEXITSTATUS(status) == 0){
                    return 0;
                }else if (WEXITSTATUS(status) == 1){
                    return 1;
                }else{
                    return WEXITSTATUS(status);
                }
            }
        }else{ // fork failed
            perror("fork");
            return 2;
        }
    }
    if (error == 0 && log_flag==0){
        // add command to log
        if(log_size == MAX_LINE_SIZE){
            for(int i = 0; i < 1023; i++){
                logs[i] = logs[i + 1];
            }
            free(logs[1023]);
            logs[1023] = malloc(strlen(command_input) + 1);
            strcpy(logs[1023], command_input);
        }else{
            logs[log_size] = malloc(strlen(command_input) + 1);
            strcpy(logs[log_size], command_input);
            log_size++;
        }
    }
    log_flag=0;
    return error;
}

int redirection_output(char *command) { // >, >>
    // Handles redirection from > and >>
    char *command_copy = strdup(command); // Make a copy to avoid modifying the original command
    if (!command_copy) {
        perror("strdup");
        return 1;
    }
    // save stdout
    saved_stdout = save_stdout();
    // check if the command contains '>' or '>>'
    char *output = strstr(command_copy, ">");
    if (!output) {
        // No redirection, execute the command
        return execute(command_copy);
    }else if (output[1] == '>') {
        // Append mode
        if (output[2] == 0) {
            // No output file specified
            printf("Error: No output file specified\n");
            free(command_copy);
            return 1;
        }else if (output[2] != ' '){
            printf("Error: Invalid operator or file name\n");
            free(command_copy);
            return 1;
        }
        redirect_stdout_to_file(clean_str(output+2), 1);
    } else {
        // Overwrite mode
        if(output[1] == 0){
            // No output file specified
            printf("Error: No output file specified\n");
            free(command_copy);
            return 1;
        }else if (output[1] != ' '){
            printf("Error: Invalid operator or file name\n");
            free(command_copy);
            return 1;
        }
        redirect_stdout_to_file(clean_str(output+1), 0);
    }
    // Execute the command
    command_copy[strlen(command_copy) - strlen(output) ] = '\0'; // Remove the output file from the command
    int error = execute(clean_str(command_copy));
    // Restore stdout
    restore_stdout(saved_stdout);
    return error;
}

int redirection_input(char *command){ // < 
    // check if < is present
    char *input = strstr(command, "<");
    if (!input) {
        // No redirection, execute the command
        return redirection_output(command);
    }
    if (input[1] == 0) {
        // No input file specified
        printf("Error: No input file specified\n");
        return 1;
    }else if (input[1] != ' '){
        printf("Error: Invalid operator or file name\n");
        return 1;
    }
    // remove < and everything after it
    command[strlen(command) - strlen(input)] = '\0';
    command = clean_str(command);
    // remove < and everything before it
    input = clean_str(input + 2);
    // check if > is present and add it to the command
    char *output = strstr(input, ">");
    if (output){ // if > is present
        // remove > and everything after it
        input[strlen(input) - strlen(output)-1] = '\0';
        input = clean_str(input);
        // add everything after > to command
        printf ("output:%s\n", output);
        command = strcat(command, " ");
        command = strcat(command, output);
    }
    // save stdin
    saved_stdin = save_stdin();
    // redirect stdin
    redirect_stdin_to_file(input);
    // execute command
    // printf ("command:%s\n", command);
    int error = redirection_output(command);
    // restore stdin
    restore_stdin(saved_stdin);
    return error;
}

void split_commands(char *input, char **commands) {
    int i = 0;
    char *token = strtok(input, "|");
    while (token != NULL && i < MAX_LINE_SIZE - 1) {
        commands[i++] = token;
        token = strtok(NULL, "|");
    }
    commands[i] = NULL;  // Null-terminate the array
}

int piping(char *input) {
    char *commands[MAX_LINE_SIZE];
    split_commands(input, commands);

    int num_commands = 0;
    while (commands[num_commands] != NULL) {
        num_commands++;
    }

    int pipes[MAX_LINE_SIZE - 1][2];
    pid_t pid;

    for (int i = 0; i < num_commands; i++) {
        // Create a pipe, except for the last command
        if (i < num_commands - 1) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe failed");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  // Child process
            // If not the first command, read from the previous pipe
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
            // If not the last command, write to the next pipe
            if (i < num_commands - 1) {
                close(pipes[i][0]);
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][1]);
            }
            // Execute the command
            redirection_input(commands[i]);
            exit(EXIT_SUCCESS);
        } else {  // Parent process
            // Close pipe descriptors not needed by parent
            if (i > 0) {
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
            // Wait for the child to finish
            wait(NULL);
        }
    }
    return 0;
}

int clean_c(char *command){ // removed spaes and tabs
    // clean command
    char *trimmed = clean_str(command);
    // execute command
    int error = piping(trimmed);
    return error;
}

int input_req(char *input, char *home){ // ;,&, and exit handled here
    // check ';' and execute commands
    char *token = __strtok_r(input, ";", &input);
    while (token != NULL){
        // find number of '&' in the token
        int count = 0;
        for(int i = 0; i < strlen(token); i++) if(token[i] == '&') count++;
        if(count == 0){
            // clean command
            clean_c(token);
        }else{
            char *parallel = __strtok_r(token, "&", &token);
            for (int i = 0; i < count; i++){
                char temp[MAX_LINE_SIZE];
                // printf("parallel:%s\n", parallel);
                snprintf(temp, MAX_LINE_SIZE, "add %s", parallel);
                loging(temp);
                pid_t pid = fork();
                if(pid == 0){ // child process
                    // print pid
                    printf("%d\n", getpid());
                    int error = clean_c(parallel);
                    // get process name: first word of parallel
                    char *process = malloc(strlen(parallel) + 1);
                    strcpy(process, parallel);
                    char *temp = __strtok_r(process, " ", &process);
                    // print process name and exit status
                    if (error == 0){
                        printf("%d: %s executed successfully\n", getpid(), temp);
                    }else if(error == 1){
                        printf("%d: %s invalid input\n", getpid(), temp);
                    }else if(error == 2){
                        printf("%d: %s invalid operation\n", getpid(), temp);
                    }else{
                        printf("%d: %s exited with unknow error id %d\n", getpid(), temp, error);
                    }
                    exit(0);
                }else if(pid > 0){ // parent process
                    parallel = __strtok_r(token, "&", &token);
                }else{ // fork failed
                    perror("fork");
                    return 2;
                }
            }
            if (parallel != NULL){
                clean_c(parallel);
            }
        }
        token = __strtok_r(input, ";", &input);
    }
    return 0;
}

int display_req(char *home){ // <user@sys:dir time>
    // print prompt message <username@systemname:working directory>
    char cwd[MAX_LINE_SIZE];
    strcpy(cwd, wd);
    // compare cwd with home
    char tempcwd[MAX_LINE_SIZE];
    if(strcmp(cwd, home) == 0){
        strcpy(tempcwd, "~");
    }else if(strstr(cwd, home) != NULL){
        strcpy(tempcwd, "~");
        strcat(tempcwd, cwd + strlen(home));
    }else{
        strcpy(tempcwd, cwd);
    }
    // Get the hostname
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname"); // Handle error
        return 2;
    }
    // time prompt
    char time_prompt[MAX_LINE_SIZE];
    time_prompt[0] = '\0';
    if(flag_time>2){
        // copy log[0] : time s
        sprintf(time_prompt, " %s : %ds", logs[0], flag_time);
        flag_time=0;
    }
    // printf("<%s@%s:%s> ", getenv("USER"), hostname, tempcwd);
    printf("<%s%s@%s%s:%s%s%s%s> ", 
        c_one, getenv("USER"), hostname, c_reset, c_two, tempcwd, time_prompt, c_reset);
    
    // read input from user
    char *input = NULL;
    size_t len = 0;
    getline(&input, &len, stdin);
    // check for exit command
    if(strcmp(input, "exit\n") == 0){ // to be changed later
        return 3;
    }
    // remove newline character
    if(input[strlen(input) - 1] == '\n'){
        input[strlen(input) - 1] = '\0';
    }
    strcpy(command_input, input);
    // handle def_myshrc
    // input = def_myshrc(input);
    // handle input
    int error = input_req(input, home);
    return error;
}

int main() { //init 
    home = getcwd(NULL, 0); //starting directory
    wd = malloc(strlen(home) + 1);
    strcpy(wd, home);
    history_wd = malloc(MAX_LINE_SIZE * sizeof(char *));
    // add home to history
    history_wd[0] = malloc(strlen(home) + 1);
    strcpy(history_wd[0], home);
    history_wd_size = 1;
    logs = malloc(MAX_LINE_SIZE * sizeof(char *));
    log_size = 0;
    command_input = malloc(MAX_LINE_SIZE);
    flag_time=0;
    log_flag=0;
    load_myshrc(".myshrc");
    saved_stdout = dup(STDOUT_FILENO);
    fd = saved_stdout;
    // intro message
    printf("\033[H\033[J");
    printf("%s", c_three);
    printf ("Welcome to the 'hoi' shell\n");
    printf ("Type 'exit' to exit the shell\n");
    printf("Enjoy bad code ;)\n\n");
    printf("%s", c_reset);
    // repeat prompt indefinitely
    while(1) {
        // if wd is not the last in history, add it
        if(strcmp(history_wd[0], wd) != 0){
            for(int i = history_wd_size; i > 0; i--){
                history_wd[i] = history_wd[i - 1];
            }
            history_wd[0] = malloc(strlen(wd) + 1);
            strcpy(history_wd[0], wd);
            history_wd_size++;
        }
        int error = display_req(home);
        if(error == 1) {
            printf("Error: Invalid input\n");
        }else if(error == 2) {
            printf("Error: Invalid operation\n");
        }else if(error == 3) {
            break;
        }
    }
    printf("\nHope you are going away to fix this bad code! \n\n");
    return 0;
}

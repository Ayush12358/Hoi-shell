// main functions

char *translate_myshrc(char *command){
    // check for alias
    for(int i = 0; i < alias_size; i++){
        char *alias = aliases[i];
        char *token = __strtok_r(alias, " ", &alias);
        if (strcmp(command, token) == 0){
            char *alias_command = alias+1;
            return alias_command;
        }
    }
    // check for function
    // for(int i = 0; i < function_size; i++){
    //     char *function = functions[i];
    //     char *token = __strtok_r(function, " ", &function);
    //     if(strcmp(token, "func") == 0){
    //         // check for function
    //         char *function_name = __strtok_r(function, "()", &function);
    //         char *function_command = function;
    //         if(strcmp(function_name, command) == 0){
    //             return function_command;
    //         }
    //     }
    // }
    return command;
}

void load_myshrc(char *filename){
    // init aliases
    aliases = malloc(MAX_LINE_SIZE * sizeof(char *));
    alias_size = 0;
    function_size = 0;
    // load the .myshrc file and store the aliases and functions in the global variables
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        perror("fopen");
        return;
    }
    char line[MAX_LINE_SIZE];
    while(fgets(line, sizeof(line), file)){
        char *command = clean_str(line);
        if(command == NULL){
            continue;
        }
        char *token = __strtok_r(command, " ", &command);
        if(strcmp(token, "alias") == 0){
            // store alias
            aliases[alias_size] = malloc(strlen(command) + 1);
            strcpy(aliases[alias_size], command);
            alias_size++;
        }
        // }else if(strcmp(token, "func") == 0){
        //     // store function
        //     functions[function_size] = malloc(strlen(command) + 1);
        //     strcpy(functions[function_size], command);
        //     function_size++;
        // }
    }
    fclose(file);

}

void print_process_info() {
    struct dirent *entry;
    DIR *dp = opendir("/proc");

    if (dp == NULL) {
        perror("opendir: /proc");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Check if the directory name is a PID (numeric)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            char pid[256];
            strcpy(pid, entry->d_name);

            // Construct the path to the status file of the process
            char status_path[512];
            sprintf(status_path, "/proc/%s/status", pid);

            FILE *status_file = fopen(status_path, "r");
            if (status_file == NULL) {
                perror("fopen");
                continue;
            }

            // Variables to store the necessary information
            char name[256] = {0};
            char state[256] = {0};
            char line[512];

            // Read the status file line by line
            while (fgets(line, sizeof(line), status_file) != NULL) {
                if (strncmp(line, "Name:", 5) == 0) {
                    sscanf(line, "Name:\t%s", name);
                } else if (strncmp(line, "State:", 6) == 0) {
                    sscanf(line, "State:\t%s", state);
                }
            }

            // Print the process details in the required format
            printf("%s: %s, %s\n", pid, name, state);

            fclose(status_file);
        }
    }

    closedir(dp);
}

int activities(char *command){
    print_process_info();
    return 0;
}

int pinging(char *command){
    // send a signal to the process with given pid
    char *token = __strtok_r(command, " ", &command);
    if(token == NULL){
        printf("Error: Too few arguments\n");
        return 1;
    }
    pid_t pid = atoi(token);
    char *signal = __strtok_r(command, " ", &command);
    if(signal == NULL){
        printf("Error: Too few arguments\n");
        return 1;
    }
    int sig = atoi(signal);
    if(kill(pid, sig) == -1){
        printf ("Error: Invalid pid or signal\n");
        return 1;
    }

    return 0;
}

int fg_bg(char *command, int bg){
    // bg: 1 for background, 0 for foreground
    char *pid_token = __strtok_r(command, " ", &command);
    if(pid_token == NULL){
        printf("Error: Too few arguments\n");
        return 1;
    }
    pid_t pid = atoi(pid_token);
    if(bg == 1){
        // change the state of a process to running
        if(kill(pid, SIGCONT) == -1){
            printf("Error: Invalid pid\n");
            return 1;
        }
    }else{
        // change the state of a process to running and wait for it to finish
        if(kill(pid, SIGCONT) == -1){
            printf("Error: Invalid pid\n");
            return 1;
        }
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)){
            if(WEXITSTATUS(status) == 0){
                return 0;
            }else if (WEXITSTATUS(status) == 1){
                return 1;
            }else{
                return WEXITSTATUS(status);
            }
        }
    }

    return 0;
}

int iMan(char *command){
    char *token = __strtok_r(command, " ", &command);
    if(token == NULL){
        printf("Error: Too few arguments\n");
        return 1;
    }else{
        // http://man.he.net/ for manual pages
        char url[MAX_LINE_SIZE];
        snprintf(url, MAX_LINE_SIZE, "http://man.he.net/?section=all&topic=%s", token);
        pid_t pid = fork();
        if(pid == 0){ // child process
            fetch_and_print_html(url);
            exit(0);
        }else if(pid > 0){ // parent process
            int status;
            waitpid(pid, &status, 0);
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
    return 0;
}

int neonate(char *command){
    char *token = __strtok_r(command, " ", &command);
    if(token == NULL){
        printf("Error: Too few arguments\n");
        return 1;
    }else if(strcmp(token, "-n") == 0){
        token = __strtok_r(command, " ", &command);
        if(token == NULL){
            printf("Error: Too few arguments\n");
            return 1;
        }
        int n = atoi(token);
        if(n == 0){
            printf("Error: Invalid argument\n");
            return 1;
        }
        // print pid every n seconds until x is pressed
        pid_t pid = fork();
        if(pid == 0){ // child process
            while(1){
                printf("%d\n", getpid());
                sleep(n);
            }
            exit(0);
        }else if(pid > 0){ // parent process
            char *input = NULL;
            size_t len = 0;
            set_input_mode();
            while(1){
                char c = getchar();
                if( c == 'x'){
                    kill(pid, SIGKILL);
                    break;
                }
            }
            reset_input_mode();
        }else{ // fork failed
            perror("fork");
            return 2;
        }
    }else{
        printf("Error: Invalid argument\n");
        return 1;
    }
    return 0;
}

int seek(char *command){
    count_g = 0;
    char *token = __strtok_r(command, " ", &command);
    int d = 0, f = 0, e = 0, h = 0;
    // d: search for directories, f: search for files, e: print 
    while(token != NULL){
        if(strncmp(token, "-", 1) == 0){
            // detect flags d,f,e
            for(int i = 1; i < strlen(token); i++){
                if(token[i] == 'd'){
                    d = 1;
                }else if(token[i] == 'f'){
                    f = 1;
                }else if(token[i] == 'e'){
                    e = 1;
                }else if(token[i] == 'h'){
                    h = 1;
                }else{
                    printf("Error: Invalid flags\n");
                    return 1;
                }
            }
        }else{
            break;
        }
        token = __strtok_r(command, " ", &command);
    }
    char search[MAX_LINE_SIZE];
    if (token == NULL){
        printf("Error: Too few arguments\n");
        return 1;
    }
    strcpy(search, token);
    token = __strtok_r(command, " ", &command);
    char directory[MAX_LINE_SIZE];
    if(token != NULL){
        strcpy(directory, token);
    }else{
        strcpy(directory, "");
    }
    if (d==1 && f==1){
        printf("Error: Invalid flags\n");
        return 1;
    }
    char temp[MAX_LINE_SIZE];
    char o_wd[MAX_LINE_SIZE];
    strcpy(o_wd, wd);
    snprintf(temp, MAX_LINE_SIZE, "%s", directory);
    int error = hop(temp);
    if(error != 0){
        return error;
    }
    strcpy (directory, wd);
    // all input taken now search
    DIR *dir;
    struct dirent *entry;
    if((dir = opendir(directory)) == NULL){
        perror("opendir");
        return 1;
    }
    int count_d = 0;
    int count_f = 0;
    printf("Searching for %s in %s\n", search, directory);
    // search nested directories
    char **results;
    results = search_dir(directory, d, f, e, h, search);
    if (count_g == 0){
        printf("No results found\n");
        return 0;
    }
    for (int i = 0; i < count_g; i++){
        if (results[i][strlen(results[i]) - 1] == '/'){
            printf("%s%s%s\n", c_two, results[i], c_reset);
        }else{
            printf("%s%s%s\n", c_one, results[i], c_reset);
        }
    }
    if (e == 1 && count_g ==1){
        // print file
        FILE *fp;
        char line[MAX_LINE_SIZE];
        if((fp = fopen(results[0], "r")) == NULL){
            perror("fopen");
            return 1;
        }
        while(fgets(line, sizeof(line), fp)){
            printf("%s", line);
        }
        printf("\n");
        fclose(fp);
    }

    // go back to original directory
    strcpy(wd, o_wd);
    return 0;
}

int proclore(char *command){
    // print pid, process state, process group, virtual memory, executable path
    char *token = __strtok_r(command, " ", &command);
    pid_t pid = getpid();
    if(token!=NULL){
        pid = atoi(token);
    }
    char path[256];
    FILE *fp;

    // 1. Process Status and Process Group
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Failed to open stat file");
        return 1;
    }

    int ppid, pgrp;
    char comm[256], state;
    unsigned long vsize;

    // Reading relevant fields from /proc/[pid]/stat
    fscanf(fp, "%d %s %c %d %d", &pid, comm, &state, &ppid, &pgrp);
    fclose(fp);

    printf("PID: %d\n", pid);
    printf("Process Status: %c\n", state);
    printf("Process Group: %d\n", pgrp);

    // 2. Virtual Memory
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Failed to open status file");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line, "VmSize: %lu kB", &vsize);
            printf("Virtual Memory: %lu kB\n", vsize);
            break;
        }
    }
    fclose(fp);

    // 3. Executable Path
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    char exe_path[256];
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        printf("Executable Path: %s\n", exe_path);
    } else {
        perror("Failed to read executable path");
    }
    return 0;
}

int loging(char *command){ // log, purge, execute
    if(strcmp(command, "") == 0){
        // print log
        for(int i = 0; i < log_size; i++){
            if(logs[i] != NULL){
                printf("%d: %s\n", i, logs[i]);
            }
        }
    }else if(strncmp(command, "purge", 5) == 0){
        // purge log
        for(int i = 0; i < log_size; i++){
            if(logs[i] != NULL){
                free(logs[i]);
            }
        }
        log_size = 0;
    }else if(strncmp(command, "execute ", 8) == 0){
        // execute command from log
        command += 8;
        int index = atoi(command);
        if(index < 0 || index >= log_size){
            printf("Error: Invalid index\n");
            return 1;
        }
        // execute command
        printf ("executing %d: %s\n",index, logs[index]);
        int error = input_req(logs[index], home);
        return error;
    }else if(strncmp(command, "add ", 4) == 0){
        // add command to log
        command += 4;
        if(log_size == MAX_LINE_SIZE){
            for(int i = 0; i < 1023; i++){
                logs[i] = logs[i + 1];
            }
            free(logs[1023]);
            logs[1023] = malloc(strlen(command) + 1);
            strcpy(logs[1023], command);
        }else{
            logs[log_size] = malloc(strlen(command) + 1);
            strcpy(logs[log_size], command);
            log_size++;
        }
    }else{
        printf("Error: Invalid command\n");
        return 1;
    }
    return 0;
}

int reveal(char *command){ // dummy ls
    // list all files and directories in the current directory in lexicographical order
    char *token = __strtok_r(command, " ", &command);
    int a = 0, l = 0;
    char *temp = malloc(strlen(command) + 1);
    strcpy(temp, "");
    while(token !=NULL){
        if(strncmp (token, "-", 1) == 0){
            for(int i = 1; i < strlen(token); i++){
                if(token[i] == 'a'){
                    a = 1;
                }else if(token[i] == 'l'){
                    l = 1;
                }else{
                    printf("Error: Invalid option\n");
                    return 1;
                }
            }
        }else{
            // remove reveal and flags from command and replace with hop
            strcat(temp, token);
        }
        token = __strtok_r(command, " ", &command);
    }
    // execute hop command
    char* temp_wd = malloc(strlen(wd) + 1);
    strcpy(temp_wd, wd);
    int error = hop(temp);
    if(error != 0){
        return error;
    }else{
        // list all files and directories in the current directory in lexicographical order
        DIR *dir;
        struct dirent *entry;
        if((dir = opendir(wd)) == NULL){
            perror("opendir");
            return 1;
        }
        while((entry = readdir(dir)) != NULL){
            if(a == 1 || entry->d_name[0] != '.'){
                if(l == 1){
                    struct stat fileStat;
                    char *path = malloc(strlen(wd) + strlen(entry->d_name) + 2);
                    strcpy(path, wd);
                    strcat(path, "/");
                    strcat(path, entry->d_name);
                    if(stat(path, &fileStat) < 0){
                        perror("stat");
                        return 1;
                    }
                    printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
                    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
                    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
                    printf(" %ld", fileStat.st_nlink);
                    printf(" %s", getpwuid(fileStat.st_uid)->pw_name);
                    printf(" %s", getgrgid(fileStat.st_gid)->gr_name);
                    printf(" %ld", fileStat.st_size);
                    printf(" %s\n", entry->d_name);
                }else{
                    printf("%s\n", entry->d_name);
                }
            }
        }
        closedir(dir);
        strcpy(wd, temp_wd);
    }
    return 0;
}

int hop(char *command){ // dummy cd
    // check for hop command
    char *token = __strtok_r(command, " ", &command);
    while (token != NULL){
        if(strcmp(token, ".") == 0){
            // do nothing
        }else if (strcmp(token, "..") == 0){
            // go to parent directory
            int i = strlen(wd) - 1;
            while(wd[i] != '/'){
                i--;
            }
            wd[i] = '\0';
        }else if(strcmp(token, "~") == 0){
            // go to home directory
            strcpy(wd, home);
        }else if(strcmp(token, "-") == 0){
            // go to previous directory in history
            if (history_wd_size < 2){
                printf("Error: No previous directory in history\n");
                return 1;
            }
            char *temp = malloc(strlen(wd) + 1);
            strcpy(temp, wd);
            strcpy(wd, history_wd[1]);
            for(int i = 0; i < 1023; i++){
                history_wd[i] = history_wd[i + 1];
            }
            history_wd[1023] = temp;
        }else if(strcmp(token, "//h") == 0){ // print history
            // print history
            for(int i = 0; i < history_wd_size; i++){
                if(history_wd[i] != NULL){
                    printf("%d: %s\n", i,history_wd[i]);
                }
            }
        }else{
            if (strncmp(token, "~/", 2) == 0){
                // go to the directory
                char *temp = malloc(strlen(home) + strlen(token) + 1);
                strcpy(temp, home);
                strcat(temp, token + 1);
                // printf ("temp:%s\n", temp);
                if(chdir(temp) == 0){
                    strcpy(wd, temp);
                }else{
                    perror("chdir");
                    return 1;
                }
            }else if (token[0] == '/'){
                // go to the directory
                if(chdir(token) == 0){
                    strcpy(wd, token);
                }else{
                    perror("chdir");
                    return 1;
                }
            }else{
                // go to the directory
                char *temp = malloc(strlen(wd) + strlen(token) + 2);
                strcpy(temp, wd);
                strcat(temp, "/");
                strcat(temp, token);
                if(chdir(temp) == 0){
                    strcpy(wd, temp);
                }else{
                    perror("chdir");
                    return 1;
                }
            }
        }
        token = __strtok_r(command, " ", &command);
    }
    return 0;
}

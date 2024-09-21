// helper functions
char *clean_str(char *command){
    // remove newline character 
    if(command[strlen(command) - 1] == '\n'){
        command[strlen(command) - 1] = '\0';
    }
    // remove leading and trailing spaces
    int start = 0;
    int end = strlen(command) - 1;
    while(command[start] == ' ' || command[start] == '\t'){
        start++;
    }
    while(command[end] == ' ' || command[end] == '\t'){
        end--;
    }
    // check for empty command
    if(start > end){
        return NULL;
    }
    // copy the command to a new string
    char *new_command = malloc(end - start + 2);
    for(int i = start; i <= end; i++){
        new_command[i - start] = command[i];
    }
    new_command[end - start + 1] = '\0';
    // copy the new command to command
    strcpy(command, new_command);
    // handle unnecessary spaces and tabs by trimming and removing them
    char *trimmed = malloc(strlen(command) + 1);
    int i = 0;
    int j = 0;
    int space = 0;
    while(command[i] != '\0'){
        if(command[i] == ' ' || command[i] == '\t'){
            if(space == 0){
                trimmed[j] = ' ';
                j++;
                space = 1;
            }
        }else{
            trimmed[j] = command[i];
            j++;
            space = 0;
        }
        i++;
    }
    trimmed[j] = '\0';
    return trimmed;
}

void reset_input_mode(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void set_input_mode(void) {
    struct termios tattr;
    
    // Save the original terminal attributes so we can restore them later
    tcgetattr(STDIN_FILENO, &orig_termios);
    
    // Set the terminal to raw mode (non-canonical mode) with no echo
    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

char **search_dir(char *directory, int d, int f, int e, int h, char *search){
    // list everything in the directory with full path
    DIR *dir;
    struct dirent *entry;
    if((dir = opendir(directory)) == NULL){
        perror("opendir");
        return NULL;
    }
    char **result = malloc(MAX_LINE_SIZE * sizeof(char *));
    int count = 0;
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            char *path = malloc(strlen(directory) + strlen(entry->d_name) + 2);
            strcpy(path, directory);
            strcat(path, "/");
            strcat(path, entry->d_name);
            struct stat fileStat;
            if(stat(path, &fileStat) < 0){
                perror("stat");
                return NULL;
            }
            // call search_dir recursively
            if(S_ISDIR(fileStat.st_mode)){
                if (h==0 && strncmp(entry->d_name, ".", 1) == 0){
                    continue;
                }
                if(d == 1 && strncmp(entry->d_name, search, strlen(search)) == 0){
                    result[count] = malloc(strlen(path) + 2);
                    snprintf(result[count], strlen(path) + 2, "%s/", path);
                }else if(d == 0 && strncmp(entry->d_name, search, strlen(search)) == 0 && f == 0){
                    result[count] = malloc(strlen(path) + 2);
                    snprintf(result[count], strlen(path) + 2, "%s/", path);
                    count++;
                }
                // search nested directories
                char **temp = search_dir(path, d, f, e, h, search);
                for (int i = 0; i < count_g; i++){
                    result[count] = malloc(strlen(temp[i]) + 1);
                    snprintf(result[count], strlen(temp[i]) + 1, "%s", temp[i]);
                    count++;
                }
            }else{
                if(f == 1 && strncmp(entry->d_name, search, strlen(search)) == 0){
                    result[count] = malloc(strlen(path) + 1);
                    snprintf(result[count], strlen(path) + 1, "%s", path);
                    count++;
                }else if(f == 0 && strncmp(entry->d_name, search, strlen(search)) == 0 && d == 0){
                    result[count] = malloc(strlen(path) + 1);
                    snprintf(result[count], strlen(path) + 1, "%s", path);
                    count++;
                }
            }

        }
    }
    closedir(dir);
    count_g = count;
    return result;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    // Callback function to write the data
    // Print the data to stdout
    // print only the text
    char *data = (char *)ptr;
    int i = 0;
    while(data[i] != '\0'){
        if(data[i] == '<'){
            while(data[i] != '>'){
                i++;
            }
        }else{
            printf("%c", data[i]);
        }
        i++;
    }

    return size * nmemb;
}

size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    size_t total_size = size * nitems;
    if(strncmp(buffer, "Date:", 5) == 0) {
        printf("Date: %.*s", (int)(total_size - 6), buffer + 6);
    } else if(strncmp(buffer, "Server:", 7) == 0) {
        printf("Server: %.*s", (int)(total_size - 8), buffer + 8);
    } else if(strncmp(buffer, "Last-Modified:", 14) == 0) {
        printf("Last Modified: %.*s", (int)(total_size - 15), buffer + 15);
    }
    return total_size;
}

void fetch_and_print_html(const char *url) {
    CURL *curl;
    CURLcode res;
    char *ip;
    long response_code;
    double total_time;
    long port;
    char *content_type;
    double content_length;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback); // Set the header callback
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  // Fetch headers only first
        // Perform the request to get headers
        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);
            printf("IP Address: %s\n", ip);
            curl_easy_getinfo(curl, CURLINFO_PRIMARY_PORT, &port);
            printf("Port: %ld\n", port);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            printf("Response Code: %ld\n", response_code);
            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
            if(content_length > 0) {
                printf("Content Length: %.0f bytes\n", content_length);
            }
            curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
            if(content_type) {
                printf("Content Type: %s\n", content_type);
            }
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
            printf("Total Time: %.2f seconds\n", total_time);
        } else {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        // Fetch the actual content
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);  // Switch to body fetching
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); // Set the write callback
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
}

int compare_strings(const void *a, const void *b) {
    const char **str_a = (const char **)a;
    const char **str_b = (const char **)b;
    return strcmp(*str_a, *str_b);
}

void redirect_stdout_to_file(const char *filename, int append) {
    int fd;
    
    // Open the file for writing, either in append or rewrite mode
    if (append) {
        fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644); // Append mode
    } else {
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);  // Overwrite mode
    }

    if (fd == -1) {
        // create file if it does not exist
        fd = open(filename, O_WRONLY | O_CREAT, 0644);
        if (fd == -1) {
            printf("Error opening file for redirection\n");
            return;
        }
    }

    // Redirect stdout to the file
    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("Error redirecting stdout to file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

void restore_stdout(int saved_stdout) {
    if (dup2(saved_stdout, STDOUT_FILENO) == -1) {
        perror("Error restoring stdout");
        exit(EXIT_FAILURE);
    }
}

int save_stdout() {
    // get current output stream
    int saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout == -1) {
        perror("Error saving stdout");
        exit(EXIT_FAILURE);
    }
    return saved_stdout;
}

void redirect_stdin_to_file(const char *filename) {
    // check if the file exists
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        // create file if it does not exist
        fd = open(filename, O_WRONLY | O_CREAT, 0644);
        if (fd == -1) {
            printf("Error opening file for redirection\n");
            return;
        }
    }
    // Redirect stdin to the file
    if (dup2(fd, STDIN_FILENO) == -1) {
        perror("Error redirecting stdin to file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

int save_stdin() {
    // get current output stream
    int saved_stdin = dup(STDIN_FILENO);
    if (saved_stdin == -1) {
        perror("Error saving stdin");
        exit(EXIT_FAILURE);
    }
    return saved_stdin;
}

void restore_stdin(int saved_stdin) {
    if (dup2(saved_stdin, STDIN_FILENO) == -1) {
        perror("Error restoring stdin");
        exit(EXIT_FAILURE);
    }
}

/*
Tutorial made by Stephen Brennan's 
https://brennan.io/2015/01/16/write-a-shell-in-c/

Compile: gcc -o main main.c
Run: ./main
*/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Function declarations for builtin shell commands
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// List of builtin commands
char *builtin_str[] = {
    "cd", "help", "exit"
};

// List of builtin function
int (*builtin_func[]) (char **) = {
    &lsh_cd, &lsh_help, &lsh_exit
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Builtin function implementations
int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
        
    }
    
    return 1;
}

int lsh_help(char **args) {
    printf("Type program names and arguments and hit enter\n");
    printf("Built in functions:\n");

    for (int i = 0; i < lsh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }
    
    // man command uses execvp to run man
    printf("Use the man command for information on other programs\n");
    return 1;
}

int lsh_exit(char **args) {return 0;}


#define LSH_RL_BUFSIZE 1024
char *lsh_read(void){
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;  // int because EOF is an int not a char

    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a nul character and return
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        
        // If we have exceeded  the buffer, reallocate
        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            
        }
        
    }
    
}

#define LSH_TOKEN_BUFSIZE 64
#define LSH_TOKEN_DELIM " \t\r\n"   // space - tab - enter - new line
char **lsh_split(char *line){
    int bufsize = LSH_TOKEN_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line,LSH_TOKEN_DELIM);   
    // strtok is a function to split strings into tokens strtok(string_to_define, delimiter)
    while (token) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            
        }
        
        token = strtok(NULL, LSH_TOKEN_DELIM);  // pick up from the separator/delim of the string used before
    }
    
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // child process
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent Process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        
    }
    
    return 1;
}

int lsh_execute(char **args) {
    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }
    
    for (int i = 0; i < lsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
        
    }
    
    return lsh_launch(args);
}

#define PATH_MAX 4096
void lsh_loop(void){
    char *line;
    char **args;
    int status;
    char path[PATH_MAX];

    do{
        getcwd(path, sizeof(path));
        printf("%s > ", path);
        line = lsh_read();
        args = lsh_split(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv){
    // load config files, if any

    // Run command loop
    lsh_loop();

    // Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}
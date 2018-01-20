#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

int jash_cd(char **args);
int jash_exit(char **args);
char *global_arg;


char* builtin_str[] = {
    "cd",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &jash_cd,
    &jash_exit
};

int jash_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


/**
 Change directory.
 @param args List of args from the change directoy command.
 @return Always returns 1, to continue executing.
 */
int jash_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "jash: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("jash");
        }
    }
    return 1;
}

/**
 Builtin command: exit.
 @param args List of args.  Not examined.
 @return Always returns 0, to terminate execution.
 */
int jash_exit(char **args)
{
    return 0;
}

/**
 Launch a program and wait for it to terminate.
 @param args list of arguments.
 @return Always returns 1, to continue execution.
 */
int jash_launch(char **args)
{
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("jash");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("jash");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

/**
 perform check to determin whether perform builtin commands or launch a program.
 @param args list of arguements.
 @return 1 if the shell should continue running, 0 if it should terminate
 */
int jash_execute(char **args)
{
    // if empty command was entered, return 1 to continue
    if (args[0] == NULL) {
        return 1;
    }
    
    for (int i = 0; i < jash_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return jash_launch(args);
}

/**
 Read a line of input from stdin.
 @return The line from stdin.
 */
#define JASH_READ_BUFSIZE 1024
char* jash_read_line()
{
    int bufsize = JASH_READ_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    if (!buffer) {
        fprintf(stderr, "jash: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        // Read a character
        c = getchar();
        if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else {
            buffer[position] = c;
        }
        
        //check if buffer needs to be resized
        position++;
        if (position >= bufsize) {
            bufsize += JASH_READ_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "jash: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define JASH_TOKEN_BUFSIZE 128
#define JASH_TOKEN_DELIM " \t\r\n\a"

char** jash_split_line(char *line)
{
    int bufsize = JASH_TOKEN_BUFSIZE,
        position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    
    if (!tokens) {
        fprintf(stderr, "jash: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, JASH_TOKEN_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        
        //check if buffer needs to be resized
        if (position >= bufsize) {
            bufsize += JASH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "jash: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, JASH_TOKEN_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


char * get_current_dir()
{
    
    char cwd[1024];
    char * ptr;
    int ch = '/';
    
    if (getcwd(cwd, sizeof(cwd)) != NULL){
        //printf("\nCurrent working dir: %s\n", cwd);
        ptr = strrchr( cwd, ch );
        printf("%s",ptr);
        //printf("\n");
    }else{
        perror("getcwd() error");
    }
    return ptr;
}

char* get_machine_name()
{
    char *name = malloc(sizeof(char) * 1024);

    if(gethostname(name, 150) == 0){
        printf("%s", name);
    } else {
        perror("gethostname() error");
    }
    return name;
}

char * get_username()
{
    char *username = getenv("USER");
    //if(username==NULL) return EXIT_FAILURE;
    printf("%s",username);
    return username;
}


int print_user_info()
{
    printf("jash ");
    //get_machine_name();
    get_current_dir();
    //get_username();
    printf(": ");
    printf("%s",global_arg);
    printf(" ");

    return 1;
}

/**
 prepend the global command to pass argument
 @return the full line
 */
char* prepend_global(char * argument)
{
    char * new_string = malloc(strlen(argument)+25);
    strcpy(new_string, global_arg);
    strcat(new_string, " ");
    strcat(new_string, argument);
    return new_string;
}

/**
Loop getting input and executing it.
 */
#define LITERAL_DELIMTER '\\'
void jash_loop(void)
{
    char *line;
    char **arguments;
    int status;
    
    do {
        print_user_info();
        line = jash_read_line();
        if(line[0] == LITERAL_DELIMTER){
            char *temp = 1 + line;
            arguments = jash_split_line(temp);
        } else {
            arguments = jash_split_line(prepend_global(line));
        }
        status = jash_execute(arguments);
        
        free(line);
        free(arguments);
    } while (status);
}

/**
 Main entry point.
 @param argc Argument count.
 @param argv Argument vector.
 @return status code
 */
//int main(int argc, char **argv)
int main(int argc, char *argv[])
{
    
    if(argv[1] == NULL){
        printf("Error: please try again providing a valid command\n");
        return EXIT_FAILURE;
    }
    global_arg = (char *)malloc((strlen(argv[1]))*sizeof(char));
    strcpy(global_arg, argv[1]);

    // Run command loop.
    jash_loop();
    return EXIT_SUCCESS;
}



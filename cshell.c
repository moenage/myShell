#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct EnvVariable {
    char *name;
    char *value;
} EnvVar;

// *******
// Built-in-commands
// *******

void exit_command(char** tokens){
    exit(0); //Exits the cshell
}

void print_command(char** tokens){
    printf("print");
}

void theme_command(char** tokens){
    printf("theme");
}

void log_command(char** tokens){
    printf("log");
}

char *BuiltIn_Names[] = {"exit", "print", "theme", "log"};
void (*BuiltIn_Commands[])(char ** tokens) = {exit_command, print_command, theme_command, log_command};

// ******           


char * read_line(){
    //Reads 1 line from input and returns it
    char *line = NULL;
    size_t size = 0;
    getline(&line, &size, stdin);
    return line;
}

char ** parse_line(char * line){
    //Parses the line by filling with null char on
    // every instance of a delim character
    int size = 32;
    int count = 0;

    char **tokens = malloc(size * sizeof(char*));

    // delim are the values that will be parsed out of line into tokens using strtok()
    char *delim = " \t\r\n\a";
    char *token = strtok(line, delim);

    while (token != NULL) {
        tokens[count] = token;
        count++;

        if (count >= size) {
            // Increases tokens size if necessary
            size += size;
            tokens = realloc(tokens, size * sizeof(char*));
        }

        token = strtok(NULL, delim);
    }

    tokens[count] = NULL;
    return tokens;
}


void execute_tokens(char **tokens) {

    //Check if built-in command is called first
    for(int i = 0; i < 4; i++){
        if(!strcmp(BuiltIn_Names[i], tokens[0])){
            BuiltIn_Commands[i](tokens);
        }
    }


    pid_t pid = fork();

    if(pid < 0){
        // Fork failed and exit
        printf("Fork Failed \n");
        exit(1);
    }
    else if(pid == 0){
        // Child Process
        // This is where we execute non-built-in commands
        execvp(tokens[0], tokens);
        printf("execvp Failed \n");
        exit(1);
    }
    else if(pid > 0){
        //Parent Process
        // Wait for child process to finish
        int status;
        do{
            waitpid (pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}


int main() {


    while (1) {

        printf("cshell$");

        char *line = read_line();
        char **tokens = parse_line(line);

        if (tokens[0] != NULL) {
            execute_tokens(tokens);
        }

        free(tokens);
        free(line);
    }


   return 0;
}
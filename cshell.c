#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>


// ******
// Log
// ******

typedef struct {
    char *name;
    struct tm *time;
    int value;
} LogStruct;

LogStruct * LogArray;
size_t LogArray_len = 32;
size_t LogArray_count = 0;

void add_Log(char * name, int value){

    //Check if array needs to be bigger
    if(LogArray_len <= LogArray_count){
        LogArray_len += LogArray_len;
        LogArray = (LogStruct *) realloc(LogArray, (LogArray_len) * sizeof(LogStruct));
    }

    time_t rawtime;
    time( &rawtime );
   

    LogArray[LogArray_count].name = name;
    LogArray[LogArray_count].time = localtime( &rawtime );
    LogArray[LogArray_count].value = value;
    LogArray_count++;
}

void failed_log(){
    LogArray[LogArray_count-1].value = -1;
}

// ******
// Environmental Variables
// ******

typedef struct EnvVariable{
    char *name;
    char *value;
} EnvVar;

EnvVar * VarArray;
size_t VarArray_len = 32;
size_t VarArray_count = 0;

int colour = 8;


void add_EnvVar(char *name, char *value){

    //Check if Var name already exists or not
    for(int i = 0; i < VarArray_count; i++){
        if(!strcmp(VarArray[i].name, name)){
            printf("ERROR: %s already exists \n", name);
            return;
        }
    }

    //Check if array needs to be bigger
    if(VarArray_len <= VarArray_count){
        VarArray_len += VarArray_len;
        VarArray = (EnvVar *) realloc(VarArray, (VarArray_len) * sizeof(EnvVar));
    }

    VarArray[VarArray_count].name = name;
    VarArray[VarArray_count].value = value;
    VarArray_count++;
}

//Checks if EnvVar exists or not
int check_EnvVar(char *name){
    for(int i = 0; i < VarArray_count; i++){
        if(!strcmp(VarArray[i].name, name)){
            return i;
        }
    }
    return -1;
}


// *******
// Built-in-commands
// *******

void exit_command(){
    printf("Bye!\n");
    exit(0); //Exits the cshell
}

void print_command(char** tokens){
    //printf("print");
    if (tokens[1] == NULL)
    {
        failed_log();
        return;
    }
    int counter = 0;
    int i = 1;
    while (tokens[i] != NULL)
    {
        counter++;
        i++;
    }
    printf("%s", tokens[1]);
    if (counter > 1)
    {
        for (int j = 1; j < counter; j++)
        {
            printf(" %s", tokens[j+1]);
        }
    }
    printf("\n");
}

void theme_command(char** tokens){
    if (tokens[1] == NULL)
    {
        return;
    }
    
    char* colourString = tokens[1];

    if(strcmp(colourString, "red") == 0)
    {
        printf("\033[0;31m");
        colour = 2;
    }
    else if(strcmp(colourString, "green") == 0)
    {
        
        printf("\033[0;32m");
        colour = 3;
    }
    else if(strcmp(colourString, "blue") == 0)
    {
        
        printf("\033[0;34m");
        colour = 5;
    }
    else
    {
        printf("unsupported theme\n");
        failed_log();
    }
}

void log_command(char** tokens){
    for(size_t i = 0; i < LogArray_count-1; i++){
        if(LogArray[i].value != -1){
            printf("%s", asctime(LogArray[i].time));
            printf("%s %d\n", LogArray[i].name, LogArray[i].value);
        }
    }
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
    char first_char = token[0];
    char *dupStr = NULL;
    int check_var = -1;

    while (token != NULL) {

        first_char = token[0];
        if((count != 0) && (first_char == '$')){ //Checks if arg is a EnvVar
            dupStr = strdup(token);
            dupStr++;
            check_var = check_EnvVar(dupStr);

            //If envVar exists then it replaces it with its value, otherwise it prints error 
            //and cancels execution by returning tokens[0] = NULL
            if(check_var > -1){
                tokens[count] = VarArray[check_var].value;
            }
            else{
                printf("ERROR: %s does not exist\n", dupStr);
                tokens[0] = NULL;
                return tokens;
            }

        }

        else{
            tokens[count] = token;
        }

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

    //Check if Env Var addition
    char first_char = tokens[0][0];
    if(first_char == '$'){
        const char deli[] = "=";

        char *dupStr = strdup(tokens[0]);
        dupStr++;

        char *token = strtok(dupStr, deli);
        dupStr += strlen(token) + 1;


        add_EnvVar(token, dupStr);

        return;
    }

    char *command_name = strdup(tokens[0]);

    //Check if built-in command is called
    for(int i = 0; i < 4; i++){
        if(!strcmp(BuiltIn_Names[i], tokens[0])){
            add_Log(command_name, 0);
            BuiltIn_Commands[i](tokens);
            return;
        }
    }


    pid_t pid = fork();
    add_Log(command_name, 0);

    if(pid < 0){
        // Fork failed and exit
        printf("Fork Failed \n");
        exit(1);
    }
    else if(pid == 0){
        // Child Process
        // This is where we execute non-built-in commands
        execvp(tokens[0], tokens);
        perror("Command not found");
        exit(-1);
    }
    else if(pid > 0){
        //Parent Process
        // Wait for child process to finish
        int status;
        do{
            waitpid (pid, &status, WUNTRACED);
            if(status != 0){
                failed_log();
            }
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}


int main(int argc, char **argv) {

    //Initialize global Log array
    LogArray = (LogStruct *) malloc(LogArray_len * sizeof(LogStruct));

    //Initialize global Envvar array
    VarArray = (EnvVar *) malloc(VarArray_len * sizeof(EnvVar));

    //Script Mode
    if(argc == 2){
        FILE* file = fopen(argv[1], "r");
        if(file == NULL){
            printf("ERROR: opening file");
            exit_command();
        }
        char line[256];
        char* checker = fgets(line, sizeof(line), file);
        while (checker != NULL) {

            char **tokens = parse_line(line);

            if (tokens[0] != NULL) {
                execute_tokens(tokens);
            }

            free(tokens);
            checker = fgets(line, sizeof(line), file);
        }
        fclose(file);
    }

    //Interactive Mode
    else{
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
    }

    free(VarArray);

   return 0;
}
# Building a Shell

Developed a command-line interpreter or shell supporting the environment variables and history of executed commands. 
This will support basic shell functionalities including but not limited to: 
- Support the following built-in commands: exit, log, print, theme
- Support non-built-in commands by creating child processes using fork() and executing using exec()
- Will work in two modes: interactive mode and script mode
- The shell will support the inner environment variables. Each environment variable could be stored in a struct like
    typedef struct {
      char *name;
      char *value;
    } EnvVar;

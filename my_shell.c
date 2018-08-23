/*Taken from Stephen Brennan Tutorial - brennan.io*/

#include <sys/wait.h> /*waitpid() and macros*/
//#include <sys/types.h>
#include <unistd.h> /* chdir() fork() exec() pid_t */
#include <stdio.h>  /* fprintf() printf( stderr getchar() perror()*/
#include <stdlib.h> /* malloc() realloc() free() exit() execvp()
                        EXIT_SUCCESS EXIT FAILURE*/
#include<string.h> /*strcmp() strtok()*/


/***** defines *****/

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"


/***** functions *****/

/* built-ins*/

/* Declare first as they re used before defined */

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {
    "cd", "help", "exit"
};

int (*builtin_func[])(char **) = {
    &lsh_cd, &lsh_help, &lsh_exit
}; /*an array of function pointers
    (that take array of strings and return an int)*/

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/* implementations*/

int lsh_cd(char **args)
{
    if(args[1] == NULL) /*check second argument exists*/
    {
        fprintf(stderr, "lsh: expected argument  to \"cd\"\n");
    }
    else
    {
        if(chdir(args[1]) != 0) /*call cjdir and check for errors*/
        {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args)
{
    int i;
    printf("Mal's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man commmand for info on other programs\n");
    return 1;
}

int lsh_exit(char **args)
{
    return 0; /*terminate command loop*/
}


/* shell operations */


/*an existing process forks itself into two separate ones
Then, the child uses exec() to replace itself with a new program*/
int lsh_launch(char **args)
{
    pid_t pid, wpid; /*signed integer data type representing process IDs*/
    int status;

    pid = fork();
    if(pid == 0) /* ie child process*/
    {
        if(execvp(args[0], args) == -1)/*The first argument is the file
                                     you wish to execute, and the second
                                     argument is an array of null-terminated
                                     strings that represent the appropriate
                                     arguments - only returns when error*/
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0)
    {
        /*error forking*/
        perror("lsh");
    }
    else
    {
        /*parent process needs to wait for the child to finish running*/
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED); /*wait for the process’s
                                                         state to change*/
        }
        while(!WIFEXITED(status) && !WIFSIGNALED(status)); 
        /*use macros for waitpid() to wait until either the processes are
         exited or killed*/
    }

    return 1; /*signal to calling func for input*/
}

int lsh_execute(char **args)
{
    int i;

    if(args[0] == NULL) /*check commands were entered*/
    {
        return 1;
    }

    for(i=0; i<lsh_num_builtins(); i++)
    {
        if(strcmp(args[0], builtin_str[i]) == 0) /*check a command is equal
                            to a builtin*/ 
        {
            return(*builtin_func[i])(args);/*and run it*/
        }
    }
    return lsh_launch(args);/*if not launch the process*/
}


char *lsh_read_line(void)
{
    char *line = NULL;
    ssize_t bufsize = 0; /*signed size_t which getline assigns a buffer size */
    getline(&line, &bufsize, stdin);
    return line;
}


char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM); /*splits a string into tokens
                                         we store them as an array of pointers*/
    while(token != NULL)
    {
        tokens[position] = token;
        position++;

        if(position >= bufsize) /*reallocate the array if needed*/
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL; /*null terminate the array*/
    return tokens;
}


void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while(status);
}


/***** init *****/

int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
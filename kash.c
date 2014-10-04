#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <errno.h> 
#include <signal.h>
#include <dirent.h>

//Maximum size of number of arguments. 
//Example: "ls | grep input.txt <" has 3 arguments.
#define ARGS_ARRAY_SIZE     20

//How many commands this shell has.
//Only 1 for now unless more custom
//commands are added.

#define COM_LENGTH          1
char *com[COM_LENGTH] = {"exit"};

void findRedirects(char *input);
int parse(char *inputLine, char *arguments[], const char *delimiters);
int findCommand(char *input, char *listOfCommands[], int n);
void forkIt(char *args[]);
void runCommand(int command);
void execToFile(char *args[], char *fileName);

const char *DELIMS = " |<>\n";

#define BUFFER              1024
char line[BUFFER];
char *args[ARGS_ARRAY_SIZE];

//Some inits for the for loops.
int i;
char *p;

int main(){
    while(1){
        printf("kash $ ");
        fgets(line, BUFFER, stdin);
        int redirectStatus = findRedirects(line);
        int count = parse(line, args, DELIMS);
        int command = findCommand(args[0], com, COM_LENGTH);
        printf("%d",command);
        runCommand(command); 
    }
}

/* Looks for '>', '<'. Then it replaces it with a space.
 * Returns:
 * 1 if >
 * 2 if <
 * 0 if no redirects
 * */
void findRedirects(char *input)
{
    for (i = 0; i < strlen(input); i++){
        if (input[i] == '>'){
            input[i] = ' ';
            return 1;
        }
        else if (input[i] == '<'){
            input[i] = ' ';
            return 2;
        }
        else
            return 0;
    }
}

int parse(char *inputLine, char *arguments[], const char *delimiters)
{
    int count = 0;
    printf("Parsed out: ");
    for (p = strtok(inputLine, delimiters); p != NULL; p = strtok(NULL, delimiters))
    {
        arguments[count] = p;
        printf("[%s]", arguments[count]);
        count++;
    }
    putchar('\n');
    arguments[count]=NULL;
    return count;
}


int findCommand(char *input, char *listOfCommands[], int n){
    for (i = 0; i < n; i++){
        if(strcmp(input, listOfCommands[i]) == 0){
          return i;
        }
    }

    //If no commands are found then return -1
    return -1;
}

void runCommand(int command){
    switch (command){
        case -1:
            forkIt(args);
            break;
        case 0:
            exit(1);
            break;
    }
}

void forkIt(char *args[])
{
    int pid = fork();   //Pretty much spawns a new shell.
    printf("pid of new fork: %d\n", pid);
    if (pid == 0){
        execvp(args[0], args);
        printf("Command not found\n");
        exit(1);        //Exits out of duplicate shell.
    }
    else{
        int status;
        waitpid(pid, &status, WCONTINUED);
    }
}

void execToFile(char *args[], char *fileName){
    int out = open(fileName, O_RDWR|O_CREAT|O_APPEND, 0600);
    int save_out = dup(fileno(stdout));

    if (-1 == dup2(out, fileno(stdout)))
        perror("cannot redirect stdout");
    execvp(args[0], args);
    fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);
}

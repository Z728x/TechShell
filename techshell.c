// Name(s):
// Description:

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT 1024
#define ERROR fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));


void displayPrompt();
char* getInput();
char** parseInput(char* input);
void executeCommand(char** command);


int main(void) // MAIN
{
	char *input;
  char **command;
		
	for (;;)
	{
      displayPrompt();
        
	    input = getInput();

	    command = parseInput(input);

      free(input);

      executeCommand(command);

      for (int i = 0; command[i] != NULL; i++) {
        free(command[i]);
      }
      free(command);
      
	}

	exit(0);
}

void displayPrompt() {
  char buf[MAX_INPUT];
  getcwd(buf,MAX_INPUT);
  printf("%s$ ",buf);
}

char* getInput() {
  char *in = malloc(MAX_INPUT);
  if (!fgets(in, MAX_INPUT, stdin)) {
    free(in);
    ERROR
    exit(EXIT_FAILURE);
  }
  return in;
}

char** parseInput(char* input) {
  char **cmd = malloc(MAX_INPUT * sizeof(char*));
  char *tempStr = malloc(MAX_INPUT * sizeof(char));
  int strPos = 0;
  int tempStrPos = 0;
  int cmdPos = 0;
  bool inPara = false;

  while (input[strPos] != '\0') {
    if (input[strPos] == '"' && inPara) {
      tempStr[tempStrPos] = '\0';
      cmd[cmdPos] = malloc(strlen(tempStr) + 1);
      strcpy(cmd[cmdPos],  tempStr);
      tempStrPos = 0;
      tempStr[0] = '\0';
      inPara = false;
      cmdPos++;
    }
    else if (input[strPos] == '"') {
      inPara = true;
    }
    else if (input[strPos] == ' ' && !inPara) {
      if (tempStrPos > 0) {
        tempStr[tempStrPos] = '\0';
        cmd[cmdPos] = malloc(strlen(tempStr) + 1);
        strcpy(cmd[cmdPos],  tempStr);
        tempStrPos = 0;
        tempStr[0] = '\0';
        cmdPos++;
      }
    }
    else if (input[strPos] == '\n'){ }
    else { 
      tempStr[tempStrPos] = input[strPos];
      tempStrPos++;
    }
    strPos++;
 }
 if (tempStrPos > 0) {
  tempStr[tempStrPos] = '\0';
  cmd[cmdPos] = malloc(strlen(tempStr) + 1);
  strcpy(cmd[cmdPos], tempStr);
  cmdPos++;
 }

 cmd[cmdPos] = NULL;
 free(tempStr);
 return cmd;
 
}

  
void executeCommand(char** command) {
  int fin = -1;
  int fout = -1;
  int ferr = -1;
  int commandSize = 0;
  int status;
  
  if ( command[0] == NULL) { 
    return;
  }
  for (int i = 0; command[i] != NULL; i++) {
    commandSize++;
  }
    if (!strcmp(command[0], "exit")) { 
      for (int i = 0; command[i] != NULL; i++) {
        free(command[i]);
      }
      free(command);
      exit(0);
    }

    else if (!strcmp(command[0], "cd")) { if (chdir(command[1]) == -1) { ERROR } }

    else {
        pid_t pid = fork();
        if (pid < 0) {
          ERROR
          return;
        }
        if (pid == 0) { 
          for (int i = 0; i < commandSize ; i++) {
            if (!strcmp(command[i], "<")) {
              fin = open(command[i+1], O_RDONLY, 0644);
              if (fin < 0) { ERROR if (fout != -1) close(fout); if ( ferr != -1) close(ferr); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
            else if (!strcmp(command[i], ">")) {
              fout = open(command[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
              if (fout < 0) { ERROR if (fin != -1) close(fin); if ( ferr != -1) close(ferr); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
            else if (!strcmp(command[i], ">>")) {
              fout = open(command[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
              if (fout < 0) { ERROR if (fin != -1) close(fin); if ( ferr != -1) close(ferr); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
            else if (!strcmp(command[i], "2>")) {
              ferr = open(command[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
              if (ferr < 0) { ERROR if (fout != -1) close(fout); if (fin != -1) close(fin); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
            else if (!strcmp(command[i], "2>>")) {
              ferr = open(command[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
              if (ferr < 0) { ERROR if (fout != -1) close(fout); if (fin != -1) close(fin); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
        }
            
        if (fout != -1) {
          dup2(fout, STDOUT_FILENO);
          close(fout);
        }

        if (fin != -1) {
          dup2(fin, STDIN_FILENO);
          close(fin);
        }
        if (ferr != -1) {
          dup2(ferr, STDERR_FILENO);
          close(ferr);
        }

        if (execvp(command[0],command) == -1) { ERROR _exit(EXIT_FAILURE); }

        }
        waitpid(pid, &status, 0);
      }
      
}




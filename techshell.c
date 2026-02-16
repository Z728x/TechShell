// Name(s): Dawson Benison, Collin Songy
// Description: Simple C implementation of a command line shell. Built in cd and exit functions. Handles >, <, >>, 2>, and 2>> redirection operators.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT 1024 // Macro for used for the max length of input
#define ERROR fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno)); // Macro used for returning error codes

// Function prototypes
void displayPrompt();
char* getInput();
char** parseInput(char* input);
void executeCommand(char** command);


int main(void)
{
	char *input; // Variable used to store the raw input 
  char **command; // Array of Strings stores the tokenized version of the input for execution
		
	for (;;)
	{
      displayPrompt();
        
	    input = getInput();

	    command = parseInput(input);

      free(input);

      executeCommand(command);

      // Loops through command array and frees memory for every index
      for (int i = 0; command[i] != NULL; i++) {
        free(command[i]);
      }
      free(command); // Then frees the array memory itself
      
	}

	exit(0);
}

// Function gets current directory and prints it as prompt
void displayPrompt() {
  char buf[MAX_INPUT];
  getcwd(buf,MAX_INPUT); // grabs current working directory stores in buf
  printf("%s$ ",buf);
}

// Function grabs command input and returns it
char* getInput() {
  char *in = malloc(MAX_INPUT);
  if (!fgets(in, MAX_INPUT, stdin)) { // Attempts to get input, frees memory allocated if failed
    free(in);
    ERROR
    exit(EXIT_FAILURE);
  }
  return in;
}

// Function parses input and returns tokenized version as array of strings
char** parseInput(char* input) {
  char **cmd = malloc(MAX_INPUT * sizeof(char*));
  char *tempStr = malloc(MAX_INPUT * sizeof(char));
  int strPos = 0; // Keeps track of current character in input string
  int tempStrPos = 0; // Keeps track of current character in temp string
  int cmdPos = 0; // Index of current token in command array
  bool inPara = false; // If the current character is within parenthese

  while (input[strPos] != '\0') { // Loops through every character in the input string
    if (input[strPos] == '"' && inPara) { // Checks if the character is the ending parenthese.
      tempStr[tempStrPos] = '\0'; // Ends current tempStr
      cmd[cmdPos] = malloc(strlen(tempStr) + 1);
      strcpy(cmd[cmdPos],  tempStr); // Copies temp string to command at index
      tempStrPos = 0; // Resets tempStrPos for next token
      tempStr[0] = '\0'; // Resets tempStr for next token
      inPara = false; // Next character no longer in parenthese
      cmdPos++; 
    }
    else if (input[strPos] == '"') { // Next characters are in parenthese
      inPara = true;
    }
    else if (input[strPos] == ' ' && !inPara) { // If character is a space and not in parenthese the next characters are part of another token
      if (tempStrPos > 0) { // This makes sure that multiple spaces in a sequence are not considered 
        tempStr[tempStrPos] = '\0';
        cmd[cmdPos] = malloc(strlen(tempStr) + 1);
        strcpy(cmd[cmdPos],  tempStr);
        tempStrPos = 0;
        tempStr[0] = '\0';
        cmdPos++;
      }
    }
    else if (input[strPos] == '\n'){ } // Skip any new line characters
    else {  // Every normal character is appedned to tempStr
      tempStr[tempStrPos] = input[strPos];
      tempStrPos++;
    }
    strPos++; 
 }
 if (tempStrPos > 0) { // If characters are left in tempStr copy to command index
  tempStr[tempStrPos] = '\0';
  cmd[cmdPos] = malloc(strlen(tempStr) + 1);
  strcpy(cmd[cmdPos], tempStr);
  cmdPos++;
 }

 cmd[cmdPos] = NULL; // Last command token is set to null as required by execvp
 free(tempStr); 
 return cmd; 
 
}

// Function to execute the tokenized input 
void executeCommand(char** command) {
  int fin = -1; // Stores input file descriptor
  int fout = -1; // Stores output file descriptor
  int ferr = -1; // Stores error file descriptor
  int commandSize = 0; 
  int status; // Stores status of child process
  
  if ( command[0] == NULL) { // If input is empty the program should just return without executing anything
    return;
  }
  for (int i = 0; command[i] != NULL; i++) { // For loop to get size of command array
    commandSize++;
  }
    if (!strcmp(command[0], "exit")) { // If input is exit command should exit
      for (int i = 0; command[i] != NULL; i++) { // Frees all assigned memory before exiting
        free(command[i]);
      }
      free(command);
      exit(0);
    }

    else if (!strcmp(command[0], "cd")) { if (chdir(command[1]) == -1) { ERROR } } // If command is cd try to change directory if failed give error message

    else { // Runs for normal commands
        pid_t pid = fork(); // Forks so that child process executes execvp
        if (pid < 0) { // If fork failed give error message and return
          ERROR
          return;
        }
        if (pid == 0) {  // Everything executed in the chile process 
          for (int i = 0; i < commandSize ; i++) { // Loops through each command token looking for redirection operators
            if (!strcmp(command[i], "<")) { // Each statements for each operator
              fin = open(command[i+1], O_RDONLY, 0644); // Sets the input file descriptor as the token after the operator, sets to readonly
              if (fin < 0) { ERROR if (fout != -1) close(fout); if ( ferr != -1) close(ferr); _exit(EXIT_FAILURE); } // If the file fails to open, print error message, close any other open file descriptors, and then exit child process
              command[i] = NULL; // Sets the token at index to be NULL so that when executed by execvp everything after is ignored
            }
            else if (!strcmp(command[i], ">")) {
              fout = open(command[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // Sets the output file descriptor as token after operator, creates the file if doesn't exist, sets to write only, and overites any existing data
              if (fout < 0) { ERROR if (fin != -1) close(fin); if ( ferr != -1) close(ferr); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
            else if (!strcmp(command[i], ">>")) {
              fout = open(command[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644); // For the >> operator sets the output file to append output instead of overwriting
              if (fout < 0) { ERROR if (fin != -1) close(fin); if ( ferr != -1) close(ferr); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
            else if (!strcmp(command[i], "2>")) {
              ferr = open(command[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // Sets error file, creates if doesn't exist, sets to write only, and overites any existing data
              if (ferr < 0) { ERROR if (fout != -1) close(fout); if (fin != -1) close(fin); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
            else if (!strcmp(command[i], "2>>")) {
              ferr = open(command[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644); // For 2>> appends to error file instead of overwriting
              if (ferr < 0) { ERROR if (fout != -1) close(fout); if (fin != -1) close(fin); _exit(EXIT_FAILURE); } 
              command[i] = NULL;
            }
        }
            
        if (fout != -1) { // For any open file descriptor redirects, STDIN, STOUT, and STDERR to the file descriptor using dup2
          dup2(fout, STDOUT_FILENO); // dup2 redirects STDOUT, STDIN, or STDERR to appropriate file descriptor
          close(fout); // Closes file descriptor after redirecting data
        }

        if (fin != -1) {
          dup2(fin, STDIN_FILENO);
          close(fin);
        }
        if (ferr != -1) {
          dup2(ferr, STDERR_FILENO);
          close(ferr);
        }

        if (execvp(command[0],command) == -1) { ERROR _exit(EXIT_FAILURE); } // Exectutes commands in command array, gives error message if command fails and exits child process

        }
        waitpid(pid, &status, 0); // Parent process waits for child process to exit
      }
      
}




#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)


#define BUFFER_SIZE 50
static char buffer[BUFFER_SIZE];
int flag = 0;
/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
    int token_count = 0;
    _Bool in_token = false;
    int num_chars = strnlen(buff, COMMAND_LENGTH);
    for (int i = 0; i < num_chars; i++) {
        switch (buff[i]) {
        // Handle token delimiters (ends):
        case ' ':
        case '\t':
        case '\n':
            buff[i] = '\0';
            in_token = false;
            break;
        case '!':
            if (!in_token) {
                tokens[token_count] = &buff[i];
                token_count++;
                in_token = false;
                break;
            }

        // Handle other characters (may be start)
        default:
            if (!in_token) {
                tokens[token_count] = &buff[i];
                token_count++;
                in_token = true;
            }
        }
    }
    tokens[token_count] = NULL;
    return token_count;
}


/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */

void run_command(char **tokens, _Bool *in_background){
  if (*in_background) {
            write(STDOUT_FILENO, "Run in background.\n", strlen("Run in background.\n"));
    }
    pid_t var_pid;
    var_pid = fork();
    if(var_pid < 0 ){
            write(STDOUT_FILENO, "fork failed", strlen("fork failed"));
            exit(-1);
    }
    else if (var_pid == 0){
            if(execvp( tokens[0], tokens) < 0) {
                    write(STDOUT_FILENO, "Error\n", strlen("Error\n"));
                    exit(-1);
            }
            exit(1);
    }
    else do{
            if(!(*in_background)){
                //write(STDOUT_FILENO, "not in background\n", strlen("not in background\n"));
                if((var_pid = waitpid(var_pid, NULL, WNOHANG)) == -1){
                    write(STDOUT_FILENO, "wait Error\n", strlen("wait Error\n"));
                    exit(-1);
                }
                else if(var_pid ==0){
                    //write(STDOUT_FILENO, "Child still running\n", strlen("Child still running\n"));
                }
                else{
                    //write(STDOUT_FILENO, "Child Completed\n", strlen("Child Completed"));
                }
            }

    } while (var_pid == 0);
  }

char history[10][50];

int count = -1;

int add_command(char input_buffer[], char **args){

  int k=1;
  if(count < 10){
    count++;

    char str[50];
    strcpy(str, input_buffer);
    while(args[k] != NULL){
      strcat(str, " ");
      strcat(str, args[k]);
      k++;
    }

    strcpy(history[count], str);
  }
  else{
    for(int k=0 ; k < 9; k++){
      strcpy(history[k], history[k+1]);
    }
    strcpy(history[9], input_buffer);
    count++;
  }
  return 0;
}


void read_command(char *buff, char *tokens[], _Bool *in_background)
{
    *in_background = false;

    // Read input
    int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

    if ( (length < 0) && (errno !=EINTR) ){
        perror("Unable to read command. Terminating.\n");
        exit(-1);  /* terminate with error */
    }

    // Null terminate and strip \n.
    buff[length] = '\0';
    if (buff[strlen(buff) - 1] == '\n') {
        buff[strlen(buff) - 1] = '\0';
    }

    // Tokenize (saving original command string)
    int token_count = tokenize_command(buff, tokens);
    if (token_count == 0) {
        return;
    }
    if(flag ==0){
      add_command(buff, tokens);
    }
    // Extract if running in background:
    if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
        *in_background = true;
        tokens[token_count - 1] = 0;
    }
}

int shell_exit(char **args);
int shell_pwd(char **args);
int shell_cd(char **args);
int shell_help(char **args);
int shell_history(char **args);

char *b_str[] = {
  "exit",
  "pwd",
  "cd",
  "help",
  "history"
};

int (*b_func[]) (char **) = {
  &shell_exit,
  &shell_pwd,
  &shell_cd,
  &shell_help,
  &shell_history
};

// int history_call(char **tokens){
//     if (tokens[0] == NULL){
//         return(0);
//     } else {
//         for (int i = 0; i < 5; i++) {
//             if (strcmp(tokens[0], b_str[i]) == 0) {
//                (*b_func[i])(tokens);

//                return i;
//             }
//          }
//      }
//      return -1;
// }

//executes the built in function if token matches and returns the index of the
int is_built_in(char **tokens){

    
    if (tokens[0] == NULL){
        return(0);
    } else {
        for (int i = 0; i < 5; i++) {
            if (strcmp(tokens[0], b_str[i]) == 0) {
               (*b_func[i])(tokens);

               return i;
            }
         }
     }
     return -1;
}

int print_command(){
  int hcount = count;
  if (count < 10) {
      for(int k = hcount; k >= 0; k--){
        char tmp[12] = {0x0};
        sprintf(tmp, "%11d", hcount);;
          write(STDOUT_FILENO, tmp, sizeof(tmp));
          write(STDOUT_FILENO, "\t", strlen("\t"));
          write(STDOUT_FILENO, history[k], strlen(history[k]));
          write(STDOUT_FILENO, "\n", strlen("\n"));
          hcount--;
      }
  } else {
    for(int i = 9; i >= 0; i--){
      char tmp[12] = {0x0};
    sprintf(tmp, "%11d", hcount);;
    write(STDOUT_FILENO, tmp, sizeof(tmp));
    write(STDOUT_FILENO, "\t", strlen("\t"));
    write(STDOUT_FILENO, history[i], strlen(history[i]));
    write(STDOUT_FILENO, "\n", strlen("\n"));
    hcount--;
      }
  }

  return 1;
}

//when user does history command. Displays the 10 most recent commands in the turtles in a half shell.
//args: *int count, char array[10] of last 10 commands
int shell_history(char **args){
  print_command();
  return 1;
}

//when user does exit command. Exits the shell.
int shell_exit(char **args){
  if(args[1] != NULL){
    write(STDOUT_FILENO, "No argument for exit. Aborting operation.\n", strlen("No argument for exit. Aborting operation.\n"));
    return(-1);
  }
    exit(0);
}

//when user does pwd command. Displays the current directory.
int shell_pwd(char **args){

    if(args[1] != NULL){
        write(STDOUT_FILENO, "No argument for pwd. Aborting operation.\n", strlen("No argument for pwd. Aborting operation.\n"));
    } else {
       char cwd[PATH_MAX];
       if (getcwd(cwd, sizeof(cwd)) != NULL) {
           //donothing
       } else {
           write(STDOUT_FILENO, "getcwd() error\n", strlen("getcwd() error\n"));
       }
       write(STDOUT_FILENO, cwd, strlen(cwd));
       write(STDOUT_FILENO, "\n", strlen("\n"));
       return(1);
    }
    return(-1);
}

//when user does cd command. Changes the current directory.
int shell_cd(char **args){
    if(args[2] != NULL) {
        write(STDOUT_FILENO, "too many arguments\n", strlen("too many arguments\n"));
    } else {
        if(chdir(args[1]) != 0){
            write(STDOUT_FILENO, "cd error\n", strlen("cd error\n"));
        }
    }
    return(1);
}

//when user does help command. Display information about the command they want or if no arguments show all possible commands.
int shell_help(char **args){

    if (args[2] != NULL) {
        write(STDOUT_FILENO, "too many arguments\n", strlen("too many arguments\n"));
    } else {
        if (args[1] == NULL){
            write(STDOUT_FILENO, "The list of commands are: cd, pwd, exit, and help.\ncd is a builtin command for changing the current working directory.\npwd is a builtin command for displaying the current working directory.\nexit is a builtin command for exiting the shell.\n", strlen("The list of commands are: cd, pwd, exit, and help.\ncd is a builtin command for changing the current working directory.\npwd is a builtin command for displaying the current working directory.\nexit is a builtin command for exiting the shell.\n"));
        } else if (strcmp(args[1], "cd") == 0){
            write(STDOUT_FILENO, "cd is a builtin command for changing the current working directory.\n", strlen("cd is a builtin command for changing the current working directory.\n"));
        } else if (strcmp(args[1],"pwd") == 0) {
            write(STDOUT_FILENO, "pwd is a builtin command for displaying the current working directory.\n", strlen("pwd is a builtin command for displaying the current working directory.\n"));
        } else if (strcmp(args[1], "exit") == 0) {
            write(STDOUT_FILENO, "exit is a builtin command for exiting the shell.\n", strlen("exit is a builtin command for exiting the shell.\n"));
        } else { //not internal command
            write(STDOUT_FILENO, "'", strlen("'"));
            write(STDOUT_FILENO, args[1], strlen(args[1]));
            write(STDOUT_FILENO, "' is not an external command or application \n", strlen("' is not an external command or application \n"));
        }
    }
    return(1);
}

/* Signal handler function */
void handle_SIGINT()
{
 write(STDOUT_FILENO, "\nThe list of commands are: cd, pwd, exit, and help.\ncd is a builtin command for changing the current working directory.\npwd is a builtin command for displaying the current working directory.\nexit is a builtin command for exiting the shell.\n", strlen("The list of commands are: cd, pwd, exit, and help.\ncd is a builtin command for changing the current working directory.\npwd is a builtin command for displaying the current working directory.\nexit is a builtin command for exiting the shell.\n"));
 write(STDOUT_FILENO, "\n", strlen("\n"));
 flag = 1;
}

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
    char input_buffer[COMMAND_LENGTH];
    char *tokens[NUM_TOKENS];

    /* set up the signal handler */
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    handler.sa_flags = 0;
    sigemptyset(&handler.sa_mask);
    sigaction(SIGINT, &handler, NULL);

    strcpy(buffer,"I caught a Ctrl-C!\n");

    //printf("Program now waiting for Ctrl-C.\n");

    while (true) {
        // Get command
        // Use write because we need to use read() to work with
        // signals, and read() is incompatible with printf().
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
       //donothing
        } else {
          write(STDOUT_FILENO, "getcwd() error\n", strlen("getcwd() error\n"));
        }
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "$ ", strlen("$ "));
        _Bool in_background = false;
        read_command(input_buffer, tokens, &in_background);
        //DEBUG: Dump out arguments:
        for (int i = 0; tokens[i] != NULL; i++) {
            //write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
            write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
            write(STDOUT_FILENO, "\n", strlen("\n"));
        }

        if(flag == 0){
          int strtoint;
          int numbers = 1; //# of numbers after the '!'
          int isDig = 1;

          if (input_buffer[0] == '!'){
            if(input_buffer[1] == '!'){
              printf("%s\n", history[(count%10)-1]);
              tokenize_command(history[(count%10)-1],tokens);
              strcpy(history[count%10], history[(count%10)-1]);

            } else { 
              while(input_buffer[numbers] != '\0'){
                if(isalpha(input_buffer[numbers])){
                  isDig = 0;
                }
                numbers++;
              }

              if(tokens[1] != NULL && isDig == 1){

                  char str[numbers];
                  strcpy(str, tokens[1]);
                  strtoint = atoi(str);

                  if(count < 10){
                    strcpy(history[count], history[strtoint]);
                    tokenize_command(history[strtoint], tokens);
                  } else if (count >= 10){
                    int getToNine = count - 9;
                    strcpy(history[count-getToNine], history[strtoint-getToNine]);
                    tokenize_command(history[strtoint-getToNine], tokens);
                  }
                }
            }
        }

            
        //DEBUG: Dump out arguments:
          if(is_built_in(tokens) > 0){
              //run the built in command

          }
          else{ //fork external Command
              run_command(tokens, &in_background);
          }
        }
        else{
          flag = 0;
        }

    }
    return(0);
}

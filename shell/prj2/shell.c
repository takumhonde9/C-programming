#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define PATH_SIZE 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10


char current_path[PATH_SIZE];
char history [HISTORY_DEPTH][COMMAND_LENGTH];
int  current_line_number = 0;
_Bool flag_for_signal = false;
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
		// Handle the ! character
		case '!':
			tokens[token_count] = "!";
			token_count++;
			in_token = false;
			break;
		// Handle other characters
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


void add(char *line)
{
        char edited_line[COMMAND_LENGTH+100], ln[100];
        sprintf(ln, "%d", current_line_number+1);
        strcpy(edited_line, ln);
        strcat(edited_line, "\t");
	strcat(edited_line, line);  
        strcpy(history[(current_line_number)%10], edited_line);
        current_line_number=current_line_number+1;
}

void get_tokens(char *tokens[], char *buff, _Bool *in_background)
{
        int token_count = tokenize_command(buff, tokens);
        if (token_count == 0) {
                return;
        }

        // Extract if running in background:
        if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
                *in_background = true;
                tokens[token_count - 1] = 0;
        }
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
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
	if(flag_for_signal == true)
	{
		strcpy(buff, "12");	
		length = 2;
	}
	// need to change code so that if there's a signal read 
	// it clears up the buffer

	if ( (length < 0) && (errno !=EINTR) ){
    		perror("Unable to read command. Terminating.\n");
    		exit(-1);  /* terminate with error */
	}


	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}
        if(buff[0]!='!' && flag_for_signal == false)
                add(buff);

	// Tokenize (saving original command string)
	get_tokens(tokens, buff, in_background);
	flag_for_signal = false;
}

/*
 * To set the path that the shell is currently in. returns current_path+"$ " 
 */
void set_current_path()
{
	getcwd(current_path, PATH_SIZE);
	strcat(current_path, "$ ");
}
/*
 * To retrieve an particular entry from history
 */

char* retrieve(int line_number)
{
	char *history_entry = malloc(COMMAND_LENGTH*sizeof(char*));
	strcpy(history_entry, history[(line_number-1)%10]);
	return history_entry;
}

/*
 * prints the history array entries
 */
void print()
{
	int start, index;
	start = index = current_line_number%10;
	while(true)
	{
		write(STDOUT_FILENO, history[index], strlen(history[index]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
		index = (index+1)%10;
		if(index == start)
			break; 
	}		
}

/*
 * handle ctrl+c signal. not working at all.
 */

void handle_SIGINT()
{
        print();
	flag_for_signal = true;
        //write(STDOUT_FILENO, current_path, strlen(current_path));
        return;
}

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	_Bool execute_from_history = false;
	
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	sigaction(SIGINT, &handler, NULL); 
	while (true) {
		set_current_path();
		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		_Bool in_background = false;
		if(execute_from_history == false)
		{
			write(STDOUT_FILENO, current_path, strlen(current_path));
			read_command(input_buffer, tokens, &in_background);
		}
		
		if(execute_from_history == true) 
			execute_from_history = false;

/*		DEBUG: Dump out arguments:
		for (int i = 0; tokens[i] != NULL; i++) {
			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		
		if (in_background) {
			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}
*/
		pid_t child_pid = fork ();
		if (child_pid < 0)
		{
			write(STDERR_FILENO, "Terminating, could not fork.\n", strlen("Terminating, could not fork.\n"));
			exit(-1);
		} 
		
		if (child_pid == 0)
		{	
	
			if(!(strcmp(tokens[0], "!") == 0 || strcmp(tokens[0], "block") == 0 || strcmp(tokens[0], "history") == 0 || strcmp(tokens[0],"exit") == 0 || strcmp(tokens[0],"pwd") == 0 || strcmp(tokens[0], "cd") == 0 || strcmp(tokens[0], "type") == 0))
				execvp(tokens[0], tokens);
			exit(0);

		}else /* Parent now*/
		{
			char pathname[PATH_SIZE];

                        if(strcmp(tokens[0], "exit") == 0)
                        {       
                                break;
                        }else if(strcmp(tokens[0], "pwd") == 0)
                        {
                       
                                getcwd(pathname, PATH_SIZE);
                                strcat(pathname, "\n");
                                write(STDOUT_FILENO, pathname, strlen(pathname));
                        }else if(strcmp(tokens[0], "cd") == 0)
                        {
				int cdWork =  chdir(tokens[1]);
				if(cdWork == -1)
                                {
                                        write(STDERR_FILENO, "Error. Cannot open directory.\n", strlen("Error. Cannot open directory.\n"));
                                        exit(-1);

                                }
                            
                        }else if(strcmp(tokens[0], "type") == 0)
                        {
                                char str[strlen(tokens[1]+25)];
				strcpy(str,tokens[1]); 
                                if(strcmp(tokens[1], "history") == 0 || strcmp(tokens[1],"exit") == 0 || strcmp(tokens[1],"pwd") == 0 || strcmp(tokens[1], "cd") == 0 || strcmp(tokens[1], "type") == 0 || strcmp(tokens[1], "!") == 0)
                                {
                                        strcat(str, " is a shell300 builtin.\n");

                                }else
				{
                                        strcat(str," is external to shell300.\n");
                                }
                                write(STDOUT_FILENO, str, strlen(str));
                        }else if(strcmp(tokens[0], "history") == 0)
			{
				print();
				
			}else if(strcmp(tokens[0], "!") == 0)
			{	
				int line_number = atoi(tokens[1]), start_line_number = 0;
				if(strcmp(tokens[1], "!") == 0)
					line_number = current_line_number;
				if(current_line_number >= 10)
					start_line_number = current_line_number-9;
			
				if(line_number > current_line_number || line_number < start_line_number || line_number == 0)
					//perror("Error. Line number out of history scope.\n");
				{
					write(STDERR_FILENO, "Error. Line number out of history scope.\n", strlen("Error. Line number out of history scope.\n"));
				}
			
				else
				{	
					int cpy_index = 0;
					char new_cmd[COMMAND_LENGTH];
					strcpy(new_cmd, history[(line_number-1)%10]);
					for(cpy_index=0; cpy_index<COMMAND_LENGTH;cpy_index++)
					{
						char c[2]; c[0] = new_cmd[cpy_index]; c[1] = '\0'; 
						if((strcmp(c, "a") >= 0 && strcmp(c,"z") <= 0) ||
						   (strcmp(c, "A") >= 0 && strcmp(c,"Z") <= 0))
							break;
					}
					char *line = malloc(COMMAND_LENGTH*sizeof(char*));
					line = retrieve(line_number);
					strncpy(new_cmd, line+cpy_index, sizeof(new_cmd));
                                        write(STDOUT_FILENO, new_cmd, strlen(new_cmd));
                                        write(STDOUT_FILENO, "\n", strlen("\n"));
					get_tokens(tokens, new_cmd, &in_background);
					add(line+cpy_index);
					execute_from_history = true;
					free(line);
				}
			}

			if(!in_background)
				wait(NULL);
		// Cleanup any previously exited background child processes
		// (The zombies)
			while (waitpid(-1, NULL, WNOHANG) > 0)
				; // do nothing.

		}

	}

	return 0;
}




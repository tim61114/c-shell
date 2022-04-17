#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "logger.h"
#include "ui.h"


char *next_token(char **str_ptr, const char *delim);

void sigint_handler(int signo) {
    if (signo == SIGINT) {
        return;
    }
}


int main(void)
{
    init_ui();

    signal(SIGINT, sigint_handler);
    char *command;
    while (true) {
        command = read_command();
        if (command == NULL) {
            break;
        }

        char *tokens[100];
        int token_count = 0;
        char *next_tok = command;
        char *curr_tok;

        while ((curr_tok = next_token(&next_tok, " \t\n\r")) != NULL) {
           tokens[token_count++] = curr_tok;
        }
        tokens[token_count] = (char*) 0;
        
        /*if (tokens[0] == NULL) {
            continue;
        }*/

        if (!strcmp(tokens[0], "exit")) {
            break;
        }

        pid_t child = fork();
        if (child == -1) {
            perror("fork");
        } else if (child == 0) {
            execvp(tokens[0], tokens);
            perror("sh");
            return EXIT_FAILURE;
        } else {
            int status;
            wait(&status);
            set_status(status);
        }
    
        //LOG("Input command: %s\n", command);
        //int status;
        /* We are done with command; free it */
        free(command);
    }
//    destroy_ui();
    return 0;
}



char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    char *current_ptr = *str_ptr + tok_start;
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        *str_ptr = NULL;
    } else {
        **str_ptr = '\0';
        (*str_ptr)++;
    }

    return current_ptr;
}


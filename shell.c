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

int execute_command(char **tokens) {
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        exit(1);
    } else if (child == 0) {
        execvp(tokens[0], tokens);
        free(tokens);
        close(STDIN_FILENO);
        perror("sh");
        exit(1);
    } else {
        int status;
        wait(&status);
        return status;
    }
}


char **parse_command(char *command) {
    char **tokens = malloc(1000);
    int token_count = 0;
    char *next_tok = command;
    char *curr_tok;

    while ((curr_tok = next_token(&next_tok, " \t\n\r")) != NULL) {
        tokens[token_count++] = curr_tok;
    }
    free(next_tok);
    tokens[token_count] = (char*) 0;
    return tokens;
}


int command_handler(char *command) {

    char *command_copy = strdup(command);
    char *comment_finder = strchr(command, '#');
    if (comment_finder != NULL) {
        *comment_finder ='\0';
    }

    char **tokens = parse_command(command);

    if (tokens[0] == NULL) {
        free(command_copy);
        return 1;
    }

    if (!strcmp(tokens[0], "exit")) {

        free(command_copy);
        free(tokens);
        return 0;

    } else if (!strcmp(tokens[0], "history")) {

        hist_add(command_copy);
        hist_print();
        free(tokens);

    } else if (!strncmp(tokens[0], "!", 1)) {

        char *param = NULL;
        int res = strtol(&tokens[0][1], &param, 10);

        if (!res) {
            if (tokens[0][1] == '!') {
                free(command_copy);
                free(tokens);
                command_handler((char *)hist_search_cnum(hist_last_cnum()));
                return 1;
                            } else {
                if (!hist_search_prefix(&tokens[0][1])) {
                    free(tokens);
                    free(command_copy);
                    return 1;
                }
                free(command_copy);
                command_handler(strdup(hist_search_prefix(&tokens[0][1])));
                free(tokens);
                return 1;
            }
        } else {
            if (!hist_search_cnum(res)) {
                free(command_copy);
                free(tokens);
                return 1;
            }
            free(command_copy);
            free(tokens);
            command_handler(strdup(hist_search_cnum(res)));
            return 1;
        }
    } else if (!strcmp(tokens[0], "cd")) {
        hist_add(command_copy);
        char path[200];
        char *user = prompt_username();
        if (tokens[1] == NULL) {
            sprintf(path, "/home/%s", user);
        } else if (!strncmp(tokens[1], "~", 1)) {
            sprintf(path, "/home/%s%s",user, tokens[1] + 1);
        } else {
            strcpy(path, tokens[1]);
        }
        free(user);
        int status = chdir(path);
        set_status(status);
        if (status == -1) {
            perror("cd");
        }
        free(command_copy);
        return 1;
    } else {
        set_status(execute_command(tokens));
        hist_add(command_copy);
        free(command_copy);
        return 1;
    }
    
    return 1;

}




int main(void)
{
    init_ui();
    hist_init(100);
    signal(SIGINT, sigint_handler);
    while (true) {
        char *command = read_command();
        //LOG("Input command: %s\n", command);
        if (command == NULL) {
            free(command);
            break;
        }
                 
        if (!command_handler(command)) {
            //free(command);
            break;
        }
                            
        //free(command);
    }
    hist_destroy();
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


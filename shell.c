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
        free(tokens);
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
    tokens[token_count] = (char*) 0;
    return tokens;
}

/*int execute_pipeline(char *command) {
    int fd[2];
    //LOGP(command);
    char *temp = strchr(command, '|');
    char *temptemp;
    if (temp) {
        *temp = '\0';
        temptemp = temp + 1;
        pipe(fd);
        pid_t pid = fork();
        if (pid == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            char **tokens = parse_command(strdup(command));
            LOG("%s\n", command);
            execvp(tokens[0], tokens);
        } else {
            wait(&pid);
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);

            execute_pipeline(temptemp);
        }
    } else {
        LOG("%s\n", command);
        char **tokens = parse_command(command);
        pipe(fd);
        pid_t pid = fork();
        if (pid == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            char **tokens = parse_command(strdup(command));
            execvp(tokens[0], tokens);
        } else {
            return 0;
        }

    }
    return 0;
}*/

// guidelines on freeing memory:
// always free parsed commands (tokens)
// free command_copy if it's not added to history list
// executed tokens freed at execute_tokens
// free other stuff not listed
// 
//command from command_handler(strdup(command))
int command_handler(char *command) {

    char *command_copy = strdup(command);
    char *comment_finder = strchr(command, '#');
    if (comment_finder != NULL) {
        *comment_finder ='\0';
        //free(comment_finder + 1);
    }

    char **tokens = parse_command(command);

    if (tokens[0] == NULL) {
        free(command_copy);
        free(command);
        free(tokens);
        return 1;
    }

    if (!strcmp(tokens[0], "exit")) {
        free(command_copy);
        free(command);
        free(tokens);
        return 0;

    } else if (!strcmp(tokens[0], "history")) {
        hist_add(command_copy);
        hist_print();
        free(command);
        free(tokens);

    } else if (!strncmp(tokens[0], "!", 1)) {

        char *param = NULL;
        int res = strtol(&tokens[0][1], &param, 10);
        
        if (!res) {
            if (tokens[0][1] == '!') {
                free(command_copy);
                free(command);
                free(tokens);
                command_handler(strdup(hist_search_cnum(hist_last_cnum())));
                return 1;
            } else {
                if (!hist_search_prefix(&tokens[0][1])) {
                    free(command_copy);
                    free(command);
                    free(tokens);
                    return 1;
                }
                command_handler(strdup((hist_search_prefix(&tokens[0][1]))));
                free(command_copy);
                free(command);
                free(tokens);
                return 1;
            }
        } else {
            free(command_copy);
            free(command);
            free(tokens);
            if (!hist_search_cnum(res)) {
                return 1;
            }
            command_handler(strdup((hist_search_cnum(res))));
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
        free(command);
        free(tokens);
        return 1;
    } else {
        set_status(execute_command(tokens));
        free(command);
        hist_add(command_copy);
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
        if (command == NULL) {
            free(command);
            break;
        }
                 
        if (!command_handler(strdup(command))) {
            free(command);
            break;
        }
                            
        free(command);
    }
//    hist_print();
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


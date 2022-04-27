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
#include "elist.h"
#include "logger.h"
#include "ui.h"


struct cmd_struct {
    struct elist* tokens;
    bool stdout_pipe;
    bool append;
    char *stdout_file;
    char *stdin_file;
};

char *next_token(char **str_ptr, const char *delim);

void sigint_handler(int signo) {
    if (signo == SIGINT) {
        return;
    }
}

void destroy_cmds(struct elist *list) {
    for (int i = 0; i < elist_size(list); ++i) {
        struct cmd_struct *temp = elist_get(list, i);
        elist_destroy(temp->tokens);
        free(temp);
    }
    elist_destroy(list);
}

void execute_pipeline(struct elist *cmds)
{
    int fd[2];
    int i;

   for (i = 0; ((struct cmd_struct *) elist_get(cmds, i))->stdout_pipe; ++i) {

        pipe(fd);
        pid_t pid = fork();
        if (pid == 0) { 
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            struct cmd_struct *temp = elist_get(cmds, i);
            char *command = elist_get(temp->tokens, 0);            
            execvp(command, (char **) temp->tokens->element_storage);
            close(STDIN_FILENO);
            perror("sh");
            exit(1);

        } else { 
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
        }
    }  
        
   struct cmd_struct *cur_cmd = elist_get(cmds, i);
   int output = cur_cmd->append ? 
       open(cur_cmd->stdout_file, O_CREAT | O_WRONLY | O_APPEND, 0666) : 
       open(cur_cmd->stdout_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
   int input = open(((struct cmd_struct *)elist_get(cmds, i))->stdin_file, O_RDONLY);
   dup2(output, STDOUT_FILENO);
   dup2(input, STDIN_FILENO);

   char *command = elist_get(cur_cmd->tokens, 0);   
   execvp(command, (char **)cur_cmd->tokens->element_storage);

}

int execute_command(struct elist *cmds) { // handler for executions
    pid_t child = fork();

    if (child == -1) {
        perror("fork");
        exit(1);
    } else if (child == 0) {
        execute_pipeline(cmds);
        destroy_cmds(cmds);
        close(STDIN_FILENO);
        perror("sh");
        exit(1);
    } else {
        int status;
        wait(&status);
        destroy_cmds(cmds);
        return status;
    }
}
       
struct elist *tokenize_command(char *command) {
    struct elist *tokens = elist_create(10);
    char *next_tok = command;
    char *curr_tok;

    while ((curr_tok = next_token(&next_tok, " \t\n\r")) != NULL) {
        elist_add(tokens, curr_tok);
    }
    elist_add(tokens, (char *) 0);
    return tokens;
}

struct elist *parse_command (struct elist *command_tok) {
    struct elist *cmds = elist_create(1);
    int i = 0, j = 0;

    while (j < elist_size(command_tok) - 1) {
        struct cmd_struct *temp;
        if (!strcmp(elist_get(command_tok, j), "|")) {    
            
            temp = malloc(sizeof(struct cmd_struct));
            temp->tokens = elist_get_sub(command_tok, i, j - 1);
            elist_add(temp->tokens, (char *) 0);
            temp->stdout_pipe = true;
            temp->append = false;
            temp->stdout_file = NULL;
            temp->stdin_file = NULL;
            elist_add(cmds, temp);
            i = j + 1;

        } else if (!strncmp(elist_get(command_tok, j), ">", 1)) { 

            if (i != j) {
                temp = malloc(sizeof(struct cmd_struct));
                temp->tokens = elist_get_sub(command_tok, i, j - 1); 
                elist_add(temp->tokens, (char *) 0);
                temp->stdout_pipe = false;
                temp->append = strlen(elist_get(command_tok, j)) == 2 ? true : false;
                temp->stdout_file = elist_get(command_tok, j + 1);
                temp->stdin_file = NULL;
                elist_add(cmds, temp);
            } else {
                struct cmd_struct *prev_cmd = elist_get(cmds, elist_size(cmds) - 1);
                prev_cmd->stdout_file = elist_get(command_tok, j + 1);
                prev_cmd->append = strlen(elist_get(command_tok, j)) == 2 ? true : false;
            }
            i = j + 2;
            ++j;

        } else if (!strcmp(elist_get(command_tok, j), "<")) {

            if (i != j) {
                temp = malloc(sizeof(struct cmd_struct));
                temp->tokens = elist_get_sub(command_tok, i, j - 1);
                elist_add(temp->tokens, (char *) 0);
                temp->stdout_pipe = false;
                temp->append = false;
                temp->stdout_file = NULL;
                temp->stdin_file = elist_get(command_tok, j + 1);
                elist_add(cmds, temp);
            } else {
                struct cmd_struct *prev_cmd = elist_get(cmds, elist_size(cmds) - 1);
                prev_cmd->stdin_file = elist_get(command_tok, j + 1);

            }
            i = j + 2;
            ++j;

        }
        ++j;
    }

    //no pipe/io redirection or last command
    if (i < j) {

        struct cmd_struct *temp = malloc(sizeof(struct cmd_struct));
        temp->tokens = elist_get_sub(command_tok, i, elist_size(command_tok) - 1);
        temp->stdout_pipe = false;
        temp->stdout_file = NULL;
        temp->stdin_file = NULL;
        elist_add(cmds, temp);
    }
    elist_destroy(command_tok);
    return cmds;
}

int command_handler(char *command) {

    char *command_copy = strdup(command);
    char *comment_finder = strchr(command, '#');
    if (comment_finder != NULL) {
        *comment_finder ='\0';
    }

    struct elist *tokens = tokenize_command(command);

    if (elist_get(tokens, 0) == NULL) {
        free(command_copy);
        free(command);
        elist_destroy(tokens);
        return 1;
    }

    if (!strcmp(elist_get(tokens, 0), "exit")) {
        free(command_copy);
        free(command);
        elist_destroy(tokens);
        return 0;

    } else if (!strcmp(elist_get(tokens, 0), "history")) {
        hist_add(command_copy);
        hist_print();
        free(command);
        elist_destroy(tokens);
        return 1;

    } else if (!strncmp(elist_get(tokens, 0), "!", 1)) {

        char *param = NULL;
        char *temp = elist_get(tokens, 0);
        int res = strtol(&temp[1], &param, 10);
        
        if (!res) {
            if (temp[1] == '!') {
                free(command_copy);
                free(command);
                elist_destroy(tokens);
                command_handler(strdup(hist_search_cnum(hist_last_cnum())));
                return 1;

            } else {
                if (!hist_search_prefix(&temp[1])) {
                    free(command_copy);
                    free(command);
                    elist_destroy(tokens);
                    return 1;
                }
                command_handler(strdup((hist_search_prefix(&temp[1]))));
                free(command_copy);
                free(command);
                elist_destroy(tokens);
                return 1;
            }

        } else {
            free(command_copy);
            free(command);
            elist_destroy(tokens);
            if (!hist_search_cnum(res)) {
                return 1;
            }
            command_handler(strdup((hist_search_cnum(res))));
            return 1;
        }

    } else if (!strcmp(elist_get(tokens, 0), "cd")) {
        hist_add(command_copy);
        char path[200];
        char *user = prompt_username();
        char *temp = elist_get(tokens, 1);
        if (temp == NULL) {
            sprintf(path, "/home/%s", user);
        } else if (!strncmp(temp, "~", 1)) {
            sprintf(path, "/home/%s%s",user, temp + 1);
        } else {
            strcpy(path, temp);
        }
        free(user);
        int status = chdir(path);
        set_status(status);
        if (status == -1) {
            perror("cd");
        }
        free(command);
        elist_destroy(tokens);
        return 1;

    } else {
        set_status(execute_command(parse_command(tokens)));
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
    destroy_ui();
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


#include <stdio.h>
#include <readline/readline.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdbool.h>

#include "history.h"
#include "logger.h"
#include "ui.h"

static const char *good_str = "👍";
static const char *bad_str  = "🤔";

static int readline_init(void);
int cmd_num_count = 0;
int status = 0;
bool scripting = 0;

void init_ui(void)
{
    LOGP("Initializing UI...\n");

    char *locale = setlocale(LC_ALL, "en_US.UTF-8");
    LOG("Setting locale: %s\n",
            (locale != NULL) ? locale : "could not set locale!");

    rl_startup_hook = readline_init;

    if (!isatty(fileno(stdin))) {
        LOGP("data piped in on stdin; entering script mode\n");
        scripting = 1;
    }
}

void destroy_ui(void)
{
    hist_destroy();   
}

char *prompt_line(void)
{
    const char *status_str = prompt_status() ? bad_str : good_str;

    char cmd_num[25];
    snprintf(cmd_num, 25, "%u", prompt_cmd_num());

    char *user = prompt_username();
    char *host = prompt_hostname();
    char *cwd = prompt_cwd();

    char *format_str = "%s[%s] %s@%s:%s ";

    size_t prompt_sz
        = strlen(format_str)
        + strlen(status_str)
        + strlen(cmd_num)
        + strlen(user)
        + strlen(host)
        + strlen(cwd)
        + 1;

    char *prompt_str =  malloc(sizeof(char) * prompt_sz);

    snprintf(prompt_str, prompt_sz, format_str,
            status_str,
            cmd_num,
            user,
            host,
            cwd);
    
    free(user);
    free(host);
    free(cwd);
    return prompt_str;
}


char *prompt_username(void)
{
    uid_t uid = geteuid();
    struct passwd *user = getpwuid(uid);
    char *username = user != NULL ? strdup(user->pw_name) : NULL;

    return username;
}

char *prompt_hostname(void)
{
    char *buf = malloc(200);    
    gethostname(buf, 200);
    return buf;
}

char *prompt_cwd(void)
{
    char *username = prompt_username();   
    char home[100];
    sprintf(home, "/home/%s", username);
    free(username);
    int home_len = strlen(home);
    char *buf = malloc(200);  
    getcwd(buf, 200);

    if (!strncmp(buf, home, strlen(home))) {
        char *temp = malloc(strlen(buf) - home_len + 2);
        sprintf(temp, "~%s", buf + home_len);
        free(buf);
        return temp;
    }
    return buf;
}

void set_status(int s) {
    status = s;
}

int prompt_status(void)
{
    return status;
}

unsigned int prompt_cmd_num(void)
{
    return cmd_num_count++;
}

char *read_command(void)
{
    char *command = NULL;
    char *prompt = prompt_line();
    if (!scripting) {
        command = readline(prompt);
        free(prompt);
        rl_clear_history();
    } else {
        free(prompt);
        size_t line_sz;
        ssize_t num_chars = getline(&command, &line_sz, stdin); 
        if (num_chars == -1) {
            free(command);
            return NULL;
        }
        *(strrchr(command, '\n')) = '\0';
    }
    return command;
}

int readline_init(void)
{
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_variable_bind("colored-completion-prefix", "on");
    return 0;
}

#include <stdio.h>
#include <readline/readline.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#include "history.h"
#include "logger.h"
#include "ui.h"

static const char *good_str = "😌";
static const char *bad_str  = "🤯";

static int readline_init(void);
int cmd_num_count = 0;
int status = 0;

void init_ui(void)
{
    LOGP("Initializing UI...\n");

    char *locale = setlocale(LC_ALL, "en_US.UTF-8");
    LOG("Setting locale: %s\n",
            (locale != NULL) ? locale : "could not set locale!");

    rl_startup_hook = readline_init;

    if (!isatty(STDIN_FILENO)) {
        LOGP("data piped in on stdin; entering script mode\n");
    }
}

void destroy_ui(void)
{
       // TODO cleanup code, if necessary
    
}

char *prompt_line(void)
{
    //printf("%d\n", prompt_status());
    const char *status = prompt_status() ? bad_str : good_str;

    char cmd_num[25];
    snprintf(cmd_num, 25, "%d", prompt_cmd_num());

    char *user = prompt_username();
    char *host = prompt_hostname();
    char *cwd = prompt_cwd();

    char *format_str = ">>-[%s]-[%s]-[%s@%s:%s]-> ";

    size_t prompt_sz
        = strlen(format_str)
        + strlen(status)
        + strlen(cmd_num)
        + strlen(user)
        + strlen(host)
        + strlen(cwd)
        + 1;

    char *prompt_str =  malloc(sizeof(char) * prompt_sz);

    snprintf(prompt_str, prompt_sz, format_str,
            status,
            cmd_num,
            user,
            host,
            cwd);
    
    free(host);
    free(cwd);
    return prompt_str;
}


char *prompt_username(void)
{
    return getlogin();
}

char *prompt_hostname(void)
{
    char *buf = malloc(200);    
    gethostname(buf, 200);
    return buf;
}

char *prompt_cwd(void)
{
    char *home = malloc( 7 + strlen(prompt_username())); // /home/$USRNAME
    sprintf(home, "/home/%s", prompt_username());
    int home_len = strlen(home);
    //LOGP(home);
    char *buf = malloc(200);  
    getcwd(buf, 200);
    //LOGP(buf);

    if (!strncmp(buf, home, strlen(home))) {
        char *temp = malloc(strlen(buf) - home_len + 2);
        sprintf(temp, "~%s", buf + home_len);
        free(buf);
        free(home);
        return temp;
    }
    free(home);
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
    //implement scripting support here
    //if we are receiving commands from a user, then do the following.
    char *prompt = prompt_line();
    if (isatty(STDIN_FILENO)) {
        char *command = readline(prompt);
        free(prompt);
        return command;
    }
    else {
        size_t line_sz = 0;
        //ssize_t num_chars = getline(&prompt, &line_sz, stdin);
        getline(&prompt, &line_sz, stdin);
        char *command = prompt;
        free(prompt);
        return command;
    }
    //if we are receiving commands from a **script**, then do the following:
    // <insert code that uses getline instead of readline here>
    return "";
}

int readline_init(void)
{
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_variable_bind("colored-completion-prefix", "on");
    return 0;
}

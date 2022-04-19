#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "elist.h"
#include "history.h"

struct elist *history;
int cur_index;
int command_num;

void hist_init(unsigned int limit)
{
    history = elist_create(limit);
    elist_clear_mem(history);
    cur_index = -1;
    command_num = 0;
}

void hist_destroy(void)
{
    for (int i = 0; i < elist_size(history); ++i) {
        char *cmd = (char *) elist_get(history, i);
        free(cmd);
    }
    elist_destroy(history);
    cur_index = -1;
    command_num = 0;
}

void hist_add(const char *cmd)
{
    if (elist_size(history) < 100) {
        elist_add(history, (char *) strdup(cmd));
    } else {
        elist_set(history, (cur_index + 1) % 100, (char *) strdup(cmd));
    }
    cur_index = (cur_index + 1) % 100;
    ++command_num;
}

void hist_print(void)
{
    for (int i = elist_size(history) - 1; i >= 0 ; --i) {
        char *temp = elist_get(history, (cur_index - i + 100) % 100);
        printf("%d %s\n",command_num - i, temp);
    }
}

const char *hist_search_prefix(char *prefix)
{
    for (int i = 0; i < elist_size(history); ++i) {
        char *temp = elist_get(history, (cur_index - i + 100) % 100);
        if (!strncmp(prefix, temp, strlen(prefix))) {
            return temp;
        }
        free(temp);
    }
    return NULL;
}

const char *hist_search_cnum(int command_number)
{
    if (command_number - command_num >= 100 || command_number > command_num) {
        return NULL;
    } else {
        int index = (cur_index - (command_num - command_number) + 100) % 100;
        return elist_get(history, index);
    }

    // TODO: Retrieves a particular command number. Return NULL if no match
    // found.
}

unsigned int hist_last_cnum(void)
{
    return command_num;
}

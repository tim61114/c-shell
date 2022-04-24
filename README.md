# Project 3: Command Line Shell
This is a shell supporting basic commands, pipeline action and I/O redirection.

Things you can do
```
cd
ls
top
cat test.txt > another_file.txt
ls -l | sort | wc
history
!! (Execute previous command)
!(num) (Execute command number (num))
```

## Building


To compile and run:

```bash
make
./sh
```

## Program Options

This is a shell that does shell things (definitely not selling extremely expensive gas)

```bash
$ ./some_prog -h
Usage: ./some_prog [-z] [-d dir]

```

## Included Files

* **shell.c** -- Where the magic happens/ where things get nasty
* **ui.c** -- Handler for input commands and prompt
* **history.c** -- Provide methods to query previous commands
* **elist.c** -- A dynamic data structure made in previous lab, used to store data

## Functions and signatures
### shell.c
* **char** *next_token(char **, const char *)
    * function to parse strings into list
* **void** sigint_handler(int)
    * handle SIGINT
* **void** destroy_cmds(struct elist *)
    * helper function to free an elist of cmd struct
* **void** execute_pipeline(struct elist *)
    * execvp normal/pipeline commands and perform redirection
* **int** execute_command(struct elist *)
    * handler for executing parsed commands
* **struct elist** *tokenize_command(char *)
    * parse input strings into an elist of strings
* **struct elist** *parse_command(struct elist *)
    * parse an elist of strings into an elist of cmd struct
* **int** command_handler(char *command)
    * handle different commands (execvp / history / chdir / exit )

### ui.c
* **char** *prompt_username()
    * return current username
* **char** *prompt_hostname()
    * return current hostname
* **char** *prompt_cwd()
    * return current working directory
* **void** set_status(int)
    * set status code
* **int** prompt_status()
    * return status code
* **unsigned** int prompt_cmd_num()
    * return the current command number
* **char** *read_command()
    * return the input command from terminal or stdin

### history.c
* **void** hist_destroy()
    * delete the history elist container
* **void** hist_add(const char *)
    * add a command into history elist
* **void** hist_print()
    * print history
* **const char** *hist_search_prefix(char *)
    * return the most recent command with prefix or NULL
* **const char** *hist_search_cnum(int)
    * return command number or NULL
* **unsigned int** hist_last_cnum()
    * return the command number of the most recent command

### elist.c
* **struct elist** *elist_get_sub(struct elist *, size_t, size_t) 
    * return an elist sublist of an elist between the two indexes inclusively

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'

# Run a test case in gdb:
make test run=4 debug=on
```

If you are satisfied with the state of your program, you can also run the test cases on the grading machine. Check your changes into your project repository and then run:

```
make grade
```

## Execution flow

![Flow chart](https://user-images.githubusercontent.com/11361069/164952117-66d9463e-146b-4309-8489-5f6803923e17.png)

## Demo Run

[test.png]



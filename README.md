# Project 3: Command Line Shell
This is a shell supporting basic commands, pipeline action and I/O redirection.

Things you can do
```
cd
ls
./sh
top
cat test.txt > another_file.txt
ls -l | sort | wc
```

## Building


To compile and run:

```bash
make
./program_name
```

## Program Options

This is a shell that does shell things (definitely not selling extremely expensive gas)

```bash
$ ./some_prog -h
Usage: ./some_prog [-z] [-d dir]

Options:
    * -d              Directory to load information from.
    * -z              Enable super secret 'Z' mode
```

## Included Files

* **shell.c** -- Where the magic happens/ where things get nasty
* **ui.c** -- Handler for input commands and prompt
* **history.c** -- Provide methods to query previous commands

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

## Demo Run

[test.png]


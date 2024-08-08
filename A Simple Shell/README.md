# A Simple Shell

## Overview
A shell is a program that allows a user to send commands to the operating system (OS) and allows the OS to respond by printing output to the screen. The shell allows a simple character-oriented interface in which the user types a string of characters (terminated by pressing Enter (`\n`)) and the OS responds by printing lines of characters back to the screen. The goal of this project is to implement a simple shell. 

## Typical Shell Interaction
The shell executes the following basic steps in a loop:

1. The shell prints `prompt >` to indicate that it is waiting for instructions.
    ```
    prompt >
    ```
2. The shell gets a command from a user and shows the results of commands to the screen.
    - **Assumption:** The user command is terminated with an `<ENTER>` character (`\n`).
    - **Format of command:** 
      ```
      COMMAND [arg1] [arg2] ... [argn]
      ```
      Each argument is separated by space or tab characters.
      
      **Example:**
      ```
      prompt > ls
      hello.c hello testprog.c testprog
      ```

3. When the shell gets a `quit` command, the shell is terminated.

## Commands that this Shell Support
There are two types of commands:
- **General commands:** These commands are the names of any compiled executable in the local directory.
    ```
    prompt > cd ./project2
    prompt > hello
    ```
- **Built-in commands:** These are performed directly by the shell. Your shell will support six built-in commands: `jobs`, `bg`, `fg`, `kill`, `cd`, `pwd`, and `quit`.

## Stages of the Project
This project consists of five different parts, each with its own score. Test each part after implementation to ensure everything works.

### Stage 1: Accepting Basic Commands
Write code that supports `cd`, `pwd`, and `quit` commands.
- `cd <name of the directory>`: Change the current working directory of the shell. If the given directory name is `..`, then go up one level in the directory tree.
- `pwd`: Print the name of the working directory.
- `quit`: Ends the shell process.

Print the prompt `prompt >` and accept a command from the user. This code should have the ability to extract arguments if the command accepts arguments.

### Stage 2: Local Executables
Support foreground jobs. A foreground job blocks the shell process until completion.
- Write a function that executes the executable program in a child process using `fork()`.
- Ensure all processes forked by the shell process are properly reaped after termination using `wait()` or `waitpid()`.
- Handle `ctrl-C` to terminate the foreground job using a handler for the `SIGINT` signal.

### Stage 3: Background Processes
Support background jobs. A background job does not block the shell.
- Execute a job in the background when an `&` is added to the end of the command line.
- Ensure child processes are reaped using a handler for the `SIGCHLD` signal.

### Stage 4: Job Control
Implement job control to manage jobs in Foreground/Running, Background/Running, and Stopped states.
- Handle `ctrl-C` to terminate a foreground job.
- Handle `ctrl-Z` to stop a foreground job using `SIGTSTP`.
- Implement `fg <job_id|pid>` to move jobs to the foreground.
- Implement `bg <job_id|pid>` to move jobs to the background.
- Implement `kill <job_id|pid>` to terminate jobs.
- Implement `jobs` to list running and stopped background jobs.

Use a struct to record information about each job, including `job_id`, `pid`, and state.

### Stage 5: I/O Redirection
Support I/O redirection:
- `>` to redirect standard output to a file.
- `<` to redirect standard input from a file.
- `>>` to append standard output to a file.

Implement permission bits for file operations.

## Implementation Notes:
- **Headers:** `stdio.h`, `string.h`, `unistd.h`, `stdlib.h`, `sys/stat.h`, `sys/types.h`, `sys/wait.h`, `ctype.h`, `signal.h`, `fcntl.h`.
- **Compilation:** Use `gcc 11.2.0` without additional compiler flags except for `-o`.
- **Constraints:** `MaxLine: 80`, `MaxArgc: 80`, `MaxJob: 5`.
- **Execution:** Use both `execvp()` and `execv()`.


## Example
```c
#include <stdio.h>
#include <unistd.h>

int main() {
    unsigned int i = 0;
    while (1) {
        printf("Counter: %d\n", i);
        i++;
        sleep(1);
    }
    return 0;
}
```

### More Info
For additional details, please refer to the specification document provided as specification.pdf.
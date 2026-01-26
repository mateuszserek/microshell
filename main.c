#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#define DIR_SIZE 4096
#define INPUT_SIZE 256
#define MAX_ARGS 16
#define HELP_MESSAGE "help message 123"

char working_directory[DIR_SIZE];
char *home_directory;
char *user_name;
int shell_signaled = 0;
pid_t child_process = -1;

int kill();

void free_function_args(char *args[]) {
    int i;
    for (i = 0; i < MAX_ARGS; i++) {
        if (args[i] != NULL) {
            free(args[i]);
        } else {
            return;
        }
    }
}

void shell_signal_handler(int sig) {
    shell_signaled = 1;
    signal(sig, shell_signal_handler);
    printf("\n");
}

void child_process_signal_handler(int sig) {
    if (child_process == -1) {
        return;
    }
    
    if (sig == SIGQUIT || sig == SIGINT) {
        kill(child_process, sig);
    }
}

void set_shell_signals() {
    signal(SIGINT, shell_signal_handler);
    signal(SIGQUIT, shell_signal_handler);
    signal(SIGTSTP, shell_signal_handler);
}

void turn_off_shell_signals() {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

void set_working_directory() {
    if(getcwd(working_directory, DIR_SIZE) == NULL) {
        perror("Error while using getcwd()");
    }
}


void cd_command(char *path) {
    int cd_status;
    if (path == NULL || strcmp(path, "~") == 0) {
        cd_status = chdir(home_directory);
    } else {
        cd_status = chdir(path);
    }

    if(cd_status != 0) {
        perror("Couldn't change directory");
    }
    set_working_directory();
}

void parse_input(char *input, char *args[]) {
    if (input[0] == '\0') {
        args[0] = NULL;
        return;
    }
    int i = 0;
    char *ptr = strtok(input, " \t\n");
    while (ptr != NULL && i < MAX_ARGS - 1) {
        args[i] = malloc(strlen(ptr) + 1);
        strcpy(args[i], ptr);
        ptr = strtok(NULL, " \t\n");
        i++;
    }
    args[i] = NULL;
}

void remove_nl(char *str) {
    size_t input_len = strlen(str);
    if (input_len > 0 && str[input_len - 1] == '\n') {
        str[input_len - 1] = '\0';
    }
}

void handle_input(char *input) {
    turn_off_shell_signals();
    char *function_args[MAX_ARGS];
    remove_nl(input);
    parse_input(input, function_args);
    char *command_name = function_args[0];

    if (command_name == NULL) {
        free_function_args(function_args);
        return;
    }

    if(strcmp(command_name, "help") == 0) {
        printf("%s\n", HELP_MESSAGE);
        free_function_args(function_args);
        return;
    }
    
    if (strcmp(command_name, "exit") == 0) {
        free_function_args(function_args);
        exit(0);
    }

    if (strcmp(command_name, "cd") == 0) {
        cd_command(function_args[1]);
        free_function_args(function_args);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
            signal(SIGINT, child_process_signal_handler);
            signal(SIGQUIT,child_process_signal_handler);
            signal(SIGTSTP, child_process_signal_handler);
        if (execvp(command_name, function_args) == -1) {
            printf("%s: command not found\n", command_name);
            exit(errno);
        }
        exit(0);
    } else {
        int status;
        child_process = pid;
        waitpid(pid, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
            printf("\nStopped: %s\n", function_args[0]);
            kill(pid, SIGKILL);
        }
        free_function_args(function_args);
        child_process = -1;
    }
}

int main(int argc, char *argv[]) {
    char input[INPUT_SIZE];
    home_directory = getenv("HOME");
    user_name = getenv("USER");
    set_working_directory();

    while(1) {
        set_shell_signals();
        printf("%s:%s$ ", user_name, working_directory);
        fgets(input, sizeof(input), stdin);
        if (shell_signaled) {
            shell_signaled = 0;
            continue;
        }
        handle_input(input);
    }
    return 0;
}
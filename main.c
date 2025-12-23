#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#define DIR_SIZE 4096
#define INPUT_SIZE 256
#define MAX_ARGS 16
#define HISTORY_FILE_NAME "history.txt"

char *history_file_directory;
char working_directory[DIR_SIZE];
char *home_directory;
char *user_name;

void set_working_directory() {
    if(getcwd(working_directory, DIR_SIZE) == NULL) {
        perror("Error while using getcwd()");
    }
}

void set_history_directory() {
    history_file_directory = malloc(strlen(working_directory) + strlen(HISTORY_FILE_NAME) + 2);
    strcat(history_file_directory, working_directory);
    strcat(history_file_directory, "/");
    strcat(history_file_directory, HISTORY_FILE_NAME);
    creat(history_file_directory, 0644);
}

void save_input_into_history(char *input) {
    if (strcmp(input, "\n") == 0) {
        return;
    }
    int history_file_directory_fd = open(history_file_directory, O_WRONLY | O_APPEND);
    write(history_file_directory_fd, input, strlen(input));
    close(history_file_directory_fd);
}

void cd_command(char *path) {
    int cd_status;
    if (path == NULL) {
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

void handle_command(char *input) {
    char *function_args[MAX_ARGS];
    save_input_into_history(input);
    remove_nl(input);
    parse_input(input, function_args);
    char *command_name = function_args[0];

    if (command_name == NULL) {
        return;
    }
    
    if (strcmp(command_name, "exit") == 0) {
        exit(0);
    }

    if (strcmp(command_name, "cd") == 0) {
        cd_command(function_args[1]);
        return;
    }

    pid_t ps = fork();
    if (ps == 0) {
        if (execvp(command_name, function_args) == -1) {
            perror(command_name);
            exit(errno);
        }
        exit(0);
    } else {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    char input[INPUT_SIZE];
    home_directory = getenv("HOME");
    user_name = getenv("USER");
    set_working_directory();
    set_history_directory();

    while(1) {
        printf("%s:%s$ ", user_name, working_directory);
        fgets(input, sizeof(input), stdin);
        handle_command(input);
    }
    return 0;
}
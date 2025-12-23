#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

#define DIR_SIZE 4096
#define INPUT_SIZE 64
#define MAX_ARGS 16

void set_working_directory(char *dir) {
    if(getcwd(dir, DIR_SIZE) == NULL) {
        perror("Error while using getcwd()");
    }
}

void cd_command(char *path, char *working_dir, char *home_dir) {
    int cd_status;
    if (path == NULL) {
        cd_status = chdir(home_dir);
    } else {
        cd_status = chdir(path);
    }

    if(cd_status != 0) {
        perror("Couldn't change directory");
    }
    set_working_directory(working_dir);
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
        i++;
        ptr = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

void remove_nl(char *str) {
    size_t input_len = strlen(str);
    if (input_len > 0 && str[input_len - 1] == '\n') {
        str[input_len - 1] = '\0';
    }
}

void handle_command(char *input, char *working_dir, char *home_dir) {
    char *function_args[MAX_ARGS];
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
        cd_command(function_args[1], working_dir, home_dir);
        return;
    }

    pid_t ps = fork();
    if (ps == 0) {
        execvp(command_name, function_args);
        perror(command_name);
        exit(0);
    } else {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    char working_directory[DIR_SIZE];
    char input[INPUT_SIZE];
    char *home_dir = getenv("HOME");
    set_working_directory(working_directory);

    while(1) {
        printf("%s: ", working_directory);
        fgets(input, sizeof(input), stdin);
        handle_command(input, working_directory, home_dir);
    }
    return 0;
}
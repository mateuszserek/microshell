#include <stdio.h>
#include <unistd.h>

int main() {

    char *args[] = {"echo", "$HOME", NULL};
    execvp("echo", args);
    return 0;
}
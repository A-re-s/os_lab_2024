#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    for (int i = 0; i < 5; i++) {
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            printf("Child process %d (PID: %d) is running.\n", i + 1, getpid());
            sleep(2); 
            exit(0); 
        }
    }

    printf("Parent process (PID: %d) is waiting...\n", getpid());
    sleep(10); 

    return 0;
}

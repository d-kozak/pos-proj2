/**
 * POS PROJ 2 - Shell
 * David Kozak
 * summer term 2018
 * main file, contains the thread/process management
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pwd.h>

#include "cmdparser.h"

#define BUFFER_SIZE  513
char buffer[BUFFER_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condBufferReady = PTHREAD_COND_INITIALIZER;
pthread_cond_t condBufferProcessed = PTHREAD_COND_INITIALIZER;

pthread_mutex_t pidMutex = PTHREAD_MUTEX_INITIALIZER;
pid_t currentProcPid;


void cleanup(char *inputFile, char *outputFile, char **arguments) {
    if (inputFile != NULL)
        free(inputFile);
    if (outputFile != NULL)
        free(outputFile);
    if (arguments != NULL)
        free(arguments);
}

int redirectStdin(char *file) {
    int fileDesc = open(file, O_RDONLY);
    if (fileDesc == -1) {
        perror("open");
        fprintf(stderr, "Could not open file %s\n", file);
        return -1;
    }
    if (dup2(fileDesc, fileno(stdin)) == -1) {
        perror("dup2");
        close(fileDesc);
        return -1;
    }
    return fileDesc;
}

int redirectStdout(char *file) {
    int fileDesc = open(file, O_WRONLY | O_CREAT, 0644);
    if (fileDesc == -1) {
        perror("open");
        fprintf(stderr, "Could not open file %s\n", file);
        return -1;
    }
    if (dup2(fileDesc, fileno(stdout)) == -1) {
        perror("dup2");
        close(fileDesc);
        return -1;
    }
    return fileDesc;
}

void sigIntCatcher(int signal) {
    pthread_mutex_lock(&pidMutex);
    if (currentProcPid != 0) {
        kill(currentProcPid, SIGINT);
    }
    pthread_mutex_unlock(&pidMutex);
    puts(""); //add newline
}

void sigChildCatcher(int signal) {
    if (signal != SIGCHLD) {
        fprintf(stderr, "Internal error, this should never happen\n");
        return;
    }

    int retCode;
    pid_t pid = wait(&retCode);
    if (pid < 0) {
        // nothing to do
        // you get here only if the process is not in the background
        return;
    }
    if (WIFEXITED(retCode)) {
        printf("[%d] normal termination (exit code = %d)\n", pid, WEXITSTATUS(retCode));
    } else if (WIFSIGNALED(retCode)) {
#ifdef WCOREDUMP
        printf("[%d] signal termination %s(signal = %d)\n", pid, WCOREDUMP(retCode) ? "with core dump" : "",
               WTERMSIG(retCode));
#else
        printf("[%d] signal termination %s(signal = %d)\n", pid,"", WTERMSIG(status));
#endif
    } else {
        printf("[%d] unknown type of termination\n", pid);
    }
}

bool processCommand(char *input) {
    char *command = getCommand(&input);
    bool inBackground = doInBackground(&input);
    char *inputFile = NULL;
    bool inputFileLoaded = getFilename(&input, '<', &inputFile);
    if (!inputFileLoaded) {
        return false;
    }
    char *outputFile = NULL;
    bool outputFileLoaded = getFilename(&input, '>', &outputFile);
    if (!outputFileLoaded) {
        if (inputFile != NULL)
            free(inputFile);
    }
    char **arguments = NULL;
    bool argumentsLoaded = getArguments(command, input, &arguments);
    if (!argumentsLoaded) {
        if (inputFile != NULL)
            free(inputFile);
        if (outputFile != NULL)
            free(outputFile);
    }

    if (strcmp("cd", command) == 0) {
        int retVal;
        if (arguments[1] == NULL || (strcmp("~", arguments[1]) == 0)) {
            struct passwd *pw = getpwuid(getuid());
            const char *homedir = pw->pw_dir;
            retVal = chdir(homedir);
        } else {
            retVal = chdir(arguments[1]);
        }

        if (retVal != 0) {
            perror("cd");
        }
        cleanup(inputFile, outputFile, arguments);
        return retVal == 0;
    }

    pid_t id = fork();
    if (id == -1) {
        perror("fork");
        cleanup(inputFile, outputFile, arguments);
        return false;
    } else if (id == 0) {
        // child
        int inputFileDesc = -1;
        if (inputFile != NULL) {
            if ((inputFileDesc = redirectStdin(inputFile)) == -1) {
                cleanup(inputFile, outputFile, arguments);
                exit(1);
            }
        }
        int outputFileDesc = -1;
        if (outputFile != NULL) {
            if ((outputFileDesc = redirectStdout(outputFile)) == -1) {
                cleanup(inputFile, outputFile, arguments);
                exit(1);
            }
        }
        if (outputFile == NULL && inBackground) {
            if (!redirectStdout("/dev/null")) {
                cleanup(inputFile, outputFile, arguments);
                exit(1);
            }
        }
        int retVal = execvp(command, arguments);
        if (retVal == -1) {
            perror("execvp");
            if (inputFileDesc != -1)
                close(inputFileDesc);
            if (outputFileDesc != -1)
                close(outputFileDesc);
            cleanup(inputFile, outputFile, arguments);
            exit(1);
        }
    } else {

        // parent, just clean up resources and wait
        if (!inBackground) {
            int status;
            pthread_mutex_lock(&pidMutex);
            currentProcPid = id;
            pthread_mutex_unlock(&pidMutex);
            waitpid(id, &status, 0);
            pthread_mutex_lock(&pidMutex);
            currentProcPid = 0;
            pthread_mutex_unlock(&pidMutex);
        } else {
            printf("\n[%d] started\n", id);
        }
        cleanup(inputFile, outputFile, arguments);
        return true;
    }
    // never should end up here...
    fprintf(stderr, "processCommand::internal error\n");
    return false;
}

void loader() {
    int index = 0;
    while (true) {
        pthread_mutex_lock(&mutex);
        while (true) {
            printf("dkozak>");
            fflush(0);
            ssize_t inputLen = read(0, buffer, BUFFER_SIZE);
            if (inputLen >= BUFFER_SIZE) {
                fprintf(stderr, "Input too long, please try something else\n");
                int c;
                while ((c = getchar()) != EOF && c != '\n');
                continue;
            }
            buffer[inputLen - 1] = '\0';
            break;
        }
//        printf("Loader: sending command: '%s'\n", buffer);
        pthread_cond_signal(&condBufferReady);

        if (strcmp(buffer, "exit") == 0) {
//            puts("Loader: Loaded exit, exiting");
            pthread_mutex_unlock(&mutex);
            return;
        }
        index++;

//        puts("Loader: Data sent, preparing to wait for them to be processed");
        while (strcmp(buffer, "") != 0) {
//            puts("Loader: Data not processed yet, waiting...");
            pthread_cond_signal(&condBufferReady);
            pthread_cond_wait(&condBufferProcessed, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *executor(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
//        puts("Executor: Testing the buffer...");
        while (strcmp(buffer, "") == 0) {
//            puts("Executor: Buffer not ready, waiting...");
            pthread_cond_signal(&condBufferProcessed);
            pthread_cond_wait(&condBufferReady, &mutex);
        }

        if (strcmp(buffer, "exit") == 0) {
//            puts("Executor: Received exit, exiting");
            pthread_mutex_unlock(&mutex);
            return NULL;
        }


//        printf("Executor: Processing the command: '%s'\n", buffer);
        processCommand(buffer);
        memset(buffer, '\0', BUFFER_SIZE);

        pthread_cond_signal(&condBufferProcessed);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char **argv) {
    struct sigaction sigact;
    sigact.sa_handler = sigIntCatcher;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);

    if (sigaction(SIGINT, &sigact, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    struct sigaction sigact2;
    sigact2.sa_handler = sigChildCatcher;
    sigact2.sa_flags = 0;
    sigemptyset(&sigact2.sa_mask);

    if (sigaction(SIGCHLD, &sigact2, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    pthread_t pt;
    if (pthread_create(&pt, NULL, executor, NULL) != 0) {
        printf("pthread failed: %d", errno);
        return 1;
    }
    loader();
//    printf("Main: waiting for executor thread to finish\n");
    if (pthread_join(pt, NULL) != 0) {
        printf("join failed: %d", errno);
        return 1;
    }
    return 0;
}

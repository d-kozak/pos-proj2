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

#include "cmdparser.h"

#define BUFFER_SIZE  513
char buffer[BUFFER_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condBufferReady = PTHREAD_COND_INITIALIZER;
pthread_cond_t condBufferProcessed = PTHREAD_COND_INITIALIZER;


bool processCommand(char *input) {
    char *command = getCommand(&input);
    char *inputFile = getFilename(&input, '<');
    char *outputFile = getFilename(&input, '>');
    if (outputFile == NULL) {
        free(inputFile);
    }
    char **arguments;
    bool argumentsLoaded = getArguments(input, &arguments);
    if (!argumentsLoaded) {
        free(inputFile);
        free(outputFile);
    }
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
        printf("Loader: sending command: '%s'\n", buffer);
        pthread_cond_signal(&condBufferReady);

        if (strcmp(buffer, "exit") == 0) {
            puts("Loader: Loaded exit, exiting");
            pthread_mutex_unlock(&mutex);
            return;
        }
        index++;

        puts("Loader: Data sent, preparing to wait for them to be processed");
        while (strcmp(buffer, "") != 0) {
            puts("Loader: Data not processed yet, waiting...");
            pthread_cond_signal(&condBufferReady);
            pthread_cond_wait(&condBufferProcessed, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *executor(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        puts("Executor: Testing the buffer...");
        while (strcmp(buffer, "") == 0) {
            puts("Executor: Buffer not ready, waiting...");
            pthread_cond_signal(&condBufferProcessed);
            pthread_cond_wait(&condBufferReady, &mutex);
        }

        if (strcmp(buffer, "exit") == 0) {
            puts("Executor: Received exit, exiting");
            pthread_mutex_unlock(&mutex);
            return NULL;
        }


        printf("Executor: Processing the command: '%s'\n", buffer);
        processCommand(buffer);
        memset(buffer, '\0', BUFFER_SIZE);

        pthread_cond_signal(&condBufferProcessed);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char **argv) {
    pthread_t pt;
    if (pthread_create(&pt, NULL, executor, NULL) != 0) {
        printf("pthread failed: %d", errno);
        return 1;
    }
    loader();
    printf("Main: waiting for executor thread to finish\n");
    if (pthread_join(pt, NULL) != 0) {
        printf("join failed: %d", errno);
        return 1;
    }
    return 0;
}

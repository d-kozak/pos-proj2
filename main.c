/**
 * POS PROJ 2 - Shell
 * David Kozak
 * summer term 2018
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE  513
char buffer[BUFFER_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condBufferReady = PTHREAD_COND_INITIALIZER;
pthread_cond_t condBufferProcessed = PTHREAD_COND_INITIALIZER;


void loader() {
    const char *messages[] = {"ls", "grep", "foo", "exit"};
    int index = 0;
    while (true) {
        pthread_mutex_lock(&mutex);
        const char *cmd = messages[index];
        memcpy(buffer,cmd,strlen(cmd));
        buffer[strlen(cmd) + 1] = '\0';
        printf("Loader: sending command: %s\n", cmd);
        pthread_cond_signal(&condBufferReady);
        pthread_mutex_unlock(&mutex);

        if (strcmp(cmd, "exit") == 0) {
            puts("Loader: Loaded exit, exiting");
            return;
        }
        index++;

        puts("Loader: Data sent, preparing to wait for them to be processed");
        pthread_mutex_lock(&mutex);
        while(strcmp(buffer,"") != 0) {
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
        while(strcmp(buffer,"") == 0) {
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
        memset(buffer,'\0',BUFFER_SIZE);

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
    if(pthread_join(pt,NULL) != 0){
        printf("join failed: %d", errno);
        return 1;
    }
    return 0;
}

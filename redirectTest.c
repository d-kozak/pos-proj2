//
// Created by david on 11/04/18.
//

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

char buffer[25];

int main() {
    const char *file = "/home/david/tmp/testOutput.txt";
    int out = open(file, O_WRONLY | O_CREAT , 0644);
    if (out == -1) {
        perror("open");
        fprintf(stderr, "cannot open %s\n", file);
        return -1;
    }

    int saveOut = dup(fileno(stdout));

    if (-1 == dup2(out, fileno(stdout))) {
        perror("cannot redirect stdout");
    }

    puts("this goes to the file");

    fflush(stdout);
    close(out);

    dup2(saveOut, fileno(stdout));

    close(saveOut);
    puts("just print");

    int in = open(file, O_RDONLY);
    if (in == -1) {
        perror("open");
        fprintf(stderr, "cannot open %s\n", file);
        return -1;
    }

    int saveIn = dup(fileno(stdin));

    if (-1 == dup2(in, fileno(stdin))) {
        perror("cannot redirect stdout");
    }

    fgets(buffer, 24, stdin);
    printf("Loaded: %s", buffer);

    close(in);

    dup2(saveIn, fileno(stdin));

    close(saveIn);
}
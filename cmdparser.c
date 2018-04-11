/**
 * POS PROJ 2 - Shell
 * David Kozak
 * summer term 2018
 * module for parsing of the arguments
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "cmdparser.h"

char *getEndOfExpression(char *string, int delim) {
    while (*string != delim && *string != '\0') string++;
    return string;
}

char *getCommand(char **cmd) {
    char *endOfCommand = getEndOfExpression(*cmd, ' ');
    unsigned long sizeOfCommand = endOfCommand - (*cmd);
    char *allocatedCmd = malloc(sizeOfCommand + 1);
    if (allocatedCmd == NULL)
        return NULL;
    memcpy(allocatedCmd, *cmd, sizeOfCommand);
    allocatedCmd[sizeOfCommand] = '\0';
    *cmd = (*cmd) + sizeOfCommand;
    return allocatedCmd;
}

bool getFilename(char **input, int delim, char **fileName) {
    char *inputChar = strchr(*input, delim);
    char *delimPostion = inputChar;
    if (inputChar == NULL)
        return true;
    inputChar++;
    while (*inputChar == ' ')
        inputChar++;
    char *endOfFileName = getEndOfExpression(inputChar, ' ');
    unsigned long sizeOfInputFileName = endOfFileName - inputChar;

    *fileName = malloc(sizeOfInputFileName + 1);
    if (*fileName == NULL)
        return false;
    memcpy(*fileName, inputChar, sizeOfInputFileName);
    *fileName[sizeOfInputFileName] = '\0';

    memset(strchr(*input, delim), ' ', endOfFileName - delimPostion);
    return true;
}

int countArguments(char *input) {
    int argumentCount = 0;
    bool isInsideArgument = false;

    while (*input != '\0') {
        if (*input == ' ') {
            if (isInsideArgument) {
                isInsideArgument = false;
                argumentCount++;
            }
        } else if (!isInsideArgument)
            isInsideArgument = true;
        input++;
    }
    if (isInsideArgument)
        argumentCount++;

    return argumentCount;
}

bool getArguments(char *command, char *input, char ***outputArguments) {
    while (*input != '\0' && *input == ' ') input++;

    int argumentCount = countArguments(input);
    char **arguments = calloc((size_t) argumentCount + 1, sizeof(char *));
    if (arguments == NULL) {
        return false;
    }
    int argumentsIndex = 0;
    arguments[argumentsIndex++] = command;

    if (argumentCount == 0) {
        *outputArguments = arguments;
        return true;
    }


    char *argumentStart = input;
    bool wasZeroInserted = false;
    while (*input != '\0' || wasZeroInserted) {
        if (wasZeroInserted)
            wasZeroInserted = false;

        if (*input == ' ') {
            if (argumentStart != NULL) {
                arguments[argumentsIndex++] = argumentStart;
                *input = '\0';
                wasZeroInserted = true;
                argumentStart = NULL;
            }
        } else if (argumentStart == NULL) {
            argumentStart = input;
        }
        input++;
    }
    if (argumentStart != NULL) {
        arguments[argumentsIndex] = argumentStart;
    }
    *outputArguments = arguments;
    return true;

}

bool doInBackground(char **cmd) {
    char *ptr = strchr(*cmd, '&');
    if (ptr != NULL) {
        *ptr = ' ';
        return true;
    } else return false;
}

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

char *getFilename(char **input, int delim) {
    char *inputChar = strchr(*input, delim);
    char *delimPostion = inputChar;
    if (inputChar == NULL)
        return NULL;
    inputChar++;
    while (*inputChar == ' ')
        inputChar++;
    char *endOfFileName = getEndOfExpression(inputChar, ' ');
    unsigned long sizeOfInputFileName = endOfFileName - inputChar;

    char *fileName = malloc(sizeOfInputFileName + 1);
    if (fileName == NULL)
        return NULL;
    memcpy(fileName, inputChar, sizeOfInputFileName);
    fileName[sizeOfInputFileName] = '\0';

    memset(strchr(*input, delim), ' ', endOfFileName - delimPostion);
    return fileName;
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

bool getArguments(char *input, char ***outputArguments) {
    while (*input != '\0' && *input == ' ') input++;

    int argumentCount = countArguments(input);
    if (argumentCount == 0) {
        return true;
    }

    char **arguments = calloc((size_t) argumentCount + 1, sizeof(char *));
    if (arguments == NULL) {
        return false;
    }
    int argumentsIndex = 0;

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

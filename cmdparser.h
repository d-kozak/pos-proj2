/**
 * POS PROJ 2 - Shell
 * David Kozak
 * summer term 2018
 * module for parsing of the arguments
 */
#ifndef POS_PROJ2_CMDPARSER_H
#define POS_PROJ2_CMDPARSER_H


char *getCommand(char **cmd);

char *getFilename(char **input, int delim);

int countArguments(char *input);

bool getArguments(char *input, char ***outputArguments);

#endif //POS_PROJ2_CMDPARSER_H

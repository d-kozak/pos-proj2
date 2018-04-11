/**
 * POS PROJ 2 - Shell
 * David Kozak
 * summer term 2018
 * module for parsing of the arguments
 */
#ifndef POS_PROJ2_CMDPARSER_H
#define POS_PROJ2_CMDPARSER_H


char *getCommand(char **cmd);

bool getFilename(char **input, int delim, char **filename);

int countArguments(char *input);

bool getArguments(char *command, char *input, char ***outputArguments);

bool doInBackground(char **cmd);

#endif //POS_PROJ2_CMDPARSER_H

#include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdbool.h>
# define MAX_LINE_LEN 81
# define MAX_NAME_LEN 20 

/*Opens the .as source file.*/
FILE* openFile(char * filename);

/*Creates the .am file for writing.*/
FILE* createAmFile(char * filename);

/*Reads from the .am file*/
FILE * readAmFile(char * filename);

/*Stores a new line into the line char array. returns true if worked.*/
bool getLine(FILE * fptr, char * line);

/*Every line that we simply want to put back on the file (for pre assembler)*/
void writeInFile(char* line,FILE *nfptr);

FILE * createEntryFile(char * fileName);   

FILE * createExternFile(char * fileName);

FILE * createObjectFile(char * fileName);

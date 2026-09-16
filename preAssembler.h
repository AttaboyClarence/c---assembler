# include <stdbool.h>
# include "file.h"
# define WORDSIZE 32

# define NOTMACRO 0
# define ISMACRO 1
# define MAXLINELEN 81
# define HASH_MACRO_SIZE 7
# define HASH_FUNCTION index*31+*string /*standard polynomial function, using a prime number space*/

typedef struct Line{
    char content[MAXLINELEN];
    struct Line* next;
}Line;

typedef struct Macro{
    char name[MAXLINELEN];
    Line * firstLine;
    struct Macro* next;
}Macro;

typedef struct linkedlist{
    char labelName[WORDSIZE];
    struct linkedlist * next;
}linkedlist;


/*Inserting a macro into the Macro Table.*/
void insertMacro(Macro* arr[], Macro* macro, FILE* fptr);

/*This gets us the index for the correct index in the macro hash table*/
int hashFunc(char * string);

/*We add to the macro it's lines.*/
void addLines(Macro *macro, FILE * current);

/*The main function of the pre assembler, here we start off our pre assembler purpose.*/
bool preAssembler(char *filename);

/*We read onto the new .am file the macro.*/
void readFromMacro(Macro * macro,FILE *nfptr);

/*We check if this macro name has already been used for a different one.*/
Macro* checkIfMacro(Macro* arr[], char * word);

/*Frees all allocated macroes.*/
void freeMacros (Macro * arr []);

void freeSymbols(linkedlist * node);

bool isItSymbol(char * label);

bool isMacroSymbol(linkedlist * symbolList, Macro ** arr);

void addToSymbolList(linkedlist ** symbolList, char * word);
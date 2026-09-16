# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <ctype.h>
# include <stdbool.h>

# define CODE 0
# define DATA 1
# define ENTRY 2
# define PRE_ENTRY -1
# define EXTERNAL 3
# define WORDSIZE 32
# define WORDLEN 13
# define HASHSIZE 11 /*Prime numbers work well with hashtables*/
# define NUM_OP 16
# define OPCODE 5
# define ATTRIBUTES_AM 4 
# define WORDFIRST word[0]
# define LAST strlen(word)-1
# define NUM_OF_REGISTERS 8
# define RAMSIZE 4096 
# define NUMMIUN 4
# define HASHFUNCTION index*31+*string
# define IMMEDIATE_ADDRESS 0
# define DIRECT_ADDRESS 1
# define RELATIVE_ADDRESS 2
# define REGISTER_DIRECT_ADDRESS 3
# define NUM_ATTR 2
# define SYMBOL_PRE_DETECTED 5
# define PRE_DETECTED_RELETIVE 6
# define ADDRESS_SIZE 3
# define OPCODE_SIZE 4
# define FUNCT_SIZE 4
# define ADD_SIZE 2
#define LAST_CHAR_COMMA 1
#define LAST_CHAR_NOTCOMMA 0
#define NOT_REGISTER -1

typedef struct Symbol{
    char name[WORDSIZE];
    int value;
    bool attribute[ATTRIBUTES_AM];
    struct Symbol* next;
}Symbol;

typedef struct Operations{
    char operationName[10];
    char opcode[OPCODE];
    char funct[OPCODE];
    bool source [4];
    bool destinations [4];
}Operations;

/*Each element will be with machinecode + 'A,R,E', or will be empty until it recieves the address of the label*/
typedef struct MachineCode{
    char word[WORDLEN];
    char ARE;
    int place;
}MachineCode;

typedef struct  DynamicArray{
    MachineCode * item;
    int size; 
    int capacity;
}DynamicArray;


typedef struct linkedList{
    char labelName[WORDSIZE];
    int indexInArray;
    struct linkedList * next;
}linkedList;

/*I chose to create a structure that would bind everything in order to not have global variables, and for my simplicity*/
typedef struct AssemStrct{
    Symbol * symbolTable[HASHSIZE];
    linkedList * fixList;
    DynamicArray  instructionArray;
    DynamicArray  dataArray;
    linkedList * externalNode;
    bool ERROR;
    int numOfEntries;
    int IC;
    int DC;
}AssemStrct;


/*I built the assembler, so that the first first pass, will be about
finding symbols and inserting them, and every 'Word' that can be
created- will, and the only ones that will be left will left unattained 
will be 'labels' of addresses that the label was not yet defined. */
void firstRound(char * filename, AssemStrct * assem);


/*We initialize the data and instructuion arrays, as well as initializing other varilbes in assem struct.*/
void initializeArrays(AssemStrct * assem);

/*Here we understand how many arguments we have. Then we seperate into source address 
and destination address (if there are any).*/
void parsingAddresses(AssemStrct * assem,char * rest,const Operations * op, int lineNumber);

/*This function is incharge of creating the room, for the source address.
With the help of other function as follows.*/
void whichSourceAddress(AssemStrct * assem, char  * arg, const Operations * op, int lineNumber );

/*This function is incharge of creating the room, for the destination address.
With the help of other function as follows.*/
void whichDestAddress(AssemStrct * assem, char  * arg, const Operations * op, int lineNumber);

/*Converting the address type, so it will bled into the Instruction binary array, for the last 4 bits*/
void convertAddType(char * st,int addressType);

/*This function helps us conclude which type of addressing type the arg is using. */
int addressType(AssemStrct * assem,char * arg, int lineNum);

/*Once the address type is known, this function converts into the actual data needed
to be converted into binary.*/
int addressNum(AssemStrct * assem,char * arg, int lineNum, int addType);

/*Checks if the argument is a defined register, otherwise returns -1 */
int isRegister(char * arg);

/*In this function we make the conversion to a binary string.*/
void convertNumToBinary(AssemStrct * assem, int inputNum, int lineNumber);

/*We diffirentiate between "data" and "string", and from there we head into 
the next to functions below. Basically this function is a fork.*/
void parseAndWriteinArray(AssemStrct * assem ,char * nextWord ,char * restOfLine, int lineNumber);

/*In the "data" lines, we take numbers between commas and insert the binary
in the data array.*/
void writeNuminArray(AssemStrct * assem, char * string, int lineNumber);

/*Same for string-characaters*/
void writeCharinArray(AssemStrct * assem,char * restOfLine, int lineNumber);

/*We apply the correct A.R.E for each binary machine code.*/
void applyARE(AssemStrct * assem, char * arg, int type);

/*The second round goes througha linked list of 'labels' that at time of running were 
unknown. The linked list holds the label name, and a slot saved for that information,
so the linkedlist will extract the info from the SymbolTable.   */
void secondRound(char * fileName, AssemStrct * assem);

/*Inserts the Symbol (Label) into the SybmbolTable after the Symbol is created, using the
hashFunct for the insertion. */
void insertSymbol(Symbol * s, AssemStrct * assem);

/*creates the symbol.*/
Symbol * createSymbol (char * word, int lineNumber, AssemStrct * assem);

/*In order to use the hash tables for symbols we use this function and it gives us the index.*/
int hashFunct(char * string);

char * skipspaces(char * arg);

/*The function recieves a string and checks whether that word fits a type of opperation */
const Operations * isOp(AssemStrct * assem, char * word, int lineNumber);

/*Checks wether the symbol has data/string in it, and will apply its attributes to the symbol.*/
void isData(char * word, Symbol * symbol);

/*checks if the word is as defined 'Symbol' */
bool isSymbol(char * s, char * label);

/*Here we check if the line is a comment or such, and we move on.*/
bool endOfLineOrComment (char * line);

/*This function moves the pointer to the next word, giving us the rest without the current word we just parsed.*/
char * getRestOfLine(char * line);

/*Gives us the upcoming word*/
void getNextWord(char * rest, char * word);

void convertNumToBinaryString(AssemStrct * assem, int inputNum, int lineNumber);

/*I used dynamic arrays for the Data array and the Instruction array. 
In the manual they explicitly said that we don't know how many items we would had.
So i figured this is the right way to go.
This function increases the capacity if needed, or just allocates room.
If all is correct- the function will return true. */
bool makeRoom(DynamicArray * arr1, DynamicArray * arrOther, int lineNumber);

/*converts the instrction into binary code and places in the Instruction array */
void writeInstructionInArray(AssemStrct * assem,const Operations * op, int sourceType, int desType, int lineNumber);

/*I crop the quotes for the string data.*/
char * cropQuotes(char * string, int lineNumber);

/*This is creating the entry/extern symbol. before we have actually had the definition.*/
void parseAndCreateESymbol(AssemStrct * assem, char * word ,char * rest, int lineNumber);

void writeExternsOnFile(FILE * fpExt,linkedList * node);

void writeObjectFileItems(FILE * fpObj, AssemStrct * assem);

void writeEntriesOnFile(FILE * fpEnt, Symbol * symbolTable []);

/*All usage of extern symbols will be written on an internal 
linked list so that once we are at the end of the second pass,
we will write all the externs onto the extern file.
This is adding in the first pass*/
void addToExtList (AssemStrct * assem, Symbol * symbol);
/*This is adding in the second pass*/
void addToExtrnList (AssemStrct * assem, linkedList * fixP);

/*Adding all the unknown parameters that will later be defined as symbols. */
void addToFixList(AssemStrct * assem,char * arg);

/*Here is basicly the second pass. We go through the list, look via
the symbolTable and insert at the index, where we left room in 
the first pass.*/
void fixAndPatchList(AssemStrct *assem);

/*Here we take from fix list and patch the binary inside the slot saved.*/
void fixConversion(MachineCode * item,int inputNum);

/*converting binary code to numbers so that I can make into hexadecimal and write on object file*/
int convertCodeToNum(MachineCode * code);

/*We free all the allocated memory.*/
void freeAssem(AssemStrct * assem);

void freeExternalNode(linkedList * node);

void freeInstructionArray(DynamicArray * arr);

void freeDataArray(DynamicArray * arr);

void freefixList(linkedList * node);

void freeSymbolTable(Symbol * symbolTable[]);

/*Based on the assignment 23. It's role is to find errors in the commas when parsing data integers  */
void check_Commas(AssemStrct * assem, char * line, int lineNumber);

bool CompatibleWithNoArgs(AssemStrct * assem, const Operations * op, int lineNumber);

void trimArgs(char * arg1, char * arg2);
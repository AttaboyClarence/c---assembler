#include "file.h"


FILE* openFile(char * filename){
FILE* fptr;
    char name[MAX_NAME_LEN]={0};
    strcpy(name,filename);
    strcat(name,".as");
    if ((fptr=fopen(name,"r"))==NULL)
        exit(EXIT_FAILURE);
    return fptr;
}

FILE* createAmFile(char * filename){
    FILE * fptr;
    char newfilename[MAX_NAME_LEN]={0};
    strcpy(newfilename,filename);
    strcat(newfilename,".am");
    fptr = fopen(newfilename,"w");
    if (fptr==NULL){
        printf("ERROR: Could not open %s.\n", newfilename);
        exit(EXIT_FAILURE);
    }
    return fptr;
}

FILE* readAmFile(char * filename){
    FILE * fptr;
    char newfilename[MAX_NAME_LEN]={0};
    strcpy(newfilename,filename);
    strcat(newfilename,".am");
    fptr = fopen(newfilename,"r");
    if (fptr==NULL){
        printf("ERROR: Could not open %s.\n", newfilename);
        exit(EXIT_FAILURE);
    }
    return fptr;
}


bool getLine(FILE * fptr, char * line){
    if (fptr==NULL){
        printf ("ERROR: could not read from file\n");
        return false;
    }
    return (fgets(line,MAX_LINE_LEN,fptr)!=NULL);
}

void writeInFile(char * line,FILE *nfptr){
    fputs(line,nfptr);
}
FILE * createEntryFile(char * fileName){
    FILE * fptr;
    char newfilename[MAX_NAME_LEN]={0};
    strcpy(newfilename,fileName);
    strcat(newfilename,".ent");
    fptr = fopen(newfilename,"w");
    if (fptr==NULL){
        printf("ERROR: Could not open %s.\n", "FILE");
        exit(EXIT_FAILURE);
    }
    return fptr;
}
FILE * createExternFile(char * fileName){
    FILE * fptr;
    char newfilename[MAX_NAME_LEN]={0};
    strcpy(newfilename,fileName);
    strcat(newfilename,".ext");
    fptr = fopen(newfilename,"w");
    if (fptr==NULL){
        printf("ERROR: Could not open %s.\n", "FILE");
        exit(EXIT_FAILURE);
    }
    return fptr;
}
FILE * createObjectFile(char * fileName){
    FILE * fptr;
    char newfilename[MAX_NAME_LEN]={0};
    strcpy(newfilename,fileName);
    strcat(newfilename,".obj");
    fptr = fopen(newfilename,"w");
    if (fptr==NULL){
        printf("ERROR: Could not open %s.\n", "FILE");
        exit(EXIT_FAILURE);
    }
    return fptr;
}





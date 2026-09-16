# include "preAssembler.h"
# include "file.h"

bool preAssembler(char *filename){ /*changed from CHAR * preassembler to FILE * preassembler. dont know if correct check later*/
    linkedlist * symbolList=NULL;
    Macro * arr[HASH_MACRO_SIZE]={0};   
    int n;
    char line[MAXLINELEN]={0};
    char word[WORDSIZE]={0};
    bool noErrors=true;
    FILE *fptr= openFile(filename);
    FILE *nfptr= createAmFile(filename);
    while (fgets(line,MAXLINELEN,fptr)!=NULL) {
        sscanf(line,"%s%n",word,&n);
        if (strcmp(word,"mcro")==0){
            Macro * macro=malloc(sizeof(Macro));    
            sscanf(line+n," %s",macro->name); 
            insertMacro(arr, macro, fptr);/*creating a new macro*/
            }
        /* else check if already macro, and then if so- read from it to newfile*/ 
        else{
            Macro * pmacro=checkIfMacro(arr, word);
            if (pmacro!=NULL){
                readFromMacro(pmacro,nfptr);
            }
        else if(isItSymbol(word)){/*To check at the end if there are symbols with the same name as macros which are prohibited*/
            addToSymbolList(&symbolList,word); 
            writeInFile(line,nfptr);
        }
        else{ /*Not a macro nor a label*/ 
            writeInFile(line,nfptr);    
        }   
        }         
    }
    fclose(fptr);
    fclose(nfptr);
    if(isMacroSymbol(symbolList,arr)){
        memset(word,0,sizeof(word));
        strcpy(word,filename);
        strcat(word,".am");
        remove(word);
        noErrors=false;
    }
    freeMacros(arr);
    freeSymbols(symbolList);
    return noErrors;
}

void freeMacros (Macro * arr []){
    int i;
    Macro * mcCurrent=NULL;
    Macro * mcNext=NULL;
    Line * lineCurrent=NULL;
    Line * lineNext=NULL;
    for (i=0;i<HASH_MACRO_SIZE;i++){
        mcCurrent=arr[i];
        while (mcCurrent!=NULL){
            mcNext=mcCurrent->next;
            lineCurrent=mcCurrent->firstLine;
            while(lineCurrent!=NULL){
                lineNext=lineCurrent->next;
                free(lineCurrent);
                lineCurrent=lineNext;
            }
            lineCurrent=NULL;
            lineNext=NULL;
            free(mcCurrent);
            mcCurrent=mcNext;
        }
        mcCurrent=NULL;
        mcNext=NULL;
    }
}

void freeSymbols(linkedlist * node){
    linkedlist * nextNode=NULL;
    while (node!=NULL){
        nextNode=node->next;        
        free(node);
        node=nextNode;
    }
}
void insertMacro(Macro* arr[], Macro* macro,FILE * fptr ){
    int index= hashFunc(macro->name);/*entering into the Head*/
    if (arr[index]==NULL){
        arr[index]=macro;
        macro->next=NULL;
    }
    else {
        macro->next = arr[index];
        arr[index] = macro;
        
    }
   addLines(macro,fptr);
}


int hashFunc(char * string){/*Using the polynomial rolling hash function*/
    int index=0;
    while (*string!='\0'){
        index = HASH_FUNCTION;
        string++;
    }
    index %= HASH_MACRO_SIZE;
    return index;
}   

void addLines(Macro * macro, FILE *fptr){
    Line * lastLine=NULL;
    Line * newLine=NULL;
    char string[MAXLINELEN];
    char sthelper[MAXLINELEN];
    macro->firstLine=NULL;
    while (fgets(string,MAXLINELEN,fptr)!=NULL){/* new function,*/
                /*check if in hash table*/
        string[strcspn(string, "\n")] = '\0';
        sscanf(string, "%s",sthelper);        
        if (strcmp(sthelper,"mcroend")==0){
            break;
        }
        else{
            newLine=malloc(sizeof(Line));
            if (newLine!=NULL){
                newLine->next=NULL;
                strcpy(newLine->content,string);
            }
            if (macro->firstLine==NULL){ /*for some reason it jumps over even though firstline=null*/
                macro->firstLine=newLine;   
                lastLine=newLine;
            }
            else{
                lastLine->next=newLine;
                lastLine=newLine;
            }
        }                       
    }
        
}

Macro* checkIfMacro(Macro* arr[], char * word){

    int index=0;
    Macro *macro;
    index= hashFunc(word);
    macro=arr[index];
    while (macro!=NULL){
        if (strcmp(macro->name,word)==0){
            return macro;
        }
        macro=macro->next;
    }
   return NULL; 
}

void readFromMacro(Macro * macro,FILE *nfptr){
    Line * line;
    line=macro->firstLine;
    while (line!=NULL){
        writeInFile(line->content,nfptr);
        fputc('\n',nfptr);
        line=line->next;
    }
}

bool isItSymbol(char * label){
    int last=0;
    if (label==NULL || label[0]=='\0')
        return false;
    last = strlen(label)-1;
    if (label[last]==':'){
        label[last]='\0';
        return true;
    }
    return false;
}

void addToSymbolList(linkedlist ** symbolList, char * word){
    linkedlist * temp=NULL;
    temp=calloc(1,sizeof(linkedlist));
    strcpy(temp->labelName,word);
    temp->next=*symbolList;
    *symbolList=temp;
}

bool isMacroSymbol(linkedlist * symbolList, Macro ** arr){

    int index=0;
    bool flagError= false;
    Macro * mp = NULL;
    linkedlist * sp = symbolList;
    while (sp!=NULL){
        index = hashFunc(sp->labelName);
        mp = arr[index];
        while (mp!=NULL){
            if (strcmp(mp->name,sp->labelName)==0){
                printf("ERROR IN PRE ASSEMBLER: The name %s was given both to Macro and a Symbol\n", sp->labelName);
                flagError=true;
            }
            mp=mp->next;
        }
    sp=sp->next;
    }
    return flagError;
}
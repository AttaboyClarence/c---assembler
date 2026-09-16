# include "passes.h"
# include "file.h"

void secondRound(char * fileName ,AssemStrct * assem){
    FILE * fpEnt=NULL;
    FILE * fpExt=NULL;
    FILE * fpObj=NULL;
    fixAndPatchList(assem);
    if (!assem->ERROR){
        fpObj = createObjectFile(fileName);
        writeObjectFileItems(fpObj,assem);
        
        if ((assem->numOfEntries) > 0){
            fpEnt = createEntryFile(fileName);
            writeEntriesOnFile(fpEnt,assem->symbolTable);
        }
        if (assem->externalNode!=NULL){
            fpExt = createExternFile(fileName);
            writeExternsOnFile(fpExt,assem->externalNode);
        }
    }
    freeAssem(assem); 
}
void freeAssem(AssemStrct * assem){
    freeSymbolTable(assem->symbolTable);
    freefixList(assem->fixList);
    freeExternalNode(assem->externalNode);
    freeInstructionArray(&assem->instructionArray);
    freeDataArray(&assem->dataArray);
}
void freeExternalNode(linkedList * node){
    linkedList * nextNode=NULL;
    while (node!=NULL){
        nextNode=node->next;
        free(node);
        node=nextNode;
        nextNode=NULL;
    }
}

void freeInstructionArray(DynamicArray * arr){
    free(arr->item);
}

void freeDataArray(DynamicArray * arr){
    free(arr->item);
}

void freefixList(linkedList * node){
    linkedList * nextNode=NULL;
    while (node!=NULL){
        nextNode=node->next;        
        free(node);
        node=nextNode;
    }
}

void freeSymbolTable(Symbol * symbolTable[]){
int i;
Symbol * symCurrent=NULL;
Symbol * symNext=NULL;
for (i=0;i<HASHSIZE;i++){
    symCurrent=symbolTable[i];
    while (symCurrent!=NULL){
        symNext=symCurrent->next;        
        free(symCurrent);
        symCurrent=symNext;
    }
    symCurrent=NULL;
    symNext=NULL;
    }
}



void writeObjectFileItems(FILE * fpObj, AssemStrct * assem){
    int i=0;
    int count=100;    
    int num=0;
    MachineCode * code;
    printf("\t %d %d\n", assem->IC-100, assem->DC);
    fprintf(fpObj, "\t %d %d\n", assem->IC-100, assem->DC);
    for (i=100; i<assem->IC;i++){
        code = &assem->instructionArray.item[i];
        num = convertCodeToNum(code) & 0xFFF;
        printf("%04d %03X %c\n",count, num, code->ARE);
        fprintf(fpObj,"%04d %03X %c\n",count++, num, code->ARE);
    }
    for (i=0;i<assem->DC;i++){
        code=&assem->dataArray.item[i];
        num = convertCodeToNum(code) & 0xFFF;
        printf("%04d %03X %c\n",count, num, code->ARE);
        fprintf(fpObj,"%04d %03X %c\n",count++, num, code->ARE);
    }
    fclose(fpObj);
}

int convertCodeToNum(MachineCode * code){
    int num=0;
    int i=0;
    for (i=0;i<WORDLEN-2;i++){
        if (code->word[WORDLEN-i-2]=='1'){
            num += (1<<i);
        }
    }
    if (code->word[0]=='1')
        num -= (1<<(WORDLEN-2));
    return num;
}

void writeExternsOnFile(FILE * fpExt,linkedList * node){
    while (node!=NULL){
        fprintf(fpExt,"%s %04d\n",node->labelName,node->indexInArray);
        node=node->next;
    }
    fclose(fpExt);
}

void writeEntriesOnFile(FILE * fpEnt, Symbol * symbolTable []){
    int i=0;
    Symbol * symbol = NULL;
    for (i=0;i<HASHSIZE;i++){
        symbol=symbolTable[i];
        while (symbol!=NULL){
            if (symbol->attribute[ENTRY] && symbol->value==PRE_ENTRY){
                printf("ERROR IN SECOND ROUND: The symbol %s was not declared after the entry statement.\n",symbol->name);
            }
            
            else if (symbol->attribute[ENTRY]){
                    fprintf(fpEnt,"%s %04d\n",symbol->name,symbol->value);
            }
            symbol=symbol->next;
        }
    }
        fclose(fpEnt);
}
/*HAVE TO SORT THINGS OUT WITH RELATIVE*/
void fixAndPatchList(AssemStrct *assem){
    linkedList * fixP= assem->fixList;
    Symbol * sp=NULL;
    char * string;
    int num;
    int index=0;
    while (fixP!=NULL){
    if (*(fixP->labelName)=='%'){        
        string = skipspaces(1+(fixP->labelName));
        index= hashFunct(string);
        sp=assem->symbolTable[index];
        while(sp!=NULL){
            if (strcmp(string,sp->name)==0){
                num = sp->value-(fixP->indexInArray);
                assem->instructionArray.item[fixP->indexInArray].ARE='A';
                fixConversion(&assem->instructionArray.item[fixP->indexInArray],num);
                break;
            }
        sp=sp->next;
        }
    }
    else{        
        index= hashFunct(fixP->labelName);
        sp=assem->symbolTable[index];
        while(sp!=NULL){
            if (strcmp(fixP->labelName,sp->name)==0){
                if (sp->attribute[ENTRY] && sp->value==PRE_ENTRY){
                    printf("ERROR IN SECOND ROUND: The symbol %s was not declared after the entry statement.\n",sp->name);
                    assem->ERROR=true;
                }
                else if (sp->attribute[EXTERNAL]){
                    assem->instructionArray.item[fixP->indexInArray].ARE='E';
                    addToExtrnList(assem,fixP);
                }
                
                else
                    assem->instructionArray.item[fixP->indexInArray].ARE='R';
        fixConversion(&assem->instructionArray.item[fixP->indexInArray],sp->value);
        break;
            }
        sp=sp->next;
        }
    }
        if (sp==NULL){
            assem->ERROR=true;
            printf("ERROR IN SECOND PASS: The symbol %s, was not declared\n",fixP->labelName);
        }
    fixP=fixP->next;
    }
}
void addToExtrnList (AssemStrct * assem, linkedList * fixP){
    linkedList * node = calloc(1,sizeof(linkedList));
    linkedList * temp;
    strcpy(node->labelName,fixP->labelName);
    node->indexInArray=fixP->indexInArray;
    temp=assem->externalNode;
    assem->externalNode=node;
    node->next=temp;
}

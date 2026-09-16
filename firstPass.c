# include <stdlib.h>
# include "passes.h"
# include "file.h"

const Operations opTable[]={
    {"mov","0000","0000",{true,true,false,true},{false,true,false,true}},
    {"cmp","0001","0000",{true,true,false,true},{true,true,false,true}},
    {"add","0010","1010",{true,true,false,true},{false,true,false,true}},
    {"sub","0010","1011",{true,true,false,true},{false,true,false,true}},
    {"lea","0100","0000",{false,true,false,false},{false,true,false,true}},
    {"clr","0101","1010",{false,false,false,false},{false,true,false,true}},
    {"not","0101","1011",{false,false,false,false},{false,true,false,true}},
    {"inc","0101","1100",{false,false,false,false},{false,true,false,true}},
    {"dec","0101","1101",{false,false,false,false},{false,true,false,true}},
    {"jmp","1001","1010",{false,false,false,false},{false,true,true,false}},
    {"bne","1001","1011",{false,false,false,false},{false,true,true,false}},
    {"jsr","1001","1100",{false,false,false,false},{false,true,true,false}},
    {"red","1100","0000",{false,false,false,false},{false,true,false,true}},
    {"prn","1101","0000",{false,false,false,false},{true,true,false,true}},
    {"rts","1110","0000",{false,false,false,false},{false,false,false,false}},
    {"stop","1111","0000",{false,false,false,false},{false,false,false,false}}
};

void firstRound(char * filename, AssemStrct * assem){
    FILE *fptr;
    const Operations * op;
    char line[MAX_LINE_LEN]={0};
    char symbolName[WORDSIZE]={0};
    char word[WORDSIZE]={0};
    char * rest;
    char * dataAfterSymbol=NULL;
    int lineNumber=0;
    Symbol * symbol;
    fptr= readAmFile(filename);
    initializeArrays(assem);
    while (getLine(fptr,line)){
    lineNumber++;
    if (isSymbol(line,symbolName)){
        symbol = createSymbol(symbolName,lineNumber, assem);       
        rest= getRestOfLine(line);
        dataAfterSymbol = rest;
        if (!dataAfterSymbol){
            printf("ERROR IN LINE %d: The label %s was not assigned with any operation or data\n",lineNumber,symbolName);
            assem->ERROR=true;
            continue;
        }
        getNextWord(rest,word);
        isData(word,symbol);/*if is has '.data.' - will add an attribute of "data" while creating the symbol*/
        insertSymbol(symbol,assem);
        if (strcmp(word,".string")==0 || strcmp(word,".data")==0){/*writing the data in the data array*/
            parseAndWriteinArray(assem, word ,rest, lineNumber);
        }
        else {
            op = isOp(assem,word, lineNumber);
            rest = getRestOfLine(rest);/*this should hold now the varibles*/
            parsingAddresses(assem,rest,op, lineNumber);
        }
        memset(symbolName, 0, WORDSIZE);
    }
    else {/*NOT SYMBOL*/
        if (endOfLineOrComment(line)){
                continue;
        }
        getNextWord(line,word);
        if (strcmp(word,".entry")==0 || strcmp(word,".extern")==0){/*writing the data in the data array*/
            rest= getRestOfLine(line);    
            parseAndCreateESymbol(assem, word ,rest, lineNumber);
        }
        else{
            rest= getRestOfLine(line);
            if (strcmp(word,".string")==0 || strcmp(word,".data")==0){/*writing the data in the data array*/
                parseAndWriteinArray(assem, word ,rest, lineNumber);
            }
            else {
                op = isOp(assem,word,lineNumber);
                if (op){
                rest = getRestOfLine(rest);/*this should hold now the varibles*/
                parsingAddresses(assem,rest,op, lineNumber);
                }
            }
        }
    }
    memset(line,0,MAX_LINE_LEN);
    dataAfterSymbol=NULL;
   }
   fclose(fptr);
}
bool endOfLineOrComment (char * line){
    char word[WORDSIZE];
    memset(word,0,WORDSIZE);
    while (isspace(*line))
        line++;
    if (*line =='\0'|| *line=='\n')
        return true;
    getNextWord(line,word);
        if (*word==';')
            return true;
    return false;
}

void parseAndCreateESymbol(AssemStrct * assem, char * word ,char * rest, int lineNumber){
    char symbolName[WORDSIZE]={0};
    int index=0;
    Symbol * symbol=NULL;
    Symbol * newSymbol=NULL;
    if (strcmp(word,".entry")==0){
        getNextWord(rest,symbolName);
        index=hashFunct(symbolName);
        symbol = assem->symbolTable[index];
        while (symbol!=NULL){
            if (strcmp(symbolName, symbol->name)==0 ){
                /*IN THIS CASE, THE SYMBOL WAS ALREADY DECLARED.
                SO WE WOULDN'T WANT TO CREATE ANYTHING ELSE. JUST ADD AN ATTRIBUTE*/
                if (symbol->attribute[EXTERNAL]){
                    printf("ERROR IN LINE %d : The symbol is already stated and is external!\n", lineNumber);
                    assem->ERROR=true;
                }
                else {
                    assem->numOfEntries++;
                    symbol->attribute[ENTRY]=true;
                    return;
                }
            }
        symbol=symbol->next; 
        }
        /*INCASE IT WASN'T CREATED.*/
        newSymbol=calloc(1,sizeof(Symbol));
        strcpy(newSymbol->name,symbolName);
        newSymbol->attribute[ENTRY]=true;
        /*assem->numOfEntries++;*/
        newSymbol->value=PRE_ENTRY;
        insertSymbol(newSymbol,assem);
    }    
    if (strcmp(word,".extern")==0){
        memset(word,0, WORDSIZE);
        getNextWord(rest,word);
        newSymbol=calloc(1,sizeof(Symbol));
        strcpy(newSymbol->name,word);
        newSymbol->attribute[EXTERNAL]=true;
        newSymbol->value=0;
        insertSymbol(newSymbol,assem);
    }
}
     
void initializeArrays(AssemStrct * assem){
    assem->dataArray.item = malloc(5 * sizeof(MachineCode));
    assem->dataArray.size=0;
    assem->dataArray.capacity=5;
    assem->instructionArray.item = malloc(105 * sizeof(MachineCode));
    assem->instructionArray.size=100;
    assem->instructionArray.capacity=105;
    assem->IC=100;
    assem->DC=0; 
    assem->numOfEntries=0;              
}  

void parsingAddresses(AssemStrct * assem,char * rest,const Operations * op, int lineNumber){
    char arg1[WORDSIZE]={0};
    char arg2[WORDSIZE]={0};
    char extra[WORDSIZE];
    int sourceType=0;
    int desType=0;
    if (rest!=NULL){
    int argNum = sscanf(rest, "%[^,\n],%s %s ",arg1,arg2,extra);
    if (argNum>2) {
        assem->ERROR=true;
        printf("ERROR IN LINE %d : Too many arguments\n", lineNumber);
    }
    trimArgs(arg1,arg2);
    if (argNum== 2){
    sourceType = addressType(assem, arg1, lineNumber ); 
    desType = addressType(assem, arg2, lineNumber ); 
    writeInstructionInArray(assem,op,sourceType,desType,lineNumber);
    whichSourceAddress(assem,arg1,op,lineNumber);
    whichDestAddress(assem,arg2,op,lineNumber);
    }
    else if (argNum == 1){
        desType = addressType(assem, arg1, lineNumber ); 
        writeInstructionInArray(assem,op,sourceType,desType,lineNumber);
        whichDestAddress (assem,arg1,op,lineNumber);
    }
    }
    else {
        if(CompatibleWithNoArgs(assem,op,lineNumber))
            writeInstructionInArray(assem,op,sourceType,desType,lineNumber);
    /**/;
    }
}
bool CompatibleWithNoArgs(AssemStrct * assem, const Operations * op, int lineNumber){
    if (strcmp(op->operationName,"rts")==0 || strcmp(op->operationName,"stop")==0)
        return true;
    else {
        printf("ERROR IN LINE %d: The operation %s is incomatible with zero arguments.\n",lineNumber,op->operationName);
        assem->ERROR=true;
        return false;
    }    

}

void whichSourceAddress(AssemStrct * assem, char  * arg, const Operations * op, int lineNumber ){
    int input=0;
    int type = addressType(assem, arg, lineNumber ); 
    if (type==SYMBOL_PRE_DETECTED){
        if (op->source[DIRECT_ADDRESS]==false){
            assem->ERROR=true;
            printf("ERROR IN LINE %d : Uncompatible source address type with operation %s.\n ",lineNumber,op->operationName);
            return;
        }
        else if (makeRoom(&assem->instructionArray,&assem->dataArray,lineNumber)){
                addToFixList(assem,arg);
                return;
        }
    } 
        
    if (op->source[type]==false){
        assem->ERROR=true;
        printf("ERROR IN LINE %d : Uncompatible source address type with operation %s.\n ",lineNumber,op->operationName);
        
    }
    input = addressNum(assem,arg,lineNumber,type);
    convertNumToBinary(assem,input, lineNumber);
    applyARE(assem,arg,type);
}    

void convertNumToBinary(AssemStrct * assem, int inputNum, int lineNumber){
    int i;
    char word[WORDLEN];
    word[WORDLEN-1]='\0';
    for (i=0;i<WORDLEN-1;i++){
        word[WORDLEN-2-i]= ((inputNum>>i)& 1) ? '1':'0';
        }
    if (makeRoom(&assem->instructionArray,&assem->dataArray,lineNumber)){
        strcpy(assem->instructionArray.item[assem->IC].word,word);
    }
}

void applyARE(AssemStrct * assem, char * arg, int type){
    int index=0;
    Symbol * s;
    if (type==DIRECT_ADDRESS || type == RELATIVE_ADDRESS){ 
        index = hashFunct(arg);
        s=assem->symbolTable[index];
        while(s!=NULL){
            if (strcmp(s->name,arg)==0){
                if (s->attribute[EXTERNAL]){
                    assem->instructionArray.item[assem->IC].ARE='E';
                    break;
                }
                else{ 
                    assem->instructionArray.item[assem->IC].ARE='R'; 
                    break;
                }
            }
        s=s->next;
        } 
    }
    else {
        assem->instructionArray.item[assem->IC].ARE='A';
    }
    assem->IC++;
    assem->instructionArray.size++;
}

void writeInstructionInArray(AssemStrct * assem,const Operations * op, int sourceType,int desType, int lineNumber){
    char st[ADDRESS_SIZE];
    char dt[ADDRESS_SIZE];
    MachineCode  * mp=NULL;
    char * p=NULL;
    convertAddType(st,sourceType);
    convertAddType(dt,desType);
    if (makeRoom(&assem->instructionArray,&assem->dataArray,lineNumber)){
        /*might have to change the index into IC++, have to see how it works out*/
        mp = &assem->instructionArray.item[assem->IC++];
        assem->instructionArray.size++;
        p = mp->word;
        memset(p, 0, WORDLEN);
        strcpy(p,op->opcode);
        p += OPCODE_SIZE;
        strcpy(p,op->funct);
        p += FUNCT_SIZE;
        strcpy(p,st);
        p += ADD_SIZE;
        strcpy(p,dt);
        mp->ARE= 'A';
        
    }
}

bool makeRoom(DynamicArray * arr1, DynamicArray * arrOther, int lineNumber){

    MachineCode *temp=NULL;
    /* do we need to allocate memory? */
    if (arr1->size<arr1->capacity)
        return true;

    /* Is there enough RAM to allocate memory?*/
    if (arr1->capacity*2+arrOther->capacity>=RAMSIZE){
        printf("ERROR IN LINE %d: There is no room for more machine code in RAM", lineNumber);
        return false;
    }
    
    arr1->capacity *= 2;
    temp=realloc(arr1->item ,arr1->capacity * sizeof(MachineCode));
    /* did allocation succeed?*/
    if (!temp){
        return false;
    }
    arr1->item = temp;
    return true;
}

void convertAddType(char * st,int addressType){
    int i;
    st[ADDRESS_SIZE-1]='\0';
    if (addressType==0){
        st[0]='0';
        st[1]='0';
    }
    for (i=0;i<ADDRESS_SIZE-1;i++){
        st[ADDRESS_SIZE-2-i]= ((addressType>>i)& 1) ? '1':'0';
        }
    /*IF ITS A NEW LABEL THAT HASNT BEEN DONE- WE'LL STILL GET 01\0 BECAUSE ITS 5*/
}

int addressType(AssemStrct * assem,char * arg, int lineNum){
    int index, num;
    Symbol * s;
    char *endp;
    char *startp;
    if (*arg=='#'){ /*Immediate*/
        startp=++arg;
        num  = strtol(startp,&endp,10);
        if (startp!=endp)
            return IMMEDIATE_ADDRESS;
    }
    /*Register Direct*/
    num=isRegister(arg);
    if (num>0 && num<NUM_OF_REGISTERS){
        return REGISTER_DIRECT_ADDRESS;
    }
    if (*arg=='%'){ /*Relative*/
        /*maybe need another funct -- skipspaces*/
        arg = skipspaces(++arg);
        index = hashFunct(++arg); 
        s = assem->symbolTable[index];
        while (s!=NULL){
        if (strcmp(arg,s->name)==0){
            return RELATIVE_ADDRESS;
            }
        s=s->next;   
        } /*DONT THINK ALL THE ABOVE IS NEEDED==== maybe yes maybe no. we'll see*/
        return PRE_DETECTED_RELETIVE;
    }
    /*MUST ADD ANOTHER OPTION--- IF THE ARG IS A LABEL*/
     /*Direct*/
    index = hashFunct(arg); 
    s = assem->symbolTable[index]; 
    while (s!=NULL){
        if (strcmp(arg,s->name)==0){
            if (s->value==PRE_ENTRY)
                return SYMBOL_PRE_DETECTED;
            return DIRECT_ADDRESS;
        }
    s=s->next;
    }
    return SYMBOL_PRE_DETECTED;
}
int isRegister(char * arg){
    char i;
    char reg[WORDLEN]={0};
    for (i=0;i<NUM_OF_REGISTERS;i++){
        sprintf(reg,"r%d",i);
        if (strcmp(reg,arg)==0){
            return i;     
        }
    }
    return NOT_REGISTER; 
}

char * skipspaces(char * arg){
    while (isspace(*arg)){
        arg++;
    }
    return arg;
}

/*THE TYPES WE HAVE ARE:
    IMMEDIATE: #(TO INSERT A NUMBER INTO) A VARIBLE
    DIRECT: A FUNCTION ON A SINGLE LABEL
    RELATIVE: '%' 

    
    SO THE GO IS TO FIND WHICH TYPE OF ADDRESSING THIS ARG IS. THEN WE
    CHECK IF ITS COMPATIBLE WITH THE OPERATION. THEN IF IT IS- WE PUT
    IN ASSEM->INSTRUCTION.
    OTHERWISE, PRINTF ERROR */
int addressNum(AssemStrct * assem,char * arg, int lineNum, int addType){
    int index, num;
    Symbol * s;
    if (addType==IMMEDIATE_ADDRESS){ 
        arg++;
        return strtol(arg,NULL,10);
    }
    
    if (addType==REGISTER_DIRECT_ADDRESS){
        num=arg[1]-'0';
        if (num<NUM_OF_REGISTERS)
            return 1<<num;
    }
    if (addType==RELATIVE_ADDRESS){ 
        index = hashFunct(++arg); 
        s = assem->symbolTable[index];
        while (s!=NULL){
        if (strcmp(arg,s->name)==0){
            if (s->attribute[EXTERNAL]){ /* HERE AND DIRECT ADDRESSES I ADDED A.R.E EVEN THOUGH NOT THE BEST PLACE FOR IT -MILUIM*/
                assem->instructionArray.item[assem->IC].ARE='E';
                addToExtList(assem,s);
            }
            else 
                assem->instructionArray.item[assem->IC].ARE='R';
            return s->value-(assem->IC+assem->DC);
            }
        s=s->next;   
        }
    }
    if (addType==DIRECT_ADDRESS){
        index = hashFunct(arg); 
        s = assem->symbolTable[index]; 
        while (s!=NULL){
            if (strcmp(arg,s->name)==0){
                if (s->attribute[EXTERNAL]){
                    assem->instructionArray.item[assem->IC].ARE='E';
                    addToExtList(assem,s);
                }
                else 
                    assem->instructionArray.item[assem->IC].ARE='R';
            return s->value;
            }
        s=s->next;
        }
    }
    return 0; 
}

void whichDestAddress(AssemStrct * assem, char  * arg, const Operations * op, int lineNumber){
    int input=0;
    int type = addressType(assem,arg,lineNumber);
    if (type==SYMBOL_PRE_DETECTED || type == PRE_DETECTED_RELETIVE){
        if ((type==SYMBOL_PRE_DETECTED && !op->destinations[DIRECT_ADDRESS]) ||
            (type ==PRE_DETECTED_RELETIVE && !op->destinations[RELATIVE_ADDRESS])){
            assem->ERROR=true;
            printf("ERROR IN LINE %d : Uncompatible source address type with operation %s.\n ",lineNumber,op->operationName);
            return;
        }

        /*remeber to check brakets*/
        if (makeRoom(&assem->instructionArray,&assem->dataArray,lineNumber)){
            addToFixList(assem,arg);
            return;
        }
    }
    if (op->destinations[type]==false){
        assem->ERROR=true;
        printf("ERROR IN LINE %d : Uncompatible source address type with operation %s.\n ",lineNumber,op->operationName);
            return;
    }
    input = addressNum(assem,arg,lineNumber,type);
    convertNumToBinary(assem,input, lineNumber);
    applyARE(assem,arg,type);    
}

void addToExtList (AssemStrct * assem, Symbol * symbol){
    linkedList * node = calloc(1,sizeof(linkedList));
    linkedList * temp;
    strcpy(node->labelName,symbol->name);
    node->indexInArray=assem->IC;
    temp=assem->externalNode;
    assem->externalNode=node;
    node->next=temp;
}

/*If a valid opperation- return a pointer to it and it's attributes.*/
const Operations * isOp(AssemStrct * assem, char * word, int lineNumber){
    int i;
    const Operations * op=NULL;
    for (i=0;i<=NUM_OP;i++){
    if (strcmp(opTable[i].operationName,word)==0){
        op = opTable+i;
        break;
        }
    }

    /*If no name was found matching- false operation name*/
    if (op==NULL){
        printf("ERROR IN LINE %d: There is no operation with the name %s\n",lineNumber, word);
        assem->ERROR=true;
    }
    return op;
}

char * getRestOfLine(char * line){
    while (!isspace(*line) && *line!='\0')/*gets to the end of the first word*/
        line++; 
    while (isspace(*line)&& *line!='\0')/*gets to the next word of the line*/
        line++;
    if (*line=='\0')
        return NULL;
    return line;
}

void getNextWord(char * rest, char * word){
    
    sscanf(rest,"%s",word); 
    
}

Symbol* createSymbol (char * word, int lineNumber, AssemStrct * assem){
    int index= hashFunct(word); 
    Symbol * newSymbol=NULL;
    Symbol * symbolptr =assem->symbolTable[index];
    word[strlen(word)-1]='\0'; /*Taking off the ':' from the end of the word */
    while (symbolptr!=NULL){/*checks if the label name is already used and then prints error*/
        if (strcmp(word,symbolptr->name)==0 && (symbolptr->value!=PRE_ENTRY)){/*addition to the same label twice(which is taken care of)*/
            assem->ERROR=true;
            printf("ERROR IN LINE %d: There cannot be two labels with the same name\n", lineNumber);
            return NULL;
        }
        else {/*IN THE CASE THAT THIS SYMBOL WAS DECLARED WITH ENTRY PRIOR TO THE ACTUAL DEFENITION*/
            if (strcmp(word,symbolptr->name)==0){
                symbolptr->value=assem->IC+assem->DC;
                symbolptr->attribute[ENTRY]=true;
                assem->numOfEntries++;
                return symbolptr;
            }
        }
        symbolptr=symbolptr->next;
    }
    newSymbol=calloc(1,sizeof(Symbol));/*building the symbol*/
    if (!newSymbol) 
        return NULL;
    strcpy(newSymbol->name,word);
    newSymbol->value=assem->IC+assem->DC;
    newSymbol->next=NULL;
    return newSymbol;
    }
/*checks if the word is as defined 'Label', if so it cuts the ':' */
bool isSymbol(char * line, char * label){
    int last;
    if(sscanf(line,"%s",label)==1){
        last = strlen(label)-1;
        if (label[last]==':'){
            return true;
        }
    }
    return false;
}
void isData(char * word, Symbol * symbol){
    if (strcmp(word,".data")==0 || strcmp(word,".string")==0){
        symbol->attribute[DATA]=true;
    }
}

int hashFunct(char * string){/*Using the polynomial rolling hash function*/
    unsigned int index=0;
    /* might be good to have a skip spaces*/
    while (isspace(*string)){
        string++;
    }
    while (*string!='\0' && !isspace(*string)){
        if (*string!=':'){
        index=HASHFUNCTION;
        }
        string++;
    }
    if (isspace(*string))
        *string = '\0'; 
    index %= HASHSIZE;
    return index;
}   

void insertSymbol(Symbol* s, AssemStrct * assem){
    Symbol * temp=NULL;
    int index= hashFunct(s->name); 
    if (assem->symbolTable[index]==NULL){/*IF THIS IS THE FIRST SYMBOL IN THE INDEX*/
        assem->symbolTable[index]=s;
    }
    else { 
        temp = assem->symbolTable[index];
        while (temp!=NULL){    
        if ((strcmp(s->name,temp->name)==0) && (temp->value==PRE_ENTRY)){
            temp->value=s->value;
            free(s);
            return;/*it has already been created and inserted, therefore we free the created symbol.*/
        }
        else if ((strcmp(s->name,temp->name)!=0) && (temp->next==NULL)){
            temp->next=s; 
            s->next=NULL;
            break;
        }
        temp=temp->next;
    }
}
}

void convertNumToBinaryString(AssemStrct * assem,int inputNum, int lineNumber){
    int i;
    char word [WORDLEN];
    word[WORDLEN-1]='\0';
    for (i=0;i<WORDLEN-1;i++){
        word[WORDLEN-2-i]= ((inputNum>>i)& 1) ? '1':'0';
        }
    if (makeRoom(&assem->dataArray,&assem->instructionArray,lineNumber)){
        strcpy(assem->dataArray.item[assem->DC].word,word);
        assem->dataArray.item[assem->DC].ARE='A';
        assem->dataArray.size++;
        assem->DC++;
    }
    
}

void addToFixList(AssemStrct * assem,char * arg){
    if (assem != NULL && assem->fixList != NULL && assem->fixList->labelName[0] != '\0'){
        linkedList * temp;
        linkedList * newfix = calloc(1,sizeof(linkedList));
        strcpy(newfix->labelName,arg);
        newfix->indexInArray=assem->IC++;
        assem->instructionArray.size++;
        temp=assem->fixList;
        assem->fixList=newfix;
        newfix->next=temp;
    }
    else {
        assem->fixList = malloc(sizeof(linkedList));
        strcpy(assem->fixList->labelName,arg);
        assem->fixList->indexInArray=assem->IC++;
        assem->instructionArray.size++;
        assem->fixList->next=NULL;
    }
}

/*MAYBE CREATE THESE AGAIN*/
void parseAndWriteinArray(AssemStrct * assem ,char * nextWord ,char * restOfLine, int lineNumber){
    if (strcmp(nextWord,".data")==0){
        restOfLine = getRestOfLine(restOfLine);
        check_Commas(assem, restOfLine, lineNumber);
        writeNuminArray(assem, restOfLine, lineNumber);
    }
    if(strcmp(nextWord,".string")==0){
        restOfLine = getRestOfLine(restOfLine);       
        writeCharinArray(assem, restOfLine, lineNumber);
    }   
}

void writeCharinArray(AssemStrct * assem,char * restOfLine, int lineNumber){  
    int num=0;
    restOfLine = cropQuotes(restOfLine, lineNumber);
    /*GO TO END OF LINE AND FIND THE FIRST " AND SEE IF PROPPER - STRCHR I THINK*/  
    while(*restOfLine!='\0'){
        if (isspace(*restOfLine)){
            restOfLine++;
            continue;
        }     
        num = *restOfLine; 
        convertNumToBinaryString(assem,num,lineNumber);/*Next time start here- with the conversion function*/
    restOfLine++;    
    }   
    convertNumToBinaryString(assem,0,lineNumber);
}

void writeNuminArray(AssemStrct * assem, char * string, int lineNumber){
    int num=0;
    char * endptr;
    while (*string != '\0' && *string !='\n'){
    num = strtol(string,&endptr,10);
    convertNumToBinaryString(assem,num,lineNumber);
    string=++endptr;
    }
}

char * cropQuotes(char * string, int lineNumber){
    char * p=NULL;
    char * lastQuote = strrchr(string,'"'); 
    if (*string != '"'){
        printf("ERROR IN LINE %d: Invalid string \n" ,lineNumber);
        return NULL;
    } 
    if (string==lastQuote){
        printf("ERROR IN LINE %d: Invalid string\n", lineNumber);
        return NULL;
    }
    string++;
    p = lastQuote+1;
    while (*p!='\n' && *p!='\0' ){
        if (!isspace(p)){
            printf("ERROR IN LINE %d: Invalid string. characters after end of string\n", lineNumber);
            return NULL;
        }
    p++;
    } 
    *lastQuote='\0';
    return string;
}

void fixConversion(MachineCode * item,int inputNum){
    int i;
    item->word[WORDLEN-1] ='\0';
    for (i=0;i<WORDLEN-1;i++){
        item->word[WORDLEN-2-i]= ((inputNum>>i)& 1) ? '1':'0';
    }
}
/*Copied with slight changes from my mySet assignment*/
void check_Commas(AssemStrct* assem, char *line, int lineNumber){
    bool lastCharComma=false;
    char *p;
    p= line;
    while (*p==' '){
        p++;
    }
    if (*p==','){
        printf("ERROR IN LINE %d: Illegal comma\n ",lineNumber);
        assem->ERROR=true;
    }
    while (*p!='\0' && *p!='\n'){
        if (*p==',' && lastCharComma){
            printf ("ERROR IN LINE %d: Multiple consecutive commas\n",lineNumber);
            assem->ERROR=true;
        }
        else if (*p==','){
            lastCharComma=true;
        }
        else if (isdigit(*p)){
            lastCharComma=false;
        }
        p++;
    }
    if (lastCharComma){
        printf("ERROR IN LINE %d: Comma at the end of the line\n",lineNumber);
        assem->ERROR=true;
    }
}
void trimArgs(char * arg1, char * arg2){
    int length1=0;
    int length2=0;
    if (arg1!=NULL){
        length1=strlen(arg1)-1;
        while (arg1[length1]==' ' || arg1[length1]=='\n'){
            arg1[length1]='\0';
            length1--;
        }
    }
    if (arg2!=NULL){
        length2=strlen(arg2)-1;
        while (arg2[length2]==' ' || arg2[length2]=='\n'){
            arg2[length2]='\0';
            length2--;
        }
    }
}
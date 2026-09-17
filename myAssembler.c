# include "preAssembler.h"
# include "passes.h"

/*Testing Git Workflow*/

int main(int argc, char * argv[]){
    
    int i;
    if (argc < 2) {
        printf("Error: No file arguments passed\n");
        return 0;
    }
    for (i=1; i<argc; i++){
        AssemStrct assem={0};
        char filename[MAX_NAME_LEN]={0};
        strcpy(filename,argv[i]);
        /*so far the filename is still .as. must make sure it becomes .am*/
        if(preAssembler(filename)){  
            firstRound(filename,&assem);  
            secondRound(filename,&assem);
        }
    }
    return 0;
}
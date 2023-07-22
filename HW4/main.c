// Omar Alshafei and Hung Tran
// 7-23-23
// HW4 Tiny Compiler


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define IDENT_MAX 11         
#define NUM_MAX 5
#define MAX_SYMBOL_TABLE_SIZE 500
#define CODE_SIZE 1000             

//SYS
#define WRITE 1 
#define READ  2
#define END   3


// token values
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, xorsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym
} token_type;


// symbol table struct 
typedef struct {
    int kind;           // const = 1, var = 2, proc = 3
    char name[10];      // name up to 11 chars
    int val;            // number (ASCII value)
    int level;          // L Level
    int addr;           // M address
    int mark;           // to indicate unavailable or deleted
}symbol;

// token struct 
typedef struct{
    int token;          //value of token type
    int val;            //value of token
    char type[15];      //if it is an identifier, it stores the name of an identifier
    int tokenKind;      //const = 1, var = 2, procedure = 3
}token;

// instruction struct
typedef struct{
    int op;             // op code
    int l;              // lexLevel
    int m;              // constant or address
}instruction;

symbol symbolTable[MAX_SYMBOL_TABLE_SIZE]; 
instruction assemblyCode[CODE_SIZE];           //stack that stores all code
int tableIndex = 1;                        //current index of symbol table
int idx = 0;                                //current index of token array
int tokenIndex = 0;       // current index of tokenArr
int line = 0;                               //current line
int flag = 0;                               //used for printing assemblyCode when line = 0
int codeIndex = 0;                          //current index of code stack
int lineTracker = 0;
int errorflag = 0;                          //used for printing error when from code start to error then keep printing if there is no error after
int lexLevel = 0;                                  //Max lexicographical level reached.
int mainIdx = 0; 

void procDestination(token tokenArray[], FILE* fp);
void printOut(FILE* fp);
int isSpecialSymbol(char ch);
int checkTable(char string[], int string_kind);
void program(token tokenArray[], FILE* fp);
void block(token tokenArray[], FILE* fp);
void constDeclaration(token tokenArray[], FILE* fp);
int varDeclaration(token tokenArray[], FILE* fp);
void statement(token tokenArray[], FILE* fp);
void condition(token tokenArray[], FILE* fp);
void expression(token tokenArray[], FILE* fp);
void term(token tokenArray[], FILE* fp);
void factor(token tokenArray[], FILE* fp);
void addTable(int, char*, int, int, int);
void emit(int op, int l, int m);
void errorRecovery(token tokenArray[]);
void procDeclaration(token tokenArray[], int size, FILE* fp);

int isSpecialSymbol(char c){
    if(c == '+' || c == '-' || c == '*' || c == '/') return 1; 

    if(c == '(' || c == ')' || c == '=' || c == ',') return 1; 

    if(c == '.' || c == '<' || c == '>' || c == ';' || c == ':') return 1; 

    return 0;
}

// driver function 
int main(int argc, char *argv[]) {

    FILE *fp;
    fp = fopen(argv[1], "r"); 


    if (fp == NULL){
        printf("File CANNOT be open!");
        
        exit(1);
    }

    // ----------------------------------- HW2 -----------------------------------

    // local variables 
    char reservedWords[15][20] = {"const", "var", "odd", "call", "begin", "end", "if", "then",
                                  "xor", "else", "while", "do", "read", "write", "procedure"};
    char* inputStr;     // hold the input from file
    char buffer[25];    // temporary string 
    int len;            // length of the file
    int cur = 0;        // current index of inputStr
    int bufferIdx = 0;  // current index of buffer
    int symbolFlag = 0;

    // calculating the size of the file
    fseek(fp, 0, SEEK_END); 
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // read in file
    inputStr = malloc(sizeof(char) * (len + 1));
    while(fscanf(fp, "%c", &inputStr[cur]) != EOF)
        cur++; 

    token tokenArr[len];        // token array that will hold the info for each token 
    
    printf("\n%s\n\n", inputStr);

    // run through the input by char to determine the token 
    cur = 0; 
    while(cur < len){

        // check if current char is a white space
        if(isspace(inputStr[cur]) != 0){
            cur++; 
            continue;
        }
        
        // check for comment
        else if (inputStr[cur] == '/' && inputStr[cur + 1] == '*' ){
            cur += 2;

            // FIXME - recheck logic
            while(inputStr[cur] != '*' || inputStr[cur + 1] != '/'){
                cur++;
            }
            cur += 2;
        }        

        // check if current char is a valid identifier or a reserved word
        else if(isalpha(inputStr[cur]) != 0 && isspace(inputStr[cur]) == 0){
            
            while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0){
                
                // check if identifier is over 11 char, error handling
                if(bufferIdx >= IDENT_MAX){
                    printf("\nError: Identifier names cannot exceed 11 characters\n");
                    flag = -1;
                    exit(1);
                    while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0)                 
                        cur++;

                    strcpy(buffer, "NO GOOD");
                }

                else{
                    buffer[bufferIdx++] = inputStr[cur++];
                }
            }

            buffer[bufferIdx] = '\0';

            // check if the identifier is a reserved word
            // const
            if(strcmp(buffer, reservedWords[0]) == 0){
                strcpy(tokenArr[tokenIndex].type, "const");
                tokenArr[tokenIndex].token = constsym; 
            }

            // var
            else if(strcmp(buffer, reservedWords[1]) == 0){
                strcpy(tokenArr[tokenIndex].type, "var");
                tokenArr[tokenIndex].token = varsym; 
            }

            // odd
            else if(strcmp(buffer, reservedWords[2]) == 0){
                strcpy(tokenArr[tokenIndex].type, "odd");
                tokenArr[tokenIndex].token = oddsym; 
            }

            // call
            else if(strcmp(buffer, reservedWords[3]) == 0){
                strcpy(tokenArr[tokenIndex].type, "call");
                tokenArr[tokenIndex].token = callsym; 
            }

            // begin
            else if(strcmp(buffer, reservedWords[4]) == 0){
                strcpy(tokenArr[tokenIndex].type, "begin");
                tokenArr[tokenIndex].token = beginsym; 
            }

            // end
            else if(strcmp(buffer, reservedWords[5]) == 0){
                strcpy(tokenArr[tokenIndex].type, "end");
                tokenArr[tokenIndex].token = endsym; 
            }

            // if
            else if(strcmp(buffer, reservedWords[6]) == 0){
                strcpy(tokenArr[tokenIndex].type, "if");
                tokenArr[tokenIndex].token = ifsym; 
            }

            // then
            else if(strcmp(buffer, reservedWords[7]) == 0){
                strcpy(tokenArr[tokenIndex].type, "then");
                tokenArr[tokenIndex].token = thensym; 
            }

            // xor
            else if(strcmp(buffer, reservedWords[8]) == 0){
                strcpy(tokenArr[tokenIndex].type, "xor");
                tokenArr[tokenIndex].token = xorsym; 
            }

            // else
            else if(strcmp(buffer, reservedWords[9]) == 0){
                strcpy(tokenArr[tokenIndex].type, "else");
                tokenArr[tokenIndex].token = elsesym; 
            }

            // while
            else if(strcmp(buffer, reservedWords[10]) == 0){
                strcpy(tokenArr[tokenIndex].type, "while");
                tokenArr[tokenIndex].token = whilesym; 
            }

            // do
            else if(strcmp(buffer, reservedWords[11]) == 0){
                strcpy(tokenArr[tokenIndex].type, "do");
                tokenArr[tokenIndex].token = dosym; 
            }

            // read
            else if(strcmp(buffer, reservedWords[12]) == 0){
                strcpy(tokenArr[tokenIndex].type, "read");
                tokenArr[tokenIndex].token = readsym; 
            }

            // write
            else if(strcmp(buffer, reservedWords[13]) == 0){
                strcpy(tokenArr[tokenIndex].type, "write");
                tokenArr[tokenIndex].token = writesym; 
            }

            // procedure
            else if(strcmp(buffer, reservedWords[14]) == 0){
                strcpy(tokenArr[tokenIndex].type, "procedure");
                tokenArr[tokenIndex].token = procsym; 
            }

            // Error handling
            else if(strcmp(buffer, "NO GOOD") == 0){
                bufferIdx = 0;
                continue;
            }

            // identifier 
            else{
                strcpy(tokenArr[tokenIndex].type, buffer);
                tokenArr[tokenIndex].token = identsym; 
            }
                     
            tokenIndex++;
            bufferIdx = 0;
            cur--;
        }

        // check if current char is a digit
        else if(isdigit(inputStr[cur]) != 0 && isspace(inputStr[cur]) == 0){

            while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0){

                // check if identifier contain digit, error handling
                if(isalpha(inputStr[cur]) != 0){
                    printf("\nError: Identifiers cannot begin with a digit\n");
                    flag = -1;
                    exit(1);

                    while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0)                 
                        cur++;

                    strcpy(buffer, "NO GOOD");  
                }

                // check if digit is over 5 char, error handling
                else if(bufferIdx >= NUM_MAX){
                    printf("\nError: Numbers cannot exceed 5 digits\n");
                    flag = -1;
                    exit(1);

                    while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0)                 
                        cur++;

                    strcpy(buffer, "NO GOOD");                 
                }

                else{
                    buffer[bufferIdx++] = inputStr[cur++];
                }

            }

            // error handling 
            if(strcmp(buffer, "NO GOOD") == 0){
                bufferIdx = 0;
                continue;
            }

            buffer[bufferIdx] = '\0';

            // store info into array
            strcpy(tokenArr[tokenIndex].type, "3");
            tokenArr[tokenIndex].val = atoi(buffer);
            tokenArr[tokenIndex].token = numbersym;

            tokenIndex++;
            bufferIdx = 0;
            cur--; 
        }

        
        // check if current char is a special symbol
        else if(isSpecialSymbol(inputStr[cur]) != 0){

            // +
            if(inputStr[cur] == '+'){
                strcpy(tokenArr[tokenIndex].type, "+");
                tokenArr[tokenIndex].token = plussym; 
            }

            // -
            else if(inputStr[cur] == '-'){
                strcpy(tokenArr[tokenIndex].type, "-");
                tokenArr[tokenIndex].token = minussym; 
            }

            // *
            else if(inputStr[cur] == '*'){
                strcpy(tokenArr[tokenIndex].type, "*");
                tokenArr[tokenIndex].token = multsym; 
            }

            // /
            else if(inputStr[cur] == '/'){
                strcpy(tokenArr[tokenIndex].type, "/");
                tokenArr[tokenIndex].token = slashsym; 
            }

            // (
            else if(inputStr[cur] == '('){
                strcpy(tokenArr[tokenIndex].type, "(");
                tokenArr[tokenIndex].token = lparentsym; 
            }

            // )
            else if(inputStr[cur] == ')'){
                strcpy(tokenArr[tokenIndex].type, ")");
                tokenArr[tokenIndex].token = rparentsym; 
            }

            // =
            else if(inputStr[cur] == '='){
                strcpy(tokenArr[tokenIndex].type, "=");
                tokenArr[tokenIndex].token = eqsym; 
            }

            // ,
            else if(inputStr[cur] == ','){
                strcpy(tokenArr[tokenIndex].type, ",");
                tokenArr[tokenIndex].token = commasym; 
            }

            // .
            else if(inputStr[cur] == '.'){
                strcpy(tokenArr[tokenIndex].type, ".");
                tokenArr[tokenIndex].token = periodsym; 
            }

            // <
            else if(inputStr[cur] == '<'){
                if((cur + 1 < len) && inputStr[cur + 1] == '>'){
                    strcpy(tokenArr[tokenIndex].type, "<>");
                    tokenArr[tokenIndex].token = neqsym; 
                    cur++;
                }
                else if((cur + 1 < len) && inputStr[cur + 1] == '='){
                    strcpy(tokenArr[tokenIndex].type, "<=");
                    tokenArr[tokenIndex].token = leqsym; 
                    cur++;
                }
                else{
                strcpy(tokenArr[tokenIndex].type, "<");
                tokenArr[tokenIndex].token = lessym; 
                }
            }

            // >
            else if(inputStr[cur] == '>'){
                if((cur + 1 < len) && inputStr[cur + 1] == '='){
                    strcpy(tokenArr[tokenIndex].type, ">=");
                    tokenArr[tokenIndex].token = geqsym; 
                    cur++;
                }
                else{
                strcpy(tokenArr[tokenIndex].type, ">");
                tokenArr[tokenIndex].token = gtrsym; 
                }
            }

            // ;
            else if(inputStr[cur] == ';'){
                strcpy(tokenArr[tokenIndex].type, ";");
                tokenArr[tokenIndex].token = semicolonsym; 
            }

            // :
            else if(inputStr[cur] == ':'){
                if(inputStr[cur + 1] == '='){
                    strcpy(tokenArr[tokenIndex].type, ":=");
                    tokenArr[tokenIndex].token = becomessym; 
                    cur++; 
                }
            }

            tokenIndex++;
            bufferIdx = 0;

        }
        
        // error handling, invalid symbols
        else{
            printf("\nError: Invalid Symbol\n");
            flag = -1;
            exit(1);
            cur++;
            continue;
        }

    cur++;

    }

    // ----------------------------------- HW3 -----------------------------------

    // init assemblyCode
    for(int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++){
            assemblyCode[i].op = 0;
            assemblyCode[i].l = 0;
            assemblyCode[i].m = 0;
        
    }

    // print output header
    
    fp = fopen("elf.txt", "w");

    // print output header
    printf("Assembly Code: \n");
    printf("Line\tOP\tL\tM\n");
    // call program
    program(tokenArr, fp);
    
       
    // print out assemblyCode
    
    printOut(fp);
    if(flag == 0)
        printf("\nNo errors, program is syntactically correct\n");
    fclose(fp);
    free(inputStr);
    return 0;

    

    // end of main
}

//return idx if found, otherwise return -1
int checkTable(char string[], int string_kind){
    int symbol_index = tableIndex - 1;

    //linear search through the symbol table looking at name
    while(symbol_index != 0){
        if((strcmp(symbolTable[symbol_index].name, string) == 0) && symbolTable[symbol_index].kind == string_kind){
            return symbol_index;
        }
        symbol_index--;
    }

    return 0;
}

void addTable(int kind_of_token ,char name_of_token[],int val_of_token, int level_of_token, int addr_of_token){
    
    symbolTable[tableIndex].kind = kind_of_token;
    strcpy(symbolTable[tableIndex].name,name_of_token);
    symbolTable[tableIndex].val = val_of_token;
    symbolTable[tableIndex].level = level_of_token;
    symbolTable[tableIndex].addr = addr_of_token;
    symbolTable[tableIndex].mark = 0;  //0 for all token 

    tableIndex++;
}

void program(token tokenArray[], FILE* fp){
    

    block(tokenArray, fp);

    if(strcmp(tokenArray[idx].type, ".") != 0){
        printOut(fp);
        printf("\nError: program must end with period \n");   
        exit(1);
    } 

    //emit end of program: SYS 03
    emit(9,0,3);
}

void block(token tokenArray[], FILE* fp){
    int numVars = 0;
    int procStarts = codeIndex;
    
    if(errorflag == 0){
        errorflag = 1;
        emit(7,0,0);
    }

    do {
        if (tokenArray[idx].token == constsym) {
            constDeclaration(tokenArray, fp);
        }

        else if (tokenArray[idx].token == varsym) {
            numVars = varDeclaration(tokenArray, fp);
        }

        else if (tokenArray[idx].token == procsym){
            //jmp to where this proc starts
            procDestination(tokenArray, fp);
            lexLevel--;
        }
        idx++;
    }while ((tokenArray[idx].token == constsym)||(tokenArray[idx].token == varsym)||(tokenArray[idx].token == procsym));
    
    
    assemblyCode[procStarts].m = (codeIndex) * 3;                  // The tentative jump address is fixed up
    emit(6,0,numVars + 3);
    statement(tokenArray, fp);
}

void constDeclaration(token tokenArray[], FILE* fp){
    if(tokenArray[idx].token == constsym){
        do{
            //get next token
            idx++;

            if(tokenArray[idx].token != identsym){
                printOut(fp);
                printf("\nError: const keywords must be followed by identifiers ");
                exit(1);
            }
            
            //save indent name, for putting into symbol table
            int len = strlen(tokenArray[idx].type);
            char indentName[len + 1];
            strcpy(indentName, tokenArray[idx].type);

            //get next token
            idx++;

            if(tokenArray[idx].token != eqsym){
                printOut(fp);
                printf("\nError: constants must be assigned with = ");
                exit(1);
            }

            //get next token
            idx++;

            if(tokenArray[idx].token != numbersym){
                printOut(fp);
                printf("\nError: constants must be assigned with an integer value ");
                exit(1);
            }

            if(tableIndex == 0 || checkTable(indentName,1) == 0) {
                addTable(1,indentName,tokenArray[idx].val,lexLevel,2);
            }else{
                printOut(fp);
                printf("\nError: symbol name has already been declared");
                exit(1);
            }
                
            //get the next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            printOut(fp);
            printf("\nError: Constant declaration must be followed by a semicolon ");

            errorRecovery(tokenArray);
            idx++;
        }

    }

}


//FIXED
//return numbers of variables
int varDeclaration(token tokenArray[], FILE* fp){
    /*Debug*/
    // if(strcpy(tokenArray[idx].type,"ans1") == 0){
    //     printf("\nAns1 does go to var declaration");
    // }
    // printf("\nVar declaration is %s\n",tokenArray[idx+1].type);

    int numVars = 0;
    if(tokenArray[idx].token == varsym){
        do{
            numVars++;

            //get the next token
            idx++;

            if(tokenArray[idx].token != identsym){
                printOut(fp);
                printf("\nError: var declaration must be followed by identifier");
                exit(1);
            }

            //if table is empty or it is not in the list, add it
            //otherwise it is error
            int result = 0;
            result = checkTable(tokenArray[idx].type,2);
            if(tableIndex == 0 || result == 0){ 
                /*Debug*/
                // printf("\nVar is added to the table is %s\n",tokenArray[idx].type);

                addTable(2, tokenArray[idx].type, 0,lexLevel,numVars + 2);  

                /*Debug*/
                // printf("\nAddress of the var is %d\n",symbolTable[tableIndex-1].addr);
            }else{
                if(symbolTable[result].level == lexLevel ){
                    printOut(fp);
                    printf("Symbol name has already been declared");
                    exit(1);
                }
            }

            //get next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            printOut(fp);
            printf("\nError: variable declarations must be followed by a semicolon");


            errorRecovery(tokenArray);

        }

    }
    return numVars;
}

void procDestination(token tokenArray[], FILE* fp){
    while(tokenArray[idx].token == procsym) {
        //get the next token 
        idx++;

        if(tokenArray[idx].token == identsym){
            //enter(procedure,&tx,&dx,lev); 
            addTable(3, tokenArray[idx].type, 0, lexLevel, 0);

            //get the next token 
            idx++;
        } 
        else{
            printf("Error: Procedure must be followed by an identifier");  
            exit(1);
        } 

        if(tokenArray[idx].token != semicolonsym){
            printOut(fp);
            printf("\nError: variable declarations must be followed by a semicolon");
            errorRecovery(tokenArray);
        }
        // Go to a block one lexLevel higher
        lexLevel++;
        block(tokenArray, fp); 
        
        if(tokenArray[idx].token != semicolonsym){
            printOut(fp);
            printf("\nError: variable declarations must be followed by a semicolon");
            errorRecovery(tokenArray);
        }

        //return from this function
        emit(2,0,0);


    }
}


void statement(token tokenArray[], FILE* fp){
    if(tokenArray[idx].token == identsym){
        int symIdx = 0;
        
        //check if identifier has been declared
        for(int i = tableIndex - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, symbolTable[i].name) == 0){  //found
                if(symbolTable[i].kind == 1){ //constant
                    printOut(fp);
                    printf("\nError: Identifier cannot be a constant in statement");
                    exit(1);
                }
                else if(symbolTable[i].kind == 2){ //var
                    symIdx = i;
                }
            }
        }
        
        if(symIdx == 0){
            printOut(fp);
            printf("\nError: Undeclared identifier");
            exit(1);
        }

        if(symbolTable[symIdx].kind != 2){
            printOut(fp);
            printf("\nError: Only variable values may be altered");
            exit(1); 
        }

        //get the next token
        idx++;

        if(tokenArray[idx].token != becomessym){
            printOut(fp);
            printf("\nError: Assignment statements must use :=");
            exit(1);
        }
        //get the next token
        idx++;

        expression(tokenArray, fp);


        /*Debug*/
        // printf("\nThe current char is %s\n", symbolTable[symIdx].name);
        //  printf("\nThe current address char is %d\n", symbolTable[symIdx].addr);

        //emit STO(M = table[symIdx].addr)
        emit(4,lexLevel - symbolTable[symIdx].level, symbolTable[symIdx].addr);
        //printOut();

        return;        
    }
    else if(tokenArray[idx].token == beginsym){
        do{
            //get next token
            idx++;

            statement(tokenArray, fp);
            
            if(tokenArray[idx].token != endsym && tokenArray[idx].token != semicolonsym && tokenArray[idx].token != periodsym){
                //printOut(fp);
                //printf("\nError: Semicolon expected");
                errorRecovery(tokenArray);
            }
        }while (tokenArray[idx].token == semicolonsym);

        if(tokenArray[idx].token != endsym){
            printOut(fp);
            printf("\nError: Begin must be followed by end");
            exit(1);
        }
        
        //get next token
        idx++;

        return;
    }
    else if(tokenArray[idx].token == ifsym){
        //get next token
        idx++;
        
        condition(tokenArray, fp);

        int jpc_idx = codeIndex;

        //emit JPC, OPR = 8, M = 0 for now
        emit(8, 0, 0);

        if(tokenArray[idx].token != thensym){
            printOut(fp);
            printf("\nError: If must be followed by then");
            exit(1);
        }

        //get next token
        idx++;

        statement(tokenArray, fp);
        
        //update m
        assemblyCode[jpc_idx].m = codeIndex * 3;
        
        return; 
    }
    else if(tokenArray[idx].token == whilesym){
        //get next token
        idx++;
        
        int loop_idx = codeIndex;
        condition(tokenArray, fp);

        if(tokenArray[idx].token != dosym){
            printOut(fp);
            printf("\nError: While must be followed by do");
            exit(1);
        }

        //get next token
        idx++;

        int jpc_idx = codeIndex;

        //emit JPC, OPR = 8, M = 0 
        emit(8,0,0);

        statement(tokenArray, fp);
        //emit JMP (M = loop_idx * 3), OPR = 7
        emit(7,0,loop_idx * 3);
        //update m for JPC
        assemblyCode[jpc_idx].m = codeIndex * 3 ;
        return;
    }
    else if(tokenArray[idx].token == readsym){
        //get next token
        idx++;
        if(tokenArray[idx].token != identsym){
            printOut(fp);
            printf("\nError: Read keywords must be followed by identifier");
            exit(1);
        }

        //store the index of identifier
        int symIdx = 0;

        //Check if identifier has been declared
        for(int i = tableIndex - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, symbolTable[i].name) == 0){ //found
                symIdx = i;
            }
        }

        if(symIdx == 0){
            printOut(fp);
            printf("\nError: Undeclared identifier");
            exit(1);
        }
        //if it is not a var
        if(symbolTable[symIdx].kind != 2){
            printOut(fp);
            printf("\nError: Only variable values may be altered");
            exit(1);
        }

        //get next token
        idx++;

        //emit READ, OPR = 9 SYS, READ = 2 
        emit(9,0,2);
        //printOut();
        idx++;
        //emit STO(M = table[symIdx].addr)
        emit(4,lexLevel - symbolTable[symIdx].level, symbolTable[symIdx].addr);
        //printOut();
        
        idx++;

        return;
    }
    else if(tokenArray[idx].token == writesym){
        //get next token
        idx++;

        expression(tokenArray, fp);

        //emit WRITE, OPR = 9, M = 1
        emit(9,0,1);
        //printOut();

        return;
    }
    else if(tokenArray[idx].token == xorsym){
        //get next token
        idx++;

        condition(tokenArray, fp);
        int jpcIdx = codeIndex;

        //emit JPC, OPR = 7, M = 0 for now
        emit(8,0,0);
        //printOut();

        if(tokenArray[idx].token != thensym){
            printOut(fp);
            printf("\nError: XOR must be followed by then");
            exit(1);
        }

        //get next token
        idx++;

        statement(tokenArray, fp);

        if(tokenArray[idx].token != semicolonsym){
            printOut(fp);
            //printf("\nError: Semicolon expected");
            errorRecovery(tokenArray);
            idx++;

        }

        //get next token
        idx++;

        if(tokenArray[idx].token != elsesym){
            printOut(fp);
            printf("\nError: XOR must be follow then and follow else");
            exit(1);
        }

        int jmpIdx = codeIndex;
        //emit JMP, m = 0 for now
        //we need to know know where to jump to if it is not 
        //else i.e. where is the end of else.
        emit(7,0,0);
        
        //get next token
        idx++;

        //update m
        assemblyCode[jpcIdx].m = codeIndex;
        statement(tokenArray, fp);
        assemblyCode[jmpIdx].m = codeIndex;

    }
    else if(tokenArray[idx].token == callsym){
        int symIdx = 0; 

        // get next token
        idx++;

        if(tokenArray[idx].token != identsym){
            printf("\nError: missing identifier\n");
            exit(1);
        }
        int i;
        for(i = tableIndex - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, symbolTable[i].name) == 0){
                symIdx = i;
                break;
            }
        }
        if(symIdx == -1){
            printf("\nError: undeclared identifier\n");
            exit(1); 
        }
        if(symbolTable[symIdx].kind != 3){
            printf("\nError: call must be followed by a prodecure identifier\n");
            exit(1);
        }
        emit(5, lexLevel - symbolTable[symIdx].level, symbolTable[symIdx].addr + 3);
        
        // get next token
        idx++;
    }
}

// condition procedure
void condition(token tokenArray[], FILE *fp){
    if(tokenArray[idx].token == oddsym ){    
        // get next token
        idx++;
        // call expression
        expression(tokenArray, fp);
        emit(2, 0, 11);
    }
    else{
        // call expression
        expression(tokenArray, fp);
        if(tokenArray[idx].token == eqsym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray, fp);
            emit(2, 0, 5);

        }
        else if(tokenArray[idx].token == neqsym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray, fp);
            emit(2, 0, 6);

        }
        else if(tokenArray[idx].token == lessym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray, fp);
            emit(2, 0, 7);

        }
        else if(tokenArray[idx].token == leqsym){
            // get next token
            idx++;
            // call expression
            expression(tokenArray, fp);
            emit(2, 0, 8);

        }
        else if(tokenArray[idx].token == gtrsym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray, fp);
            emit(2, 0, 9);

        }
        else if(tokenArray[idx].token == geqsym){
            // get next token
            idx++;
            // call expression
            expression(tokenArray, fp);
            emit(2, 0, 10);

        }
        else{
            printOut(fp);
            printf("\nError: condition must contain comparison operator \n");
            flag = -1;
            exit(1);
        }
    }
}

void expression(token tokenArray[], FILE* fp){
    term(tokenArray, fp);
    while(tokenArray[idx].token == plussym || tokenArray[idx].token == minussym){
        if(tokenArray[idx].token == plussym){
            //get the next token
            idx++;
            term(tokenArray, fp);
            emit(2, 0 , 1);             //emit ADD, OPR = 2, ADD = 1
            // printOut();
        }
        else{
            //get the next token
            idx++;

            term(tokenArray, fp);
            emit(2, 0, 2);              //emit SUB, OPR = 2, SUB = 2
            // printOut();
        }
    }
}

void term(token tokenArray[], FILE* fp){
    factor(tokenArray, fp);
    while(tokenArray[idx].token == multsym || tokenArray[idx].token == slashsym){
        if(tokenArray[idx].token == multsym){
            //get next token
            idx++;

            factor(tokenArray, fp);
            emit(2,0,3);                //emit MUL, OP = 2, MUL = 3
        }
        else{
            //get next token
            idx++;
            
            factor(tokenArray, fp);
            emit(2,0,4);                //emit DIV, OP = 2, DIV = 4
        }
        
    }
}


//Note: next token is updated
void factor(token tokenArray[], FILE* fp){
    if(tokenArray[idx].token == identsym){
        int symIdx = 0;
        //check for undeclared
        for(int i = tableIndex - 1; i > 0 ; i--){
            if(strcmp(tokenArray[idx].type, symbolTable[i].name) == 0){
                symIdx = i;
            }
        }
        if(symIdx == 0){
            printOut(fp);
            printf("\nError: Undeclared identifier");
            exit(1);
        }
        //var or const
        if(symbolTable[symIdx].kind == 1){         
            emit(1, 0, symbolTable[symIdx].val);
        }
        else if(symbolTable[symIdx].kind == 2){
            emit(3, lexLevel - symbolTable[symIdx].level, symbolTable[symIdx].addr);
        }
        //get next token
        idx++;
    }
    else if(tokenArray[idx].token == numbersym){
        emit(1, 0, tokenArray[idx].val);        //emit LIT
        //get the next token
        idx++;      
    }
    else if(tokenArray[idx].token == lparentsym){
        //get the next token
        idx++;

        expression(tokenArray,fp);
        
        if(tokenArray[idx].token != rparentsym){
            printOut(fp);
            printf("\nError: Right parenthesis must follow left parenthesis");
            exit(1);
        }
        //get the next token
        idx++;
    }
    else{
       printOut(fp);
       printf("\nError: Arithmetic equations must contain operands, parentheses, numbers, or symbols"); 
       exit(1);
    }
}

void errorRecovery(token tokenArray[]){
    while(tokenArray[idx].token != semicolonsym && idx <= tokenIndex){
        idx++;
    }
}

//FIXED: we do not need the register right?
void emit(int op, int l, int m){
    if(codeIndex > CODE_SIZE){
        printf("\nError: Code index exceeds code max size");
        exit(1);
    }
    else{
        assemblyCode[codeIndex].op = op;
        assemblyCode[codeIndex].l = l;
        assemblyCode[codeIndex].m = m;
        codeIndex++;
    }
}
// print the current assemblyCode line
void printOut(FILE* fp){
    
    while(lineTracker < codeIndex){
        char str[5];

        if (assemblyCode[lineTracker].op == 2){
            switch (assemblyCode[lineTracker].m){
                case 0:
                    strcpy(str, "RTN");
                    break;
                case 1:
                    strcpy(str, "ADD");
                    break;
                case 2:
                    strcpy(str, "SUB");
                    break;
                case 3:
                    strcpy(str, "MUL");
                    break;
                case 4:
                    strcpy(str, "DIV");
                    break;
                case 5:
                    strcpy(str, "EQL");
                    break;
                case 6:
                    strcpy(str, "NEQ");
                    break;
                case 7:
                    strcpy(str, "LSS");
                    break;
                case 8:
                    strcpy(str, "LEQ");
                    break;
                case 9:
                    strcpy(str, "GTR");
                    break;
                case 10:
                    strcpy(str, "GEQ");
                    break;
                case 11:
                    strcpy(str, "ODD");
                    break;
                default:
                    break;
            }  
        }
        else{
            switch (assemblyCode[lineTracker].op){
                case 1:
                    strcpy(str, "LIT");
                    break;
                case 2:
                    strcpy(str, "OPR");
                    break;
                case 3:
                    strcpy(str, "LOD");
                    break;
                case 4:
                    strcpy(str, "STO");
                    break;
                case 5:
                    strcpy(str, "CAL");
                    break;
                case 6:
                    strcpy(str, "INC");
                    break;
                case 7:
                    strcpy(str, "JMP");
                    break;
                case 8:
                    strcpy(str, "JPC");
                    break;
                case 9:
                    strcpy(str, "SYS");
                    break;
                default:
                    break;
            }  
        }

        printf("%d\t%s\t%d\t%d\n", lineTracker, str, assemblyCode[lineTracker].l, assemblyCode[lineTracker].m);
        fprintf(fp, "%d\t%d\t%d\n", assemblyCode[lineTracker].op, assemblyCode[lineTracker].l, assemblyCode[lineTracker].m);

        lineTracker++;
    }
}



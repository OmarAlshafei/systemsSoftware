// Omar Alshafei and Hung Tran
// 7-9-23
// HW3 Tiny Compiler


// pre-processor directives
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


// constants
#define IDENT_MAX 11
#define NUM_MAX 5
#define MAX_SYMBOL_TABLE_SIZE 500


// symbol table struct 
typedef struct {
    int kind;           // const = 1, var = 2, proc = 3
    char name[10];      // name up to 11 chars
    int val;            // number (ASCII value)
    int level;          // L level
    int addr;           // M address
    int mark;           // to indicate unavailable or deleted
}symbol;

// token struct 
typedef struct token {
    int token;   
    int  val;   
    char type[15];  
}token;

// instruction struct
typedef struct{
    int op;             // op code
    int l;              // level
    int m;              // constant or address
}instruction;


// global variables
instruction assembly[MAX_SYMBOL_TABLE_SIZE];
symbol table[MAX_SYMBOL_TABLE_SIZE]; 
int codeIndex = 0;      // index of assembly
int idx = 0;            // index of token array
int table_index = 1;    // index of symbol table
int tokenIdx = 0;       // current index of tokenArr


// token values
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, xorsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym
} token_type;

void printThis();
int isSpecialSymbol(char ch);
int checkTable(char string[], int string_kind);
void program(token tokenArray[], int size);
void block(token tokenArray[], int size);
void constDeclaration(token tokenArray[]);
int varDeclaration(token tokenArray[]);
void statement(token tokenArray[]);
void condition(token tokenArray[]);
void expression(token tokenArray[]);
void term(token tokenArray[]);
void factor(token tokenArray[]);
void addTable(int, char*, int, int, int);
void emit(int op, int l, int m);
void errorRecovery(token tokenArray[]);


// return 1 if char is a special symbols, else return 0
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
    char reservedWords[14][20] = {"const", "var", "odd", "call", "begin", "end", "if", "then",
                                  "xor", "else", "while", "do", "read", "write"};
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
                    printf("Error: Identifier names cannot exceed 11 characters\n");
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
                strcpy(tokenArr[tokenIdx].type, "const");
                tokenArr[tokenIdx].token = constsym; 
            }

            // var
            else if(strcmp(buffer, reservedWords[1]) == 0){
                strcpy(tokenArr[tokenIdx].type, "var");
                tokenArr[tokenIdx].token = varsym; 
            }

            // odd
            else if(strcmp(buffer, reservedWords[2]) == 0){
                strcpy(tokenArr[tokenIdx].type, "odd");
                tokenArr[tokenIdx].token = oddsym; 
            }

            // call
            // else if(strcmp(buffer, reservedWords[3]) == 0){
            //     strcpy(tokenArr[tokenIdx].type, "call");
            //     tokenArr[tokenIdx].token = callsym; 
            // }

            // begin
            else if(strcmp(buffer, reservedWords[4]) == 0){
                strcpy(tokenArr[tokenIdx].type, "begin");
                tokenArr[tokenIdx].token = beginsym; 
            }

            // end
            else if(strcmp(buffer, reservedWords[5]) == 0){
                strcpy(tokenArr[tokenIdx].type, "end");
                tokenArr[tokenIdx].token = endsym; 
            }

            // if
            else if(strcmp(buffer, reservedWords[6]) == 0){
                strcpy(tokenArr[tokenIdx].type, "if");
                tokenArr[tokenIdx].token = ifsym; 
            }

            // then
            else if(strcmp(buffer, reservedWords[7]) == 0){
                strcpy(tokenArr[tokenIdx].type, "then");
                tokenArr[tokenIdx].token = thensym; 
            }

            // xor
            else if(strcmp(buffer, reservedWords[8]) == 0){
                strcpy(tokenArr[tokenIdx].type, "xor");
                tokenArr[tokenIdx].token = xorsym; 
            }

            // else
            else if(strcmp(buffer, reservedWords[9]) == 0){
                strcpy(tokenArr[tokenIdx].type, "else");
                tokenArr[tokenIdx].token = elsesym; 
            }

            // while
            else if(strcmp(buffer, reservedWords[10]) == 0){
                strcpy(tokenArr[tokenIdx].type, "while");
                tokenArr[tokenIdx].token = whilesym; 
            }

            // do
            else if(strcmp(buffer, reservedWords[11]) == 0){
                strcpy(tokenArr[tokenIdx].type, "do");
                tokenArr[tokenIdx].token = dosym; 
            }

            // read
            else if(strcmp(buffer, reservedWords[12]) == 0){
                strcpy(tokenArr[tokenIdx].type, "read");
                tokenArr[tokenIdx].token = readsym; 
            }

            // write
            else if(strcmp(buffer, reservedWords[13]) == 0){
                strcpy(tokenArr[tokenIdx].type, "write");
                tokenArr[tokenIdx].token = writesym; 
            }

            // Error handling
            else if(strcmp(buffer, "NO GOOD") == 0){
                bufferIdx = 0;
                continue;
            }

            // identifier 
            else{
                strcpy(tokenArr[tokenIdx].type, buffer);
                tokenArr[tokenIdx].token = identsym; 
            }
                     
            tokenIdx++;
            bufferIdx = 0;
            cur--;
        }

        // check if current char is a digit
        else if(isdigit(inputStr[cur]) != 0 && isspace(inputStr[cur]) == 0){

            while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0){

                // check if identifier contain digit, error handling
                if(isalpha(inputStr[cur]) != 0){
                    printf("Error: Identifiers cannot begin with a digit\n");
                    exit(1);

                    while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0)                 
                        cur++;

                    strcpy(buffer, "NO GOOD");  
                }

                // check if digit is over 5 char, error handling
                else if(bufferIdx >= NUM_MAX){
                    printf("Error: Numbers cannot exceed 5 digits\n");
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
            strcpy(tokenArr[tokenIdx].type, "3");
            tokenArr[tokenIdx].val = atoi(buffer);
            tokenArr[tokenIdx].token = numbersym;

            tokenIdx++;
            bufferIdx = 0;
            cur--; 
        }

        
        // check if current char is a special symbol
        else if(isSpecialSymbol(inputStr[cur]) != 0){

            // +
            if(inputStr[cur] == '+'){
                strcpy(tokenArr[tokenIdx].type, "+");
                tokenArr[tokenIdx].token = plussym; 
            }

            // -
            else if(inputStr[cur] == '-'){
                strcpy(tokenArr[tokenIdx].type, "-");
                tokenArr[tokenIdx].token = minussym; 
            }

            // *
            else if(inputStr[cur] == '*'){
                strcpy(tokenArr[tokenIdx].type, "*");
                tokenArr[tokenIdx].token = multsym; 
            }

            // /
            else if(inputStr[cur] == '/'){
                strcpy(tokenArr[tokenIdx].type, "/");
                tokenArr[tokenIdx].token = slashsym; 
            }

            // (
            else if(inputStr[cur] == '('){
                strcpy(tokenArr[tokenIdx].type, "(");
                tokenArr[tokenIdx].token = lparentsym; 
            }

            // )
            else if(inputStr[cur] == ')'){
                strcpy(tokenArr[tokenIdx].type, ")");
                tokenArr[tokenIdx].token = rparentsym; 
            }

            // =
            else if(inputStr[cur] == '='){
                strcpy(tokenArr[tokenIdx].type, "=");
                tokenArr[tokenIdx].token = eqsym; 
            }

            // ,
            else if(inputStr[cur] == ','){
                strcpy(tokenArr[tokenIdx].type, ",");
                tokenArr[tokenIdx].token = commasym; 
            }

            // .
            else if(inputStr[cur] == '.'){
                strcpy(tokenArr[tokenIdx].type, ".");
                tokenArr[tokenIdx].token = periodsym; 
            }

            // <
            else if(inputStr[cur] == '<'){
                if((cur + 1 < len) && inputStr[cur + 1] == '>'){
                    strcpy(tokenArr[tokenIdx].type, "<>");
                    tokenArr[tokenIdx].token = neqsym; 
                    cur++;
                }
                else if((cur + 1 < len) && inputStr[cur + 1] == '='){
                    strcpy(tokenArr[tokenIdx].type, "<=");
                    tokenArr[tokenIdx].token = leqsym; 
                    cur++;
                }
                else{
                strcpy(tokenArr[tokenIdx].type, "<");
                tokenArr[tokenIdx].token = lessym; 
                }
            }

            // >
            else if(inputStr[cur] == '>'){
                if((cur + 1 < len) && inputStr[cur + 1] == '='){
                    strcpy(tokenArr[tokenIdx].type, ">=");
                    tokenArr[tokenIdx].token = geqsym; 
                    cur++;
                }
                else{
                strcpy(tokenArr[tokenIdx].type, ">");
                tokenArr[tokenIdx].token = gtrsym; 
                }
            }

            // ;
            else if(inputStr[cur] == ';'){
                strcpy(tokenArr[tokenIdx].type, ";");
                tokenArr[tokenIdx].token = semicolonsym; 
            }

            // :
            else if(inputStr[cur] == ':'){
                if(inputStr[cur + 1] == '='){
                    strcpy(tokenArr[tokenIdx].type, ":=");
                    tokenArr[tokenIdx].token = becomessym; 
                    cur++; 
                }
            }

            tokenIdx++;
            bufferIdx = 0;

        }
        
        // error handling, invalid symbols
        else{
            printf("Error: Invalid Symbol\n");
            exit(1);
            cur++;
            continue;
        }

    cur++;

    }

    // ----------------------------------- HW3 -----------------------------------

    // init assembly
    for(int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++){
        if(i == 0){
            assembly[i].op = 7;
            assembly[i].l = 0;
            assembly[i].m = 3;
            codeIndex++;
        }
        else{
            assembly[i].op = 0;
            assembly[i].l = 0;
            assembly[i].m = 0;
        }
    }

    // print assembly
    printf("\nAssembly Code: \n");
    printf("Line\tOP\tL\tM\n");



    // call program
    program(tokenArr, tokenIdx);

    printf("\n");
    fclose(fp);
    free(inputStr);
    return 0;

    // end of main
}


// program procedure
void program(token tokenArray[], int size){
    
    block(tokenArray, size);

    if(strcmp(tokenArray[idx-1].type, ".") != 0){
            printThis();
        printf("\nError: program must end with period \n");   
        exit(1);
    } 

    // HALT
    emit(9, 0, 3);
}


// block procedure
void block(token tokenArray[], int size){
    while(idx < size){
        if(tokenArray[idx].token == constsym){
            // call constDeclaration
            constDeclaration(tokenArray);
        }
        else if(tokenArray[idx].token == varsym){
            // call varDeclaration
            int numVars = varDeclaration(tokenArray);
            emit(6, 0, numVars + 3);

        }
        else{
            // call statement
            statement(tokenArray);
        }
        // get next token
        idx++;
    }
}


// constDeclaration procedure
void constDeclaration(token tokenArray[]){
    if(tokenArray[idx].token == constsym){
        do{
            // get next token
            idx++;
            if(tokenArray[idx].token != identsym){
            printThis();
                printf("\nError: const keywords must be followed by identifiers \n");
                exit(1);
            }
            
            // save ident name
            int len = strlen(tokenArray[idx].type);
            char indentName[len + 1];
            strcpy(indentName, tokenArray[idx].type);

            // something weird here 
            // if(checkTable(indentName, 1) == -1){
            //     printf("\nError: undeclared constant \n");
            //     exit(1);
            // }

            // get next token
            idx++;
            if(tokenArray[idx].token !=eqsym){
            printThis();
                printf("Error: constants must be assigned with = \n");
                exit(1);
            }

            // get next token
            idx++;
            if(tokenArray[idx].token != numbersym){
            printThis();
                printf("\nError: constants must be assigned with an integer value \n");
                exit(1);
            }

            // check if already declared
            if(table_index == 0 || checkTable(indentName,1) == -1) {
                addTable(1, indentName, tokenArray[idx].val, 0, 0);
            }else{
                printThis();
                printf("\nError: symbol name has already been declared \n");
                exit(1);
            }
                
            // get the next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            printThis();
            printf("\nError: constant declaration must be followed by a semicolon \n");
            errorRecovery(tokenArray);
        }
    }
}


// varDeclaration procedure
int varDeclaration(token tokenArray[]){
    int numVars = 0;
    if(tokenArray[idx].token == varsym){
        do{
            numVars++;

            // get the next token
            idx++;
            if(tokenArray[idx].token != identsym){
            printThis();

                printf("\nError: var keywords must be followed by identifier \n");
                exit(1);
            }

            // check if already declared
            if(table_index == 0 || checkTable(tokenArray[idx].type,2) == -1){ 
                addTable(2, tokenArray[idx].type, 0, 0, numVars + 2);  
            }else{
            printThis();
                printf("\nError: symbol name has already been declared \n");
                exit(1);
            }

            // get next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            printThis();
            printf("\nError: variable declarations must be followed by a semicolon \n");
            errorRecovery(tokenArray);
        }
    }
    return numVars;
}


// statement procedure
void statement(token tokenArray[]){

   if(tokenArray[idx].token == identsym){

        int symIdx = -1;
        
        // check table
        for(int i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, table[i].name) == 0){
                if(table[i].kind == 1){
                    printThis();
                    printf("\nError: only variable values may be altered \n");
                    exit(1);
                }
                else if(table[i].kind == 2){
                    symIdx = i;
                }
            }
        }
        
        if (symIdx == -1){
            printThis();
            printf("\nError : undeclared symbol \n");
            exit(1); 
        }
        
        if(table[symIdx].kind != 2){
            printThis();
           printf("\nError: only variable values may be altered \n");
           exit(1); 
        }

        // get the next token
        idx++;

        if(tokenArray[idx].token != becomessym){
            printThis();
            printf("\nError: assignment statements must use := \n");
            exit(1);
        }
        // get the next token
        idx++;
        // call espression
        expression(tokenArray);
        emit(4, 0, table[symIdx].addr);

        return;        
    }
    else if(tokenArray[idx].token == beginsym){
        do{
            // get next token
            idx++;
            // call statement
            statement(tokenArray);

            // FIXME - bug here
            if(tokenArray[idx].token != semicolonsym){
                printf("\nError: expected a semicolon HERE \n");
                errorRecovery(tokenArray);
            }

        }while (tokenArray[idx].token == semicolonsym);
        
        if(tokenArray[idx].token != endsym){
            printThis();
            printf("\nError: begin must be followed by end \n");
            exit(1);
        }
        
        // get next token
        idx++;
        return;
    }
    else if(tokenArray[idx].token == ifsym){
        // get next token
        idx++;
        // call condition
        condition(tokenArray);

        int jpc_idx = codeIndex;
        emit(8, 0, jpc_idx * 3);
        if(tokenArray[idx].token != thensym){
            printThis();
            printf("\nError: if must be followed by then \n");
            exit(1);
        }

        // get next token
        idx++;
        // call statement
        statement(tokenArray);
        // JPC - update m
        assembly[jpc_idx].m = codeIndex * 3;
        return; 
    }
    else if(tokenArray[idx].token == xorsym){
        // get next token
        idx++;
        // call conditinon
        condition(tokenArray);
        int jpcIdx = codeIndex;

        emit(8, 0, jpcIdx * 3);
        if(tokenArray[idx].token != thensym){
            printThis();
            printf("\nError: XOR must be followed by then \n");
            exit(1);
        }

        // get next token
        idx++;
        // call statement
        statement(tokenArray);

        if(tokenArray[idx].token != semicolonsym){
            printThis();
            printf("\nError: expected a semicolon \n");
            errorRecovery(tokenArray);
        }

        // get next token
        else 
            idx++;
            
        if(tokenArray[idx].token != elsesym){
            printf("Error: XOR must be follow then and follow else\n");
            exit(1);
        }
        assembly[jpcIdx].m = (codeIndex + 1) * 3; // FIXME
        jpcIdx = codeIndex;
        emit(7, 0, codeIndex * 3);

        // get next token
        idx++;
        // call statement
        assembly[jpcIdx].m = codeIndex * 3;
        statement(tokenArray);

        // JMP? - update m
        assembly[jpcIdx].m = codeIndex * 3;        
    }
    else if(tokenArray[idx].token == whilesym){
        // get next token
        idx++;
        
        int loop_idx = codeIndex;
        // call condtion
        condition(tokenArray);

        if(tokenArray[idx].token != dosym){
            printThis();
            printf("\nError: while must be followed by do \n");
            exit(1);
        }

        // get next token
        idx++;
        int jpc_idx = codeIndex;
        emit(8, 0, 0);
        // call statement
        statement(tokenArray);
        emit(7, 0, loop_idx * 3);

        // JPC - update m
        assembly[jpc_idx].m = codeIndex * 3;
        return;
    }
    else if(tokenArray[idx].token == readsym){
        // get next token
        idx++;

        if(tokenArray[idx].token != identsym){
            printThis();
            printf("\nError: read keywords must be followed by identifier \n");
            exit(1);
        }

        // table index
        int symIdx = -1;

        // check if already declared
        for(int i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, table[i].name) == 0){ 
                symIdx = i;
                break;
            }
        }

        if(symIdx == -1){
            printThis();
            printf("\nError: undeclared identifier \n");
            exit(1);
        }
        if(table[symIdx].kind != 2){
            printThis();
            printf("\nError: only variable values may be altered \n");
            exit(1);
        }

        // get next token
        idx++;

        emit(9, 0, 2);
        emit(4, 0, table[symIdx].addr);

        return;
    }
    else if(tokenArray[idx].token == writesym){
        // get next token
        idx++;
        // call expression
        expression(tokenArray);
        emit(9, 0, 1);
        return;
    }

}


// condition procedure
void condition(token tokenArray[]){
    if(tokenArray[idx].token == oddsym ){    
        // get next token
        idx++;
        // call expression
        expression(tokenArray);
        emit(2, 0, 11);
    }
    else{
        // call expression
        expression(tokenArray);
        if(tokenArray[idx].token == eqsym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray);
            emit(2, 0, 5);

        }
        else if(tokenArray[idx].token == neqsym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray);
            emit(2, 0, 6);

        }
        else if(tokenArray[idx].token == lessym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray);
            emit(2, 0, 7);

        }
        else if(tokenArray[idx].token == leqsym){
            // get next token
            idx++;
            // call expression
            expression(tokenArray);
            emit(2, 0, 8);

        }
        else if(tokenArray[idx].token == gtrsym){ 
            // get next token
            idx++;
            // call expression
            expression(tokenArray);
            emit(2, 0, 9);

        }
        else if(tokenArray[idx].token == geqsym){
            // get next token
            idx++;
            // call expression
            expression(tokenArray);
            emit(2, 0, 10);

        }
        else{
            printThis();
            printf("\nError: condition must contain comparison operator \n");
            exit(1);
        }
    }
}


// expression procedure
void expression(token tokenArray[]){
    // call term
    term(tokenArray);
    while(tokenArray[idx].token == plussym || tokenArray[idx].token == minussym){
        if(tokenArray[idx].token == plussym){
            // get the next token
            idx++;
            // call term
            term(tokenArray);
            emit(2, 0 , 1);

        }
        else{
            idx++;
            // call term
            term(tokenArray);
            emit(2, 0, 2);

        }
    }
}


// term procedure
void term(token tokenArray[]){
    // call factor
    factor(tokenArray);
    
    while(tokenArray[idx].token == multsym || tokenArray[idx].token == slashsym){
        if(tokenArray[idx].token == multsym){
            // get next token
            idx++;
            // call factor
            factor(tokenArray);
            emit(2, 0, 3); 

        }
        else{
            // get next token
            idx++;
            // call factor
            factor(tokenArray);
            emit(2, 0, 4); 

        }
    }
}


// factor procedure
void factor(token tokenArray[]){
    if(tokenArray[idx].token == identsym){
        // table index
        int symIdx = -1;

        // check if not declared
        for(int i = table_index - 1; i > 0 ; i--){
            if(strcmp(tokenArray[idx].type, table[i].name) == 0){
                symIdx = i;
            }
        }

        if(symIdx == -1){
            printThis();
            printf("\nError: undeclared identifier \n");
            exit(1);
        }

        // check kind
        if(table[symIdx].kind == 1){         
            emit(1, 0, table[symIdx].val); 

        }
        else if(table[symIdx].kind == 2){
            emit(3, 0, table[symIdx].addr);

        }

        // get next token
        idx++;
    }
    else if(tokenArray[idx].token == numbersym){

        emit(1, 0, tokenArray[idx].val);
        // get the next token
        idx++;      
    }
    else if(tokenArray[idx].token == lparentsym){
        // get the next token
        idx++;
        // call expression
        expression(tokenArray);
        
        if(tokenArray[idx].token != rparentsym){
            printThis();
            printf("\nError: right parenthesis must follow left parenthesis \n");
            exit(1);
        }
        // get the next token
        idx++;
    }
    else{
        printThis();
        printf("\nError: arithmetic equations must contain operands, parentheses, numbers, or symbols \n"); 
        exit(1);
    }
}


// emit op, l, and m into assembly at given index
void emit(int op, int l, int m){
    if(codeIndex > MAX_SYMBOL_TABLE_SIZE){
        printf("Error: Code index exceeds code max size");
        exit(1);
    }
    else{
        assembly[codeIndex].op = op;
        assembly[codeIndex].l = l;
        assembly[codeIndex].m = m;
        codeIndex++;
    }
}


// add values to symbol table
void addTable(int kind ,char name[],int val, int level, int addr){
    table[table_index].kind = kind;
    strcpy(table[table_index].name, name);
    table[table_index].val = val;
    table[table_index].level = level;
    table[table_index].addr = addr;
    table[table_index++].mark = 0;  // no procedure implemention
}


// return the index if found, else return -1
int checkTable(char string[], int string_kind){
    int index = table_index - 1;

    // searching
    while(index != 0){
        if((strcmp(table[index].name, string) == 0) && table[index].kind == string_kind){
            return index;
        }
        index--;
    }
    return -1;
}


// skip tokens until a “;” is found 
// return the next token
void errorRecovery(token tokenArray[]){
    while(tokenArray[idx].token != semicolonsym && idx <= tokenIdx){
        idx++;
    }
}


// print the current assembly line
void printThis(){
    for(int i = 0; i < codeIndex; i++){
        char str[5];

        if (assembly[i].op == 2){
            switch (assembly[i].m){
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
            switch (assembly[i].op){
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

        printf("%d\t%s\t%d\t%d \n", i, str, assembly[i].l, assembly[i].m);
    }
}
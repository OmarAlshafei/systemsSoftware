// Hung Tran and Omar Alshafei
// COP-3402 Summer 2023
// HW2 Lexical Analyzer


// pre-processor directives
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// constants
#define LIT  1         // Literal
#define OPR  2         // Operation
#define LOD  3         // Load
#define STO  4         // Store
#define CAL  5         // Call prodecure
#define INC  6         // Increment
#define JMP  7         // Jump
#define JPC  8         // Jump Condition
#define SYS  9         // Start Input Output

#define RTN  0         // Return
#define ADD  1         // Add
#define SUB  2         // Subtract
#define MUL  3         // Multiply
#define DIV  4         // Divide
#define EQL  5         // Equal
#define NEQ  6         // Not Equal
#define LSS  7        // Less Th$an
#define LEQ  8        // Less Than or Equal 
#define GTR  9        // Greater Than 
#define GEQ  10        // Greater Than or Equal
#define ODD  11         // ODD

//SYS
#define WRITE 1 
#define READ  2
#define END   3

typedef struct {
    int kind;           //const = 1, var = 2, proc = 3
    char name[10];      //name up to 11
    int val;            //number(ASCII value)
    int level;          //L level
    int addr;           //M address
    int mark;           //to indicate unavailable or deleted
}symbol;

// token struct 
typedef struct token {
    int token;   
    int  val;   
    char type[15];  
}token;


typedef struct{
    int op;             //Operation code
    int l;              // Lexicographical level
    int m;              //Modifier/Argument (constant, address, or OPR)
}instruction;

#define IDENT_MAX 11
#define NUM_MAX 5
#define MAX_SYMBOL_TABLE_SIZE 500
instruction assembly[MAX_SYMBOL_TABLE_SIZE];
symbol table[MAX_SYMBOL_TABLE_SIZE]; 
int codeIndex = 0;                          //current index of code stack
int idx = 0;                                //current index of token array
int table_index = 1;                        //current index of symbol table

// token values
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, xorsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym
} token_type;



void print(char *source,int len);
int isSpecialSymbol(char ch);
void addTable(int, char*, int, int, int);
int checkTable(char string[], int string_kind);
void emit(int op,  int l, int m);
void constDeclaration(token tokenArray[]);
int varDeclaration(token tokenArray[]);
void block(token tokenArray[], int size);
void term(token tokenArray[]);
void expression(token tokenArray[]);
void factor(token tokenArray[]);
void program(token tokenArray[], int size);
void condition(token tokenArray[]);
void statement(token tokenArray[]);
void printAssembly();
void OpCode(char*, int);

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
        
        exit(0);
    }

    // local variables 
    char reservedWords[14][20] = {"const", "var", "procedure", "call", "begin", "end", "if", "then",
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
    int tokenIdx = 0;           // current index of tokenArr
    

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
            while(inputStr[cur] != '*' && inputStr[cur + 1] != '/'){
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

            // procedure
            else if(strcmp(buffer, reservedWords[2]) == 0){
                strcpy(tokenArr[tokenIdx].type, "procedure");
                tokenArr[tokenIdx].token = procsym; 
            }

            // call
            else if(strcmp(buffer, reservedWords[3]) == 0){
                strcpy(tokenArr[tokenIdx].type, "call");
                tokenArr[tokenIdx].token = callsym; 
            }

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

                    while((isalpha(inputStr[cur]) != 0 || isdigit(inputStr[cur]) != 0) && isspace(inputStr[cur]) == 0)                 
                        cur++;

                    strcpy(buffer, "NO GOOD");  
                }

                // check if digit is over 5 char, error handling
                else if(bufferIdx >= NUM_MAX){
                    printf("Error: Numbers cannot exceed 5 digits\n");

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
            cur++;
            continue;
        }

    cur++;

    }

    int size = 0;
    token tokenArray[len];
    while((size) < tokenIdx){
        tokenArray[(size)].token = tokenArr[(size)].token;
        tokenArray[(size)].val = tokenArr[(size)].val;
        strcpy(tokenArray[(size)].type, tokenArr[(size)].type);

        (size)++;
    }
    /*Parser and code generator*/
    //Initilze code stack 
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

    program(tokenArray, size);
    
    printAssembly();
    
    printf("\n");
    fclose(fp);
    free(inputStr);

}


void program(token tokenArray[], int size){
    
    block(tokenArray, size);

    //emit end of program: SYS 03
    emit(9,0,3);

    
}

void block(token tokenArray[], int size){
    while(idx < size){
        if(tokenArray[idx].token == constsym){
            constDeclaration(tokenArray);
        }
        else if(tokenArray[idx].token == varsym){
            int numVars = varDeclaration(tokenArray);
            //emit INC(M = 3 + numVars), OPR = 6
            emit(6,0,numVars + 3);

        }
        else{
            statement(tokenArray);
        }
        idx++;
    }
    
}

int varDeclaration(token tokenArray[]){
    int numVars = 0;
    if(tokenArray[idx].token == varsym){
        do{
            numVars++;

            //get the next token
            idx++;
            if(tokenArray[idx].token != identsym){
                printf("Error: var keywords must be followed by identifier");
                exit(1);
            }

            //if table is empty or it is not in the list, add it
            //otherwise it is error
            if(table_index == 0 || checkTable(tokenArray[idx].type,2) == 0){ 
                addTable(2, tokenArray[idx].type, 0,0,numVars + 2);  
            }else{
                printf("Symbol name has already been declared");
                exit(1);
            }

            //get next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            printf("Error: variable declarations must be followed by a semicolon");
            exit(1);
        }

        //get the next token 
        // idx++;
    }
    return numVars;
}


void statement(token tokenArray[]){
    if(tokenArray[idx].token == identsym){

        int symIdx = 0;
        
        //check if identifier has been declared
        for(int i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, table[i].name) == 0){  //found
                if(table[i].kind == 1){ //constant
                    
                    /*Debugging*/
                    printf("\nBug is at location :b");
                    for(int j = idx; j >= 0; j--){
                        printf("\nCurrent index of token array is %d\n", j);
                        printf("Current symbol is %s\n", tokenArray[j].type);
                    }
                    printf("\nCurrent index of token array is %d\n", idx);
                    printf("Current symbol is %s\n", table[i].name);


                    printf("\nError: Symbol cannot be a constant in statment");
                    exit(1);
                }
                else if(table[i].kind == 2){ //var
                    symIdx = i;
                }
            }
        }
        

        if(table[symIdx].kind != 2){
           printf("\nError: Only variable values may be altered");
           exit(1); 
        }

        //get the next token
        idx++;

        if(tokenArray[idx].token != becomessym){
            printf("\nError: Assignment statements must use :=");
            exit(1);
        }
        //get the next token
        idx++;
        
        expression(tokenArray);

        //emit STO(M = table[symIdx].addr)
        emit(4,0,table[symIdx].addr);

        return;        
    }
    else if(tokenArray[idx].token == beginsym){
        do{
            
            //get next token
            idx++;

            statement(tokenArray);
            
        }while (tokenArray[idx].token == semicolonsym);
        
        if(tokenArray[idx].token != endsym){
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
        condition(tokenArray);

        int jpc_idx = codeIndex;

        //emit JPC, OPR = 7, M = 0 for now
        //FIXME:make sure m is updated
        emit(7, 0, 0);

        if(tokenArray[idx].token != thensym){
            printf("\nError: If must be followed by then");
            exit(1);
        }

        //get next token
        idx++;

        statement(tokenArray);
        
        //update m
        assembly[jpc_idx].m = codeIndex;
        return; 
    }
    else if(tokenArray[idx].token == whilesym){
        
        //get next token
        idx++;
        
        int loop_idx = codeIndex;
        condition(tokenArray);

        if(tokenArray[idx].token != dosym){
            printf("\nError: While must be followed by do");
            exit(1);
        }

        //get next token
        idx++;
        int jpc_idx = codeIndex;

        //emit JPC, OPR = 7, M = 0 for now??
        //FIXME: do we update at line 1058
        emit(8,0,0);

        statement(tokenArray);

        //emit JMP (M = loop_idx), OPR = 7
        emit(7,0,loop_idx);

        //update m for JPC
        assembly[jpc_idx].m = codeIndex;
        return;
    }
    else if(tokenArray[idx].token == readsym){
        //get next token
        idx++;

        if(tokenArray[idx].token != identsym){
            printf("\nError: Read keywords must be followed by identifier");
            exit(1);
        }

        //store the index of identifier
        int symIdx = 0;

        //Check if identifier is has been declared
        for(int i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, table[i].name) == 0){ //found
                symIdx = i;
            }
        }

        if(symIdx == 0){
            printf("\nError: Undeclared identifier");
            exit(1);
        }
        //if it is not a var
        if(table[symIdx].kind != 2){
            printf("\nError: Only variable values may be altered");
            exit(1);
        }

        //get next token
        idx++;

        //emit READ, OPR = 9 SYS, READ = 2 
        emit(9,0,2);

        //emit STO(M = table[symIdx].addr)
        emit(4,0,table[symIdx].addr);

        return;
    }
    else if(tokenArray[idx].token == writesym){
        //get next token
        idx++;
        expression(tokenArray);
        //emit WRITE, OPR = 9, M = 1
        emit(9,0,1);
        return;
    }
    else if(tokenArray[idx].token == xorsym){
        
        //get next token
        idx++;

        condition(tokenArray);
        int jpcIdx = codeIndex;

        emit(7,0,0);
        if(tokenArray[idx].token != thensym){
            printf("\nError: XOR must be followed by then");
            exit(1);
        }

        //get next token
        idx++;

        statement(tokenArray);

        //FIXME: what we should print for error
        if(tokenArray[idx].token != semicolonsym){
            printf("\nError: Missing a semicolon");
            exit(1);
        }

        //get next token
        idx++;
        if(tokenArray[idx].token != elsesym){
            printf("\nError: XOR must be follow then and follow else");
            exit(1);
        }

        //get next token
        idx++;
        statement(tokenArray);

        //update m
        assembly[jpcIdx].m = codeIndex;

    }

}

//NOTE: also need to take care of lexical level, has problem if value is 3.14
void constDeclaration(token tokenArray[]){
    if(tokenArray[idx].token == constsym){
        do{
            //get next token
            idx++;
            if(tokenArray[idx].token != identsym){
                printf("Error: const keywords must be followed by identifiers ");
                exit(1);
            }
            //if (checkTable(tokenArray[idx)) != -1)   //NOTE: ASK PROF HERE
                //printf('Error: ');
                //exit(0);
            
            //save indent name, for putting into symbol table
            int len = strlen(tokenArray[idx].type);
            char indentName[len + 1];
            strcpy(indentName, tokenArray[idx].type);


            //get next token
            idx++;
            if(tokenArray[idx].token != becomessym){
                printf("Error: constants must be assigned with = ");
                exit(1);
            }

            //get next token
            idx++;
            if(tokenArray[idx].token != numbersym){
                printf("Error: constants must be assigned with an integer value ");
                exit(1);
            }

            //if table is empty or it is not in the list, add it. Otherwise if it is in the table, return false
            if(table_index == 0 || checkTable(indentName,1) == 0) {
                addTable(1,indentName,tokenArray[idx].val,0,0);
            }else{
                printf("Error: symbol name has already been declared");
                exit(1);
            }
                
            //get the next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            printf("Error: constant declaration must be followed by a semicolon ");
            exit(1);
        }

        //get next token
        // idx++;

    }

}

void expression(token tokenArray[]){
    term(tokenArray);
    while(tokenArray[idx].token == plussym || tokenArray[idx].token == minussym){
        if(tokenArray[idx].token == plussym){
            //get the next token
            idx++;
            term(tokenArray);
            //emit ADD, OPR = 2, ADD = 1
            emit(2, 0 , 1);
        }
        else{

            idx++;
            /*Debug*/
            // printf("\nCurrent token at expression at minussym: %s\n", tokenArray[idx].type);

            term(tokenArray);
            //emit SUB, OPR = 2, SUB = 2
            emit(2, 0, 2);
        }
    }
}


void condition(token tokenArray[]){
    if(tokenArray[idx].token == oddsym ){    
        /*Debug*/
        // printf("\nCurrent token at condition at oddsym %s\n", tokenArray[idx].type);


        //get next token
        idx++;
        expression(tokenArray);
        //emit ODD, OPR = 2, ODD = 11
        emit(2,0,11);
    }
    else{
        expression(tokenArray);
        //Note: there maybe a bug here due to idx tracking
        if(tokenArray[idx].token == eqsym){                    // =
            //get next token
            idx++;
            expression(tokenArray);
            //emit EQL, OPR = 2, EQL = 6
            emit(2,0,6);
        }
        else if(tokenArray[idx].token == neqsym){               // <>
            //get next token
            idx++;
            expression(tokenArray);
            //emit NQL, OPR = 2, NQL = 7
            emit(2,0,7);
        }
        else if(tokenArray[idx].token == lessym){               // <
            //get next token
            idx++;
            expression(tokenArray);
            //emit LSS, OPR = 2, LSS = 8
            emit(2,0,8);
        }
        else if(tokenArray[idx].token == leqsym){               // <= (NOTE: ASK ABOUT THIS <= OR =<)
            //get next token
            idx++;
            expression(tokenArray);
            //emit LEQ, OPR = 2, LEQ = 9
            emit(2,0,9);

        }
        else if(tokenArray[idx].token == gtrsym){               // >
            //get next token
            idx++;
            expression(tokenArray);
            //emit GTR, OPR = 2, GTR = 10
            emit(2,0,10);
        }
        else if(tokenArray[idx].token == geqsym){               // >=
            //get next token
            idx++;
            expression(tokenArray);
            //emit GEQ, OPR = 2, GEQ = 11
            emit(2,0,11);
        }
        else{
            printf("\nError:Condition must contain comparison operator");
            exit(1);
        }
    }

}

void factor(token tokenArray[]){
    if(tokenArray[idx].token == identsym){
        /*Debug*/
        // printf("\nCurrent token at factor at identisym: %s\n", tokenArray[idx].type);
        
        int symIdx = 0;

        //check for undeclared
        for(int i = table_index - 1; i > 0 ; i--){
            if(strcmp(tokenArray[idx].type, table[i].name) == 0){
                symIdx = i;
            }
        }

        if(symIdx == 0){
            // printf("Bug is location: a");
            printf("\nError: Undeclared identifier");
            exit(1);
        }

        //var or const
        if(table[symIdx].kind == 1){         
            //emit LID(M = table[symIdx].value), opcode = 1
            emit(1 , 0, table[symIdx].val); 
        }
        else if(table[symIdx].kind == 2){
            //emit LOD (M = table[symIdx].address), opcode = 3
            emit(3 , 0, table[symIdx].addr);
        }

        //get next token
        idx++;

        /*Debug*/
        // printf("\nIt reaches here");
        // printf("Token at the end of factor is %s\n", tokenArray[idx].type);
    }
    else if(tokenArray[idx].token == numbersym){
        //emit LIT
        //FIXME: is this correct?
        emit(1 /*LIT*/, 0, tokenArray[idx].val);

        //get the next token
        idx++;      
    }
    else if(tokenArray[idx].token == lparentsym){
        //get the next token
        idx++;
        expression(tokenArray);
        
        //I wonder if the expression will update idx, it expression not update idx, may need to update here
        if(tokenArray[idx].token != rparentsym){
            printf("\nError: Right parenthesis must follow left parenthesis");
            exit(1);
        }
        //get the next token
        idx++;
    }
    else{
       printf("\nError: Arithmetic equations must contain operands, parentheses, numbers, or symbols"); 
       exit(1);
    }
}

void term(token tokenArray[]){
    factor(tokenArray);
    while(tokenArray[idx].token == multsym || tokenArray[idx].token == slashsym){
        if(tokenArray[idx].token == multsym){
            //get next token
            idx++;
            factor(tokenArray);
            //emit MUL
            emit(2,0,3);      //MUL :  3
        }
        else{
            //get next token
            idx++;
            factor(tokenArray);
            //emit DIV
            emit(2,0,4);     //DIV :  4
        }
        
    }
}

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
//add token into symbol table
void addTable(int kind_of_token ,char name_of_token[],int val_of_token, int level_of_token, int addr_of_token){
    table[table_index].kind = kind_of_token;
    strcpy(table[table_index].name,name_of_token);
    table[table_index].val = val_of_token;
    table[table_index].level = level_of_token;
    table[table_index].addr = addr_of_token;
    table[table_index].mark = 0;  //initilize equal = 0 for now

    table_index++;
}


//return idx if found, otherwise return -1
int checkTable(char string[], int string_kind){
    // printf("\n symbol table size; %d\n", table_index);

    int symbol_index = table_index - 1;

    //linear search through the symbol table looking at name
    while(symbol_index != 0){
        // printf("\n %s\t\t %d", table[symbol_index].name, table[symbol_index].kind);
        if((strcmp(table[symbol_index].name, string) == 0) && table[symbol_index].kind == string_kind){
            // printf("\n return symbol index value %d\n", symbol_index);
            return symbol_index;
        }
        symbol_index--;
    }

    return 0;
}

void typeOPR(char stringOPR[], int opr){
    switch (opr)
    {
        case RTN:
            strcpy(stringOPR, "RTN");
            break;
        case ADD:
            strcpy(stringOPR, "ADD");
            break;
        case SUB:
            strcpy(stringOPR, "SUB");
            break;
        case MUL:
            strcpy(stringOPR, "MUL");
            break;
        case DIV:
            strcpy(stringOPR, "DIV");
            break;
        case EQL:
            strcpy(stringOPR, "EQL");
            break;
        case NEQ:
            strcpy(stringOPR, "NEQ");
            break;
        case LSS:
            strcpy(stringOPR, "LSS");
            break;
        case LEQ:
            strcpy(stringOPR, "LEQ");
            break;
        case GTR:
            strcpy(stringOPR, "GTR");
            break;
        case GEQ:
            strcpy(stringOPR, "GEQ");
            break;
        case ODD:
            strcpy(stringOPR, "ODD");
            break;
        default:
            break;
    }  
}


void printAssembly(){
    printf("\nAssembly Code");
    printf("\nLine\t\tOP\t\tL\t\tM");
    int line = 0;

    for(int i = 0; i < codeIndex; i++){
        char string[100];

        if (assembly[i].op == 2)
            typeOPR(string, assembly[i].m);
        else 
        OpCode(string,assembly[i].op);

        printf("\n%d\t\t%s\t\t%d\t\t%d ",line,string,assembly[i].l,assembly[i].m);
        line++;
    }
}


//Print assymbly code
void OpCode(char stringOP[], int OP){
    switch (OP)
    {
        case LIT:
            strcpy(stringOP, "LIT");
            break;
        case OPR:
            strcpy(stringOP, "OPR");
            break;
        case LOD:
            strcpy(stringOP, "LOD");
            break;
        case STO:
            strcpy(stringOP, "STO");
            break;
        case CAL:
            strcpy(stringOP, "CAL");
            break;
        case INC:
            strcpy(stringOP, "INC");
            break;
        case JMP:
            strcpy(stringOP, "JMP");
            break;
        case JPC:
            strcpy(stringOP, "JPC");
            break;
        case SYS:
            strcpy(stringOP, "SYS");
            break;
        default:
            break;
    }  
}
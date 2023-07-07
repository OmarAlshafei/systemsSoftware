// Omar Alshafei and Hung Tran
// Hw 3

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// constants
#define IDENT_MAX 11
#define NUM_MAX 5
#define MAX_SYMBOL_TABLE_SIZE 500

// token values
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, xorsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym
} token_type;

typedef struct instructions{
    char OP[4];
    int L;
    int M;
} instructions;

// token struct 
typedef struct token {
    int token;   
    int  val;   
    char type[15];  
}token;

// symbol struct
typedef struct{
    int kind;       // const = 1, var = 2, proc = 3
    char name[10];  // name up to 11 chars
    int val;        // number (ASCII value)
    int level;      // L level
    int addr;       // M address
    int mark;       // to indicate unavailable or deleted
}symbol;


// global variable
instructions assembly[MAX_SYMBOL_TABLE_SIZE];
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
token* tokenArr; 
int tableIndex = 0;    // index of the symbol table
int curToken;          // stores the current token to analyze
int maxToken;          // stores the max index of token array
int curTokenIdx = 0;   // stores the current token to analyze
int lineCount = 0;


// return 1 if char is a special symbols, else return 0
int isSpecialSymbol(char c){
    if(c == '+' || c == '-' || c == '*' || c == '/') return 1; 

    if(c == '(' || c == ')' || c == '=' || c == ',') return 1; 

    if(c == '.' || c == '<' || c == '>' || c == ';' || c == ':') return 1; 

    return 0;
}


int program(FILE*);
int block(FILE*);
int constDeclaration(FILE*);
int varDeclaration(FILE*);
int statement(FILE*);
int condition(FILE*);
int expression(FILE*);
int term(FILE*);
int factor(FILE*);
void errorRecovery(FILE*);
void addTable(int, char*, int, int, int, int);
int checkTable(int, char*);
void printTable();
int getToken(FILE*);


// driver function 
int main(int argc, char *argv[]) {

    FILE *fp;
    fp = fopen(argv[1], "r"); 


    if (fp == NULL){
        printf("File CANNOT be open!");
        
        exit(0);
    }

    // local variables 
    char reservedWords[14][20] = {"const", "var", "call", "begin", "end", "if", "then",
                                  "xor", "while", "do", "read", "write", "odd"};
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

    tokenArr = malloc(sizeof(token) * len);        // token array that will hold the info for each token 
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

            // call
            else if(strcmp(buffer, reservedWords[2]) == 0){
                strcpy(tokenArr[tokenIdx].type, "call");
                tokenArr[tokenIdx].token = callsym; 
            }

            // begin
            else if(strcmp(buffer, reservedWords[3]) == 0){
                strcpy(tokenArr[tokenIdx].type, "begin");
                tokenArr[tokenIdx].token = beginsym; 
            }

            // end
            else if(strcmp(buffer, reservedWords[4]) == 0){
                strcpy(tokenArr[tokenIdx].type, "end");
                tokenArr[tokenIdx].token = endsym; 
            }

            // if
            else if(strcmp(buffer, reservedWords[5]) == 0){
                strcpy(tokenArr[tokenIdx].type, "if");
                tokenArr[tokenIdx].token = ifsym; 
            }

            // then
            else if(strcmp(buffer, reservedWords[6]) == 0){
                strcpy(tokenArr[tokenIdx].type, "then");
                tokenArr[tokenIdx].token = thensym; 
            }

            // xor
            else if(strcmp(buffer, reservedWords[7]) == 0){
                strcpy(tokenArr[tokenIdx].type, "xor");
                tokenArr[tokenIdx].token = xorsym; 
            }

            // else
            // else if(strcmp(buffer, reservedWords[8]) == 0){
            //     strcpy(tokenArr[tokenIdx].type, "else");
            //     tokenArr[tokenIdx].token = elsesym; 
            // }

            // while
            else if(strcmp(buffer, reservedWords[8]) == 0){
                strcpy(tokenArr[tokenIdx].type, "while");
                tokenArr[tokenIdx].token = whilesym; 
            }

            // do
            else if(strcmp(buffer, reservedWords[9]) == 0){
                strcpy(tokenArr[tokenIdx].type, "do");
                tokenArr[tokenIdx].token = dosym; 
            }

            // read
            else if(strcmp(buffer, reservedWords[10]) == 0){
                strcpy(tokenArr[tokenIdx].type, "read");
                tokenArr[tokenIdx].token = readsym; 
            }

            // write
            else if(strcmp(buffer, reservedWords[11]) == 0){
                strcpy(tokenArr[tokenIdx].type, "write");
                tokenArr[tokenIdx].token = writesym; 
            }

            // odd
            else if(strcmp(buffer, reservedWords[12]) == 0){
                strcpy(tokenArr[tokenIdx].type, "odd");
                tokenArr[tokenIdx].token = oddsym; 
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
    char* tokenVar = malloc(sizeof(char) * (tokenIdx * 3 + 1));

    int x = 0;
    for (int i = 0; i < tokenIdx; i++) {
        x += sprintf(tokenVar + x, "%d ", tokenArr[i].token);
        
        if (tokenArr[i].token == 3) {
            x += sprintf(tokenVar + x, "%d ", tokenArr[i].val);
        }
        if (tokenArr[i].token == 2) {
            x += sprintf(tokenVar + x, "%s ", tokenArr[i].type);
        }
    }


    // ----------------------------------- HW3 -----------------------------------
    
    FILE *ifp = fopen("lexemelist.txt", "w+");
    fputs(tokenVar, ifp);
    fclose(ifp);

    FILE *rfp = fopen("lexemelist.txt", "r");
    if (program(rfp) == -1){
        printf("Error");
        fclose(ifp);
        return -1;
    }
    
    fclose(fp);
    fclose(rfp);
    free(tokenVar);
    free(inputStr);
}

// ----------------------------------- HW3 -----------------------------------

// get the next token
int getToken(FILE* input){

    if(input != EOF){
        int temp;
        fscanf(input, "%d", &temp); // get next token
        return temp;
    }

    printf("End of file");
    return INT32_MIN;
}

// skip tokens until a “;” is found 
// return the next token
void errorRecovery(FILE *input){
    while(curToken != semicolonsym){
        curToken = getToken(input);

        if(curToken == INT32_MIN){
            printf("Error: no more token before finding ;");
            exit(1);
        }
    }
}

// add a symbol to the symbol table
void addTable(int kind, char name[10], int val, int level, int address, int mark){
    symbol_table[tableIndex].kind = kind; 
    strcpy(symbol_table[tableIndex].name, name);
    symbol_table[tableIndex].val = val; 
    symbol_table[tableIndex].level = level; 
    symbol_table[tableIndex].addr = address; 
    symbol_table[tableIndex].mark = mark; 
    tableIndex++;
}

// check the symbol table for a token kind and name
// if found then return the index, else return -1
int checkTable(int kind, char name[10]){
    for(int i = tableIndex - 1; i > 0; i--){
        if((symbol_table[i].kind == kind) && (strcmp(symbol_table[i].name, name) == 0))
            return i; 
    }

    return -1;
}

// print the content of the symbol table
void printTable(){
    int cur = 1;

    printf("\nSymbol Table: ");
    printf("\nKind\t\tName\t\tValue\t\tLevel\t\tAddress\t\tMark");

    while(cur < tableIndex){
        printf("\n%d\t\t%s\t\t%d\t\t%d\t\t%d\t\t%d",symbol_table[cur].kind, symbol_table[cur].name, 
        symbol_table[cur].val, symbol_table[cur].level, symbol_table[cur].addr, symbol_table[cur].mark);
        cur++;
    }
}


int program(FILE *input){
    
    if (block(input) == -1){
        return -1;
    }
    if (tokenArr->token != periodsym){
        printf("Error : program must end with period %d %s\n", tokenArr->token, tokenArr->type);
        return -1;
    }
    
    // FIXME - emit(9, "SYS", 0, 3); we need to make an emit func
    
    printf("Assembly Code:\n\n");
    printf("Line\tOP\tL\tM\n");
    printf("  0\tJMP\t0\t3\n");

    for (int i = 0; i < lineCount; i++){
        printf("%d\t%s\t%d\t%d\n", i, assembly[i].OP, assembly[i].L, assembly[i].M);
    }

    return 0;
}

int block(FILE *input){
    
  int contant = constDeclaration(input);
  int var = varDeclaration(input);

  //emit(6, "INC", 0, (4 + num)); we need to make an emit func
  
  int result = statement(input);

  return result;
}

int constDeclaration(FILE *input){

    curToken = getToken(input);

    if(curToken != constsym) return 0;

    else if(curToken == constsym){

        do{
            char tempName[10];
            int tempVal;

            curToken = getToken(input);

            // error handling
            if(curToken != identsym){
                printf("Error: const keywords must be followed by identifiers ");
                exit(1);
            }

            fscanf(input, "%d", tempName);

            curToken = getToken(input);

            if(curToken != becomessym){
                printf("Error: constants must be assigned with := ");
                exit(1);
            }

            curToken = getToken(input);
            if(checkTable(1, tempName) == -1){
                addTable(1, tempName, tempVal, 0, 0, 0);
            }
            else{
                printf("Error: symbol name has already been declared");
                exit(1);
            }

            curToken = getToken(input);

        }while(curToken == commasym);

        // error handling
        if(curToken != semicolonsym){
            printf("Error: constant declaration must be followed by a semicolon ");
            errorRecovery(input);
        }

    }
}


int condition(FILE *input){
    curToken = getToken(input);
    
    if (curToken == oddsym){
        getToken(input);

    if (expression(input) == -1) 
        return -1;

    emit(2, "OPR", 0, 6);
    }
    else{
        if (expression(input) == -1) 
            return -1;

        switch(curToken){
            case eqsym:
                getToken(input);
                if (expression(input) == -1) 
                    return -1;
                emit(2, "OPR", 0, 8);
                break;
            case neqsym:
                getToken(input);
                if (expression(input) == -1) 
                    return -1;
                emit(2, "OPR", 0, 9);
                break;
            case lessym:
                getToken(input);
                if (expression(input) == -1) 
                    return -1;
                emit(2, "OPR", 0, 10);
                break;
            case leqsym:
                getToken(input);
                if (expression(input) == -1) 
                    return -1;
                emit(2, "OPR", 0, 11);
                break;
            case gtrsym:
                getToken(input);
                if (expression(input) == -1) 
                    return -1;
                emit(2, "OPR", 0, 12);
                break;
            case geqsym:
                getToken(input);
                if (expression(input) == -1) 
                    return -1;
                emit(2, "OPR", 0, 13);
                break;
            default:
                printf("Error : condition must contain comparison operator");
                return -1;
        }
  }

  return 1;
}

int expression(FILE *input){
    
    curToken = getToken(input);

    if (curToken == minussym){
        curToken = getToken(input);

        if (term(input) == -1)
            return -1;
        
        emit(2, "OPR", 0, 1);

        while (curToken == plussym || curToken == minussym){
            if (curToken == plussym){
                curToken = getToken(input);
                if (term(input) == -1)
                    return -1;
                emit(2, "OPR", 0, 2);
            }
            else{
                curToken = getToken(input);
                if (term(input) == -1)
                    return -1;
                emit(2, "OPR", 0, 3);
            }
        }
    }
    else{
        if (curToken == plussym)
            curToken = getToken(input);

        if (term(input) == -1)
            return -1;

        while (curToken == plussym || curToken == minussym){
            if (curToken == plussym){
                curToken = getToken(input);
                if (term(input) == -1)
                    return -1;
                emit(2, "OPR", 0, 2);
            }
            else{
                curToken = getToken(input);
                if (term(input) == -1)
                    return -1;
                emit(2, "OPR", 0, 3);
            }
        }
    }
    return 1;
}

int term(FILE *input){
    curToken = getToken(input);

  if (factor(input) == -1){
    return -1;
  }

  while (curToken == multsym || curToken == slashsym){
    switch(curToken){
        
        case multsym:
            curToken = getToken(input);
            if (factor(input) == -1)
            return -1;
            emit(2, "OPR", 0, 4);
            break;
        
        case slashsym:
            curToken = getToken(input);
            if (factor(input) == -1)
            return -1;
            emit(2, "OPR", 0, 5);
            break;
        
        default:
            printf("Error : Unexpected token %d. Line %d.\n", curToken, lineCount);
            return -1;
            
    }

  }

  return 1;
}
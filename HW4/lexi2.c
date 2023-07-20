//Hong Thy Nguyen 
/*
    REDO:emit in statements
        printing part.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define CHARACTER_MAX_SIZE 11         //max length of characters
#define NUM_MAX_SIZE 5                //max length of a number
#define MAX_SYMBOL_TABLE_SIZE 500
#define CODE_SIZE 1000             

#define LIT  1         // Literal
#define OPR  2         // Operation
#define LOD  3         // Load
#define STO  4         // Store
#define CALL 5         // Call
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
#define LSS  7         // Less Than
#define LEQ  8         // Less Than or Equal 
#define GTR  9         // Greater Than 
#define GEQ  10        // Greater Than or Equal
#define ODD  11        // ODD

//SYS
#define WRITE 1 
#define READ  2
#define END   3


typedef enum{
    oddsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5,
multsym = 6, slashsym = 7, xorsym = 8, eqsym = 9, neqsym = 10, lessym = 11,
leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16,
commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20,
beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26,
callsym = 27, constsym = 28, varsym = 29, procsym = 30, writesym = 31,
readsym = 32, elsesym = 33
}tokenType;


typedef struct{
    int token;          //value of token type
    int val;            //value of token
    char type[15];      //if it is an identifier, it stores the name of an identifier
                        //otherwise it stores the token type in string
    int tokenKind;      //const = 1, var = 2, procedure = 3
}token;


typedef struct {
    int kind;           //const = 1, var = 2, proc = 3
    char name[10];      //name up to 11
    int val;            //number(ASCII value)
    int level;          //L level
    int addr;           //M address
    int mark;           //to indicate unavailable or deleted
}symbol;

typedef struct{
    int op;             //Operation code
    int l;              // Lexicographical level
    int m;              //Modifier/Argument (constant, address, or OPR)
}instruction;


symbol symbol_table[MAX_SYMBOL_TABLE_SIZE]; 
int table_index = 1;                        //current index of symbol table
int idx = 0;                                //current index of token array
instruction codeStack[CODE_SIZE];           //stack that stores all code
int codeIndex = 0;                          //current index of code stack
int line = 0;                               //current line
int flag = 0;                               //used for printing assembly when line = 0

int errorflag = 0;                          //used for printing error when from code start to error then keep printing if there is no error after
int code_start = 0;                         //index where code starts
int code_end = 0;                           //index where code ends

int level = 0;                                  //Max lexicographical level reached.
int curr_level = 0;                             //Current lexicographical level.


char *reservedWords[] = {"const", "var", "begin", "end","if", "then", "xor", "else", "while", "do",  "read","write","odd", "call", "procedure"};

char specialSymbols[13] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'};


void print_source(char *source,int len);
void remove_comment(char *source, char *newSource, int length) ;
int is_special_symbol(char ch);
void tokenize(char newSource[], int sourceLength, token tokenArray[], int *size);
void symbol_table_add(int kind_of_token ,char name_of_token[],int val_of_token, int level_of_token, int addr_of_token);
int symbol_table_check(char string[], int string_kind);
void emit(int op,  int l, int m);
void const_declaration(token tokenArray[],int size);
int var_declaration(token tokenArray[], int size);
void block(token tokenArray[], int size);
void term(token tokenArray[],int size);
void expression(token tokenArray[], int size);
void factor(token tokenArray[],int size);
void program(token tokenArray[], int size);
void condition(token tokenArray[], int size);
void statement(token tokenArray[], int size);
void print_assembly_code();
void proc_declaration(token tokenArray[], int size);


int main(int argc, char *argv[]){
    
    /*Read the input ifp*/
    FILE *ifp = fopen(argv[1], "r");

    //calculating the size of the file
    int len;
    fseek(ifp, 0, SEEK_END);
    len = ftell(ifp);
    fseek(ifp, 0, SEEK_SET);

    //input souce is input code from the file
    //new source is code after removing comments
    char inputSource[len];
    char newSource[len];

    //read in the file
    int cur = 0;
    while(fscanf(ifp,"%c", &inputSource[cur]) != EOF){
        cur++;
    }
    
    /*Removes comments and tab line*/
    remove_comment(inputSource,newSource,len);

    //declare token array and size of array
    token tokenArray[len];
    int size = 0;

    /*Tokenize and print out lexeme table, lexeme list*/
    tokenize(newSource,len, tokenArray, &size);
    
    /*Parser and code generator*/
    //Initilze code stack 
    for(int i = 0; i < CODE_SIZE; i++){
        if(i == 0){
            codeStack[i].op = 7;
            codeStack[i].l = 0;
            codeStack[i].m = 3;
            codeIndex++;
        }else{
            codeStack[i].op = 0;
            codeStack[i].l = 0;
            codeStack[i].m = 0;
        }
    }

    //starting parsing and generating assembly code
    program(tokenArray, size);

    print_assembly_code();

    fclose(ifp);
    return 0;
}

void print_source(char *source,int len){
    for(int i = 0; i < len; i++){
        printf("%c",source[i]);
    }
    printf("\n");
}


void remove_comment(char *source, char *newSource, int length)  {
    int i = 0;                  //keep track of characters in source

    while(i <= length){

        if(source[i] == '/' && source[i+1] == '*'){
                newSource[i] = ' ';
                newSource[i+1] = ' ';
                i +=2 ;

                while(true){
                    if(source[i] == '*' && source[i+1] != '/'){ 
                        //case: /*hello world ... I am *Hong**/
                        newSource[i] = ' ';
                        i += 1;
                    }
                    else if(source[i] == '*' && source[i+1] == '/'){
                        newSource[i] = ' '; 
                        i++;
                        break; 
                    }
                    else{
                        newSource[i] = ' ';
                        i++;
                    }
                }

                newSource[i] = ' ';
                newSource[i+1] = ' ';
                i += 2;
                
        }else{
            newSource[i] = source[i];
            i++;
        }
        
    }
    // newSource[i] = '.';

    
}


//return 1 if a character is a special symbol
//otherwise return 0
int is_special_symbol(char ch){
    for(int i = 0; i < 13; i++){
        if(ch == specialSymbols[i])
            return 1;
    } 
    return 0;
}


void tokenize(char newSource[], int sourceLength, token tokenArr[], int *size){

    char buffer[100];                   //temporary string 
    int bufferIdx = 0;                  //current size of buffer
    int tokenIdx = 0;                   //current size of token
    int currentIndex = 0;

    // tokenize
    while(currentIndex < sourceLength){

        //check if the current char is a white space
        if(isspace(newSource[currentIndex]) != 0){
            currentIndex++;
            continue;
        }

        //check if the current char is a letter
        else if (isalpha(newSource[currentIndex]) != 0 && isspace(newSource[currentIndex]) == 0 ){
            
            //continuing checking if the next char is a letter or a digit
            while( (isalpha(newSource[currentIndex]) != 0 || isdigit(newSource[currentIndex]) != 0) && isspace(newSource[currentIndex]) == 0){

                //check if identifier is over 11 char, error handling
                if(bufferIdx >= CHARACTER_MAX_SIZE){
                    printf("Error: Identifier names cannot exceed 11 characters\n");
                    while( (isalpha(newSource[currentIndex]) != 0 || isdigit(newSource[currentIndex]) != 0) && isspace(newSource[currentIndex]) == 0){
                        currentIndex++;
                    }
                    strcpy(buffer,"ERROR");
                }
                else{
                    buffer[bufferIdx++] = newSource[currentIndex++];
                }
            }
            buffer[bufferIdx] = '\0';

            //reach here meaning we encountered a space, which end of a word
            //check if it is a reserved word
            //const
            if(strcmp(buffer,reservedWords[0]) == 0){
                strcpy(tokenArr[tokenIdx].type, "const");
                tokenArr[tokenIdx].token = constsym;
            
            }

            // var
            else if(strcmp(buffer, reservedWords[1]) == 0){
                strcpy(tokenArr[tokenIdx].type, "var");
                tokenArr[tokenIdx].token = varsym; 
            }

            // begin
            else if(strcmp(buffer, reservedWords[2]) == 0){
                strcpy(tokenArr[tokenIdx].type, "begin");
                tokenArr[tokenIdx].token = beginsym; 
            }

            // end
            else if(strcmp(buffer, reservedWords[3]) == 0){
                strcpy(tokenArr[tokenIdx].type, "end");
                tokenArr[tokenIdx].token = endsym; 
            }

            // if
            else if(strcmp(buffer, reservedWords[4]) == 0){
                strcpy(tokenArr[tokenIdx].type, "if");
                tokenArr[tokenIdx].token = ifsym; 
            }

            // then
            else if(strcmp(buffer, reservedWords[5]) == 0){
                strcpy(tokenArr[tokenIdx].type, "then");
                tokenArr[tokenIdx].token = thensym; 
            }

            // xor
            else if(strcmp(buffer, reservedWords[6]) == 0){
                strcpy(tokenArr[tokenIdx].type, "xor");
                tokenArr[tokenIdx].token = xorsym; 
            }

            // else
            else if(strcmp(buffer, reservedWords[7]) == 0){
                strcpy(tokenArr[tokenIdx].type, "else");
                tokenArr[tokenIdx].token = elsesym; 
            }

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

            //odd
            else if(strcmp(buffer, reservedWords[12]) == 0){
                strcpy(tokenArr[tokenIdx].type, "odd");
                tokenArr[tokenIdx].token = oddsym;
            }

            //call
            else if(strcmp(buffer, reservedWords[13]) == 0){
                strcpy(tokenArr[tokenIdx].type, "call");
                tokenArr[tokenIdx].token = callsym;
            }

            //procedure
            else if(strcmp(buffer, reservedWords[14]) == 0){
                strcpy(tokenArr[tokenIdx].type, "procedure");
                tokenArr[tokenIdx].token = procsym;
            }
            
            // Error handling
            else if(strcmp(buffer, "ERROR") == 0){
                bufferIdx = 0;
                continue;
            }

            // identifier 
            else{

                strcpy(tokenArr[tokenIdx].type, buffer);
                tokenArr[tokenIdx].token = identsym; 
            }

            // printf("%s\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);            
                     
            tokenIdx++;
            bufferIdx = 0;
            currentIndex--;
        }

        //check if the current char is a digit
        else if(isdigit(newSource[currentIndex]) != 0 && isspace(newSource[currentIndex]) == 0){

            while((isalpha(newSource[currentIndex]) != 0 || isdigit(newSource[currentIndex]) != 0) && isspace(newSource[currentIndex]) == 0){
                    
                    // check if identifier contain digit, error handling
                    if(isalpha(newSource[currentIndex]) != 0){
                        printf("Error: Identifiers cannot begin with a digit\n");

                        while((isalpha(newSource[currentIndex]) != 0 || isdigit(newSource[currentIndex]) != 0) && isspace(newSource[currentIndex]) == 0)                 
                            currentIndex++;

                        strcpy(buffer, "ERROR");
                    }

                    // check if digit is over 5 char, error handling
                    else if(bufferIdx >= NUM_MAX_SIZE){
                        printf("Error: Numbers cannot exceed 5 characters\n");

                        while((isalpha(newSource[currentIndex]) != 0 || isdigit(newSource[currentIndex]) != 0) && isspace(newSource[currentIndex]) == 0)                 
                            currentIndex++;

                        strcpy(buffer, "ERROR");   
                    }

                    else{
                        buffer[bufferIdx++] = newSource[currentIndex++];
                    }
                }


            // error handling 
            if(strcmp(buffer, "ERROR") == 0){
                bufferIdx = 0;
                continue;
            }

            buffer[bufferIdx] = '\0';

            //store info into array
            strcpy(tokenArr[tokenIdx].type,"3");
            tokenArr[tokenIdx].val = atoi(buffer);
            tokenArr[tokenIdx].token = numbersym;

            // printf("%d\t\t%d\n", tokenArr[tokenIdx].val, tokenArr[tokenIdx].token);
            tokenIdx++;
            bufferIdx = 0;
            currentIndex--; 

        
        }

        //check if the current char is a special character
        else if(is_special_symbol(newSource[currentIndex]) != 0){

            // +
            if(newSource[currentIndex] == '+'){
                strcpy(tokenArr[tokenIdx].type, "+");
                tokenArr[tokenIdx].token = plussym; 
            }

            // -
            else if(newSource[currentIndex] == '-'){
                strcpy(tokenArr[tokenIdx].type, "-");
                tokenArr[tokenIdx].token = minussym; 
            }

            // *
            else if(newSource[currentIndex] == '*'){
                strcpy(tokenArr[tokenIdx].type, "*");
                tokenArr[tokenIdx].token = multsym; 
            }

            // /
            else if(newSource[currentIndex] == '/'){
                strcpy(tokenArr[tokenIdx].type, "/");
                tokenArr[tokenIdx].token = slashsym; 
            }

            // (
            else if(newSource[currentIndex] == '('){
                strcpy(tokenArr[tokenIdx].type, "(");
                tokenArr[tokenIdx].token = lparentsym; 
            }

            // )
            else if(newSource[currentIndex] == ')'){
                strcpy(tokenArr[tokenIdx].type, ")");
                tokenArr[tokenIdx].token = rparentsym; 
            }

            // =
            else if(newSource[currentIndex] == '='){
                strcpy(tokenArr[tokenIdx].type, "=");
                tokenArr[tokenIdx].token = eqsym; 
            }

            // ,
            else if(newSource[currentIndex] == ','){
                strcpy(tokenArr[tokenIdx].type, ",");
                tokenArr[tokenIdx].token = commasym; 
            }

            // .
            else if(newSource[currentIndex] == '.'){
                strcpy(tokenArr[tokenIdx].type, ".");
                tokenArr[tokenIdx].token = periodsym; 
            }

            // <
            else if(newSource[currentIndex] == '<'){
                if(newSource[currentIndex + 1] == '>'){
                    strcpy(tokenArr[tokenIdx].type, "<>");
                    tokenArr[tokenIdx].token = neqsym; 
                    currentIndex++; 
                }
                else if(newSource[currentIndex + 1] == '='){
                    strcpy(tokenArr[tokenIdx].type, "<=");
                    tokenArr[tokenIdx].token = leqsym; 
                    currentIndex++; 
                }
                else{
                    strcpy(tokenArr[tokenIdx].type, "<");
                    tokenArr[tokenIdx].token = lessym; 
                }
            }

            // >
            else if(newSource[currentIndex] == '>'){
                if(newSource[currentIndex + 1] == '='){
                    strcpy(tokenArr[tokenIdx].type, ">=");
                    tokenArr[tokenIdx].token = geqsym; 
                    currentIndex++;                         
                }
                else{
                    strcpy(tokenArr[tokenIdx].type, ">");
                    tokenArr[tokenIdx].token = gtrsym; 
                }
            }

            // ;
            else if(newSource[currentIndex] == ';'){
                strcpy(tokenArr[tokenIdx].type, ";");
                tokenArr[tokenIdx].token = semicolonsym; 
            }

            // :
            else if(newSource[currentIndex] == ':'){
                if(newSource[currentIndex + 1] == '='){
                    strcpy(tokenArr[tokenIdx].type, ":=");
                    tokenArr[tokenIdx].token = becomessym; 
                    currentIndex++; 
                }
            }

            // printf("%s\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);
            tokenIdx++;
            bufferIdx = 0;

        }

        //error handling, invalid characters
        else{
            printf("Error: Invalid Symbol\n");
            currentIndex++;
            continue;
        }
        currentIndex++;
    }


    *size = tokenIdx;
}


//PARSER AND CODE GENERATOR --------------------------------NEED TO BE FIXED---------------

//FIXED
void symbol_table_add(int kind_of_token ,char name_of_token[],int val_of_token, int level_of_token, int addr_of_token){
    
    symbol_table[table_index].kind = kind_of_token;
    strcpy(symbol_table[table_index].name,name_of_token);
    symbol_table[table_index].val = val_of_token;
    symbol_table[table_index].level = level_of_token;
    symbol_table[table_index].addr = addr_of_token;
    symbol_table[table_index].mark = 0;  //0 for all token 

    table_index++;
}


//return idx if found, otherwise return -1
int symbol_table_check(char string[], int string_kind){

    int symbol_index = table_index - 1;

    //linear search through the symbol table looking at name
    while(symbol_index != 0){
        if((strcmp(symbol_table[symbol_index].name, string) == 0) && symbol_table[symbol_index].kind == string_kind){
            return symbol_index;
        }
        symbol_index--;
    }

    return 0;
}


//FIXED: we do not need the register right?
void emit(int op, int l, int m){
    if(codeIndex > CODE_SIZE){
        printf("\nError: Code index exceeds code max size");
        exit(1);
    }
    else{
        codeStack[codeIndex].op = op;
        codeStack[codeIndex].l = l;
        codeStack[codeIndex].m = m;
        codeIndex++;
    }
}

//FIXED
void const_declaration(token tokenArray[], int size){
    if(tokenArray[idx].token == constsym){
        do{
            //get next token
            idx++;

            if(tokenArray[idx].token != identsym){
                print_assembly_code();
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
                print_assembly_code();
                printf("\nError: constants must be assigned with = ");
                exit(1);
            }

            //get next token
            idx++;

            if(tokenArray[idx].token != numbersym){
                print_assembly_code();
                printf("\nError: constants must be assigned with an integer value ");
                exit(1);
            }

            if(table_index == 0 || symbol_table_check(indentName,1) == 0) {
                symbol_table_add(1,indentName,tokenArray[idx].val,curr_level,0);
            }else{
                print_assembly_code();
                printf("\nError: symbol name has already been declared");
                exit(1);
            }
                
            //get the next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            print_assembly_code();
            printf("\nError: Constant declaration must be followed by a semicolon ");
            //update code code start
            code_start = codeIndex;

            //error recovery: keep reading until find a semicolon
            idx++;
            while(tokenArray[idx].token != semicolonsym){
                idx++;
            }
        }

    }

}


//FIXED
//return numbers of variables
int var_declaration(token tokenArray[], int size){
    int numVars = 0;
    if(tokenArray[idx].token == varsym){
        do{
            numVars++;

            //get the next token
            idx++;

            if(tokenArray[idx].token != identsym){
                print_assembly_code();
                printf("\nError: var declaration must be followed by identifier");
                exit(1);
            }

            //if table is empty or it is not in the list, add it
            //otherwise it is error
            if(table_index == 0 || symbol_table_check(tokenArray[idx].type,2) == 0){ 
                symbol_table_add(2, tokenArray[idx].type, 0,curr_level,numVars + 2);  
            }else{
                print_assembly_code();
                printf("Symbol name has already been declared");
                exit(1);
            }

            //get next token
            idx++;

        }while(tokenArray[idx].token == commasym);

        if(tokenArray[idx].token != semicolonsym){
            print_assembly_code();
            printf("\nError: variable declarations must be followed by a semicolon");
            //update code start
            code_start = codeIndex;

            //error recovery: keep reading until find a semicolon
            idx++;
            while(tokenArray[idx].token != semicolonsym){
                idx++;
            }
        }

    }
    return numVars;
}


void expression(token tokenArray[], int size){
    term(tokenArray,size);
    while(tokenArray[idx].token == plussym || tokenArray[idx].token == minussym){
        if(tokenArray[idx].token == plussym){
            //get the next token
            idx++;
            term(tokenArray,size);
            emit(2, 0 , 1);             //emit ADD, OPR = 2, ADD = 1
            // print_assembly_code();
        }
        else{
            //get the next token
            idx++;

            term(tokenArray,size);
            emit(2, 0, 2);              //emit SUB, OPR = 2, SUB = 2
            // print_assembly_code();
        }
    }
}

//Note: next token is updated
void factor(token tokenArray[], int size){
    if(tokenArray[idx].token == identsym){
        int symIdx = 0;

        //check for undeclared
        for(int i = table_index - 1; i > 0 ; i--){
            if(strcmp(tokenArray[idx].type, symbol_table[i].name) == 0){
                symIdx = i;
            }
        }

        if(symIdx == 0){
            print_assembly_code();
            printf("\nError: Undeclared identifier");
            exit(1);
        }

        //var or const
        if(symbol_table[symIdx].kind == 1){         
            emit(1, 0, symbol_table[symIdx].val);       //emit LIT(M = table[symIdx].value), opcode = 1
            // print_assembly_code();
        }
        else if(symbol_table[symIdx].kind == 2){
            emit(3, 0, symbol_table[symIdx].addr);     //emit LOD (M = table[symIdx].address), opcode = 3
            // print_assembly_code();
        }

        //get next token
        idx++;
    }
    else if(tokenArray[idx].token == numbersym){
        emit(1, 0, tokenArray[idx].val);        //emit LIT
        // print_assembly_code();

        //get the next token
        idx++;      
    }
    else if(tokenArray[idx].token == lparentsym){
        //get the next token
        idx++;

        expression(tokenArray,size);
        
        if(tokenArray[idx].token != rparentsym){
            print_assembly_code();
            printf("\nError: Right parenthesis must follow left parenthesis");
            exit(1);
        }
        //get the next token
        idx++;
    }
    else{
       print_assembly_code();
       printf("\nError: Arithmetic equations must contain operands, parentheses, numbers, or symbols"); 
       exit(1);
    }
}


void term(token tokenArray[], int size){
    factor(tokenArray,size);
    while(tokenArray[idx].token == multsym || tokenArray[idx].token == slashsym){
        if(tokenArray[idx].token == multsym){
            //get next token
            idx++;

            factor(tokenArray,size);
            emit(2,0,3);                //emit MUL, OP = 2, MUL = 3
            // print_assembly_code();
        }
        else{
            //get next token
            idx++;
            
            factor(tokenArray,size);
            emit(2,0,4);                //emit DIV, OP = 2, DIV = 4
            //print_assembly_code();
        }
        
    }
}


void condition(token tokenArray[], int size){
    if(tokenArray[idx].token == oddsym ){    
        //get next token
        idx++;
        
        expression(tokenArray,size);
        emit(2,0,11);                                          //emit ODD, OPR = 2, ODD = 11
        //print_assembly_code();
    }
    else{
        expression(tokenArray,size);
        if(tokenArray[idx].token == eqsym){                    // =
            //get next token
            idx++;

            expression(tokenArray,size);
            emit(2,0,5);                                        //emit EQL, OPR = 2, EQL = 5
            //print_assembly_code();
        }
        else if(tokenArray[idx].token == neqsym){               // <>
            //get next token
            idx++;

            expression(tokenArray,size);
            emit(2,0,6);                                        //emit NQL, OPR = 2, NQL = 6
            //print_assembly_code();
        }
        else if(tokenArray[idx].token == lessym){               // <
            //get next token
            idx++;

            expression(tokenArray,size);
            emit(2,0,7);                                        //emit LSS, OPR = 2, LSS = 7
            //print_assembly_code();
        }
        else if(tokenArray[idx].token == leqsym){               // <= 
            //get next token
            idx++;

            expression(tokenArray,size);
            emit(2,0,8);                                        //emit LEQ, OPR = 2, LEQ = 8
            //print_assembly_code();

        }
        else if(tokenArray[idx].token == gtrsym){               // >
            //get next token
            idx++;

            expression(tokenArray,size);
            emit(2,0,9);                                       //emit GTR, OPR = 2, GTR = 9
            //print_assembly_code();
        }
        else if(tokenArray[idx].token == geqsym){               // >=
            //get next token
            idx++;

            expression(tokenArray,size);                        //emit GEQ, OPR = 2, GEQ = 10
            emit(2,0,10);
            //print_assembly_code();
        }
        else{
            print_assembly_code();
            printf("\nError:Condition must contain comparison operator");
            exit(1);
        }
    }

}


void statement(token tokenArray[], int size){
    if(tokenArray[idx].token == identsym){
        int symIdx = 0;
        
        //check if identifier has been declared
        for(int i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, symbol_table[i].name) == 0){  //found
                if(symbol_table[i].kind == 1){ //constant
                    print_assembly_code();
                    printf("\nError: Identifier cannot be a constant in statement");
                    exit(1);
                }
                else if(symbol_table[i].kind == 2){ //var
                    symIdx = i;
                }
            }
        }
        
        if(symIdx == 0){
            print_assembly_code();
            printf("\nError: Undeclared identifier");
            exit(1);
        }

        if(symbol_table[symIdx].kind != 2){
            print_assembly_code();
           printf("\nError: Only variable values may be altered");
           exit(1); 
        }

        //get the next token
        idx++;

        if(tokenArray[idx].token != becomessym){
            print_assembly_code();
            printf("\nError: Assignment statements must use :=");
            exit(1);
        }
        //get the next token
        idx++;

        expression(tokenArray,size);

        //emit STO(M = table[symIdx].addr)
        emit(4,0,symbol_table[symIdx].addr);
        //print_assembly_code();

        return;        
    }
    else if(tokenArray[idx].token == beginsym){
        do{
            //get next token
            idx++;

            statement(tokenArray,size);
            
            if(tokenArray[idx].token != endsym && tokenArray[idx].token != semicolonsym && tokenArray[idx].token != periodsym){
                print_assembly_code();
                printf("\nError: Semicolon expected");
                //update code start
                code_start = codeIndex;

                //Error recovery: get the next token until we find a semicolon
                while (tokenArray[idx].token != semicolonsym)
                {
                    idx++;
                    if(idx > size){
                        printf("\nError: Cannot find a semicolon in the while program");
                        exit(1);
                    }
                }
                
            }
        }while (tokenArray[idx].token == semicolonsym);

        if(tokenArray[idx].token != endsym){
            print_assembly_code();
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
        
        condition(tokenArray,size);

        int jpc_idx = codeIndex;

        //emit JPC, OPR = 8, M = 0 for now
        emit(8, 0, 0);
        //print_assembly_code();


        if(tokenArray[idx].token != thensym){
            print_assembly_code();
            printf("\nError: If must be followed by then");
            exit(1);
        }

        //get next token
        idx++;

        statement(tokenArray,size);
        
        //update m
        codeStack[jpc_idx].m = codeIndex * 3;
        
        return; 
    }
    else if(tokenArray[idx].token == whilesym){
        //get next token
        idx++;
        
        int loop_idx = codeIndex;
        condition(tokenArray,size);

        if(tokenArray[idx].token != dosym){
            print_assembly_code();
            printf("\nError: While must be followed by do");
            exit(1);
        }

        //get next token
        idx++;

        int jpc_idx = codeIndex;

        //emit JPC, OPR = 8, M = 0 
        emit(8,0,0);
        //print_assembly_code();

        statement(tokenArray,size);

        //emit JMP (M = loop_idx * 3), OPR = 7
        emit(7,0,loop_idx * 3);
        //print_assembly_code();

        //update m for JPC
        codeStack[jpc_idx].m = codeIndex * 3 ;
        return;
    }
    else if(tokenArray[idx].token == readsym){
        //get next token
        idx++;

        if(tokenArray[idx].token != identsym){
            print_assembly_code();
            printf("\nError: Read keywords must be followed by identifier");
            exit(1);
        }

        //store the index of identifier
        int symIdx = 0;

        //Check if identifier has been declared
        for(int i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, symbol_table[i].name) == 0){ //found
                symIdx = i;
            }
        }

        if(symIdx == 0){
            print_assembly_code();
            printf("\nError: Undeclared identifier");
            exit(1);
        }
        //if it is not a var
        if(symbol_table[symIdx].kind != 2){
            print_assembly_code();
            printf("\nError: Only variable values may be altered");
            exit(1);
        }

        //get next token
        idx++;

        //emit READ, OPR = 9 SYS, READ = 2 
        emit(9,0,2);
        //print_assembly_code();

        //emit STO(M = table[symIdx].addr)
        emit(4,0,symbol_table[symIdx].addr);
        //print_assembly_code();

        return;
    }
    else if(tokenArray[idx].token == writesym){
        //get next token
        idx++;

        expression(tokenArray,size);

        //emit WRITE, OPR = 9, M = 1
        emit(9,0,1);
        //print_assembly_code();

        return;
    }
    else if(tokenArray[idx].token == xorsym){
        //get next token
        idx++;

        condition(tokenArray,size);
        int jpcIdx = codeIndex;

        //emit JPC, OPR = 7, M = 0 for now
        emit(8,0,0);
        //print_assembly_code();

        if(tokenArray[idx].token != thensym){
            print_assembly_code();
            printf("\nError: XOR must be followed by then");
            exit(1);
        }

        //get next token
        idx++;

        statement(tokenArray,size);

        if(tokenArray[idx].token != semicolonsym){
            print_assembly_code();
            printf("\nError: Semicolon expected");
            //update code start
            code_start = codeIndex;

            //Error recovery: keep reading until find a semicolon
            idx++;
            while(tokenArray[idx].token != semicolonsym){
                idx++;
                if(idx > size){
                        printf("\nError: Cannot find a semicolon in the while program");
                        exit(1);
                }
            }
        }

        //get next token
        idx++;

        if(tokenArray[idx].token != elsesym){
            print_assembly_code();
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
        codeStack[jpcIdx].m = codeIndex * 3;
        statement(tokenArray,size);
        codeStack[jmpIdx].m = codeIndex * 3;

    }
    else if(tokenArray[idx].token == callsym){
        int symIdx = -1; 

        // get next token
        idx++;

        if(tokenArray[idx].token != identsym){
            printf("\nError: missing identifier\n");
            exit(1);
        }

        int i;
        for(i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, symbol_table[i].name) == 0){
                symIdx = i;
                break;
            }
        }
        if(symIdx == -1){
            // printOut(fp);
            printf("\nError: undeclared identifier\n");
            exit(1); 
        }
        if(symbol_table[symIdx].kind == 3){
            emit(CALL, curr_level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
        }
        else{
            // printOut(fp);
            printf("\nError: call must be followed by a prodecure identifier\n");
            exit(1);
        }

        // get next token
        idx++;
    }
}


void program(token tokenArray[], int size){
    
    block(tokenArray, size);

    
    // idx--;
    if(strcmp(tokenArray[idx].type, ".") != 0){
        print_assembly_code();
        printf("\nThe current character of previous is %s", tokenArray[idx-1].type);
        printf("\nThe current character is %s", tokenArray[idx].type);
        printf("\nError: program must end with period \n");   
        exit(1);
    } 

    //emit end of program: SYS 03
    emit(9,0,3);
    //print_assembly_code();
}


//PRINTING ASSEMBLY CODE FUNCTIONS
void typeOpCode(char stringOP[], int OP){
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


void print_assembly_code(){
    char string[100];

    code_end = codeIndex;

    for(int i = code_start; i < code_end; i++){
        if(i == 0){
            printf("\nAssembly Code");
            printf("\nLine\t\tOP\t\tL\t\tM");
            typeOpCode(string,codeStack[0].op);
            typeOpCode(string,codeStack[0].op);
            printf("\n%d\t\t%s\t\t%d\t\t%d ",line,string,codeStack[0].l,codeStack[0].m);

        }
        else{
            if (codeStack[i].op == 2)
                typeOPR(string, codeStack[i].m);
            else 
            typeOpCode(string,codeStack[i].op);

            printf("\n%d\t\t%s\t\t%d\t\t%d ",line,string,codeStack[i].l,codeStack[i].m);
        }
        line++;


    }
    

    // if (codeStack[codeIndex - 1].op == 2)
    //     typeOPR(string, codeStack[codeIndex - 1].m);
    // else 
    // typeOpCode(string,codeStack[codeIndex - 1].op);

    // printf("\n%d\t\t%s\t\t%d\t\t%d ",line,string,codeStack[codeIndex - 1].l,codeStack[codeIndex - 1].m);
    // line++;
}


//NEW BLOCK OF CODE FOR COMPILER

//FIXME:Not sure if need to do while loop 
void block(token tokenArray[], int size){
    int dx, tx0, cx0;
    int numVars = 0;
    dx = 3;

    tx0 = table_index;

    //need explanation
    //symbol_table[0].addr = codeIdex:
    symbol_table[table_index].addr = codeIndex;

    emit(JMP,0,0);

    //FIXME: if the current level is higher than max level, do we just stop??
    // if(curr_level > level){
    //     printf("\nError: Current level exceeded max level");
    //     exit(1);
    // } 

    do {
        if (tokenArray[idx].token == constsym) {
            const_declaration(tokenArray,size);
        }

        else if (tokenArray[idx].token == varsym) {
            numVars = var_declaration(tokenArray,size);
        }

        else if (tokenArray[idx].token == procsym){
            proc_declaration(tokenArray,size);
            curr_level--;
        }
        idx++;
    }while ((tokenArray[idx].token == constsym)||(tokenArray[idx].token == varsym)||(tokenArray[idx].token == procsym));

    symbol_table[tx0].addr = codeIndex;                                // the space for address for the above jmp is now occupied by the new code idx
    codeStack[symbol_table[tx0].addr].m = codeIndex ;                  // The tentative jump address is fixed up
    cx0 = codeIndex;                                                   // inc 0,dx is generated. At run time, the space of dx is secured
    emit(INC,curr_level,dx + numVars);
    statement(tokenArray,size);
    emit(OPR,0,0);
}

void proc_declaration(token tokenArray[], int size){
    while(tokenArray[idx].token == procsym) {
        //get the next token 
        idx++;

        if(tokenArray[idx].token == identsym){
            //enter(procedure,&tx,&dx,lev); 
            symbol_table_add(3, tokenArray[idx].type, 0, curr_level - 1, 0);

            //get the next token 
            idx++;
        } 
        else{
            printf("Error: Procedure must be followed by an identifier");  
            exit(1);
        } 

        if(tokenArray[idx].token != semicolonsym){
            print_assembly_code();
            printf("\nError: variable declarations must be followed by a semicolon");
            //update code start
            code_start = codeIndex;

            //error recovery: keep reading until find a semicolon
            idx++;
            while(tokenArray[idx].token != semicolonsym){
                idx++;
            }
        }
        // Go to a block one level higher
        curr_level++;
        block(tokenArray,size); 
        
        if(tokenArray[idx].token != semicolonsym){
            print_assembly_code();
            printf("\nError: variable declarations must be followed by a semicolon");
            //update code start
            code_start = codeIndex;

            //error recovery: keep reading until find a semicolon
            idx++;
            while(tokenArray[idx].token != semicolonsym){
                idx++;
            }
        }
    }

}

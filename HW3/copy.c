// Omar Alshafei and Hung Tran

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define CHARACTER_MAX_SIZE 11         //max length of characters
#define NUM_MAX_SIZE 5                //max length of a number
#define MAX_SYMBOL_TABLE_SIZE 500
#define CODE_SIZE 1000               //NOT: NEED TO CHECK FOR CODE SIZE

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

// token values
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, xorsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym
} token_type;

char *reservedWords[] = {"const", "var", "procedure", "call", "begin", "end","if", "then", "xor", "else", "while", "do",  "read","write","odd"};

char specialSymbols[13] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'};


typedef struct{
    int token;          //value of token type
    int val;            //value of token
    char type[15];      //Token Type in string
    int tokenKind;      //const = 1, var = 2, procedure = 3
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

typedef struct instruction{
    int op;
    int l;
    int m;
} instruction;

symbol assembly[MAX_SYMBOL_TABLE_SIZE];
int table_index = 1;                        //current index of symbol table
int idx = 0;                                //current index of token array
instruction codeStack[CODE_SIZE];           //stack that stores all code
int codeIndex = 0;                          //current index of code stack


void printSource(char *source,int len);
void removeComments(char *source, char *newSource, int length) ;
int isSpecialSymbol(char ch);
void tokeize(char newSource[], int sourceLength, token tokenArray[], int *size);
void addTable(int kind_of_token ,char name_of_token[],int val_of_token, int level_of_token, int addr_of_token);
int checkTable(char string[], int string_kind);
void printTable();
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
void print();


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

    printf("Source Program: \n");
    printSource(inputSource,len);
    
    /*Removes comments and tab line*/
    removeComments(inputSource,newSource,len);

    int size = 0;
    token tokenArray[len];

    /*Tokenize and print out lexeme table, lexeme list*/
    tokeize(newSource,len, tokenArray, &size);

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

    program(tokenArray, size);
    
    symbol_table_print();

    print();

    fclose(ifp);
    return 0;
}


//LEXICAL ANALYZER ------------------------------------------
void printSource(char *source,int len){
    
    for(int i = 0; i < len; i++){
        printf("%c",source[i]);
    }
    printf("\n");
}


void removeComments(char *source, char *newSource, int length)  {
    int i = 0;                  //keep track of characters in source

    while(i <= length){

        if(source[i] == '/' && source[i+1] == '*'){
                newSource[i] = ' ';
                newSource[i+1] = ' ';
                i +=2 ;
                while(source[i] != '*' && source[i+1] != '/'){
                    newSource[i] = ' '; /*Amy*/
                    i++; 
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


//return 1 if a character is a special symbol, otherwise return 0
int isSpecialSymbol(char ch){

    for(int i = 0; i < 13; i++){
        if(ch == specialSymbols[i])
            return 1;
    } 
    return 0;
}


void tokeize(char newSource[], int sourceLength, token tokenArray[], int *size){

    char buffer[100];                    // temporary string 
    int bufferIdx = 0;                  // current size of buffer
    token tokenArr[sourceLength];       // token array
    int tokenIdx = 0;                   //current size of token

    // printf("%d", sourceLength);

    printf("\nLexeme Table:\n\n");
    
    printf("Lexeme\t\tToken Type\n");

    //go through the file
    int currentIndex = 0;
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

            //odd
            else if(strcmp(buffer, reservedWords[14]) == 0){
                strcpy(tokenArr[tokenIdx].type, "odd");
                tokenArr[tokenIdx].token = oddsym;
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

            printf("%s\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);            
                     
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

            printf("%d\t\t%d\n", tokenArr[tokenIdx].val, tokenArr[tokenIdx].token);
            tokenIdx++;
            bufferIdx = 0;
            currentIndex--; 

        
        }

        //check if the current char is a special character
        else if(isSpecialSymbol(newSource[currentIndex]) != 0){

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

            printf("%s\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);
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

    // printing Lexeme List
    printf("\nLexeme List:\n");

    currentIndex = 0;
 
    while(currentIndex < tokenIdx){
        printf("%d ", tokenArr[currentIndex].token);

        if(tokenArr[currentIndex].token == 3){
            printf("%d ", tokenArr[currentIndex].val);
        }
        if(tokenArr[currentIndex].token == 2){
            printf("%s ", tokenArr[currentIndex].type);
        }    
              
        currentIndex++;
    }

    //pass to new array
    printf("\n");
    while((*size) < tokenIdx){
        tokenArray[(*size)].token = tokenArr[(*size)].token;
        tokenArray[(*size)].val = tokenArr[(*size)].val;
        strcpy(tokenArray[(*size)].type, tokenArr[(*size)].type);

        (*size)++;
    }

}



//PARSER AND CODE GENERATOR --------------------------------
//add token into symbol table
void symbol_table_add(int kind_of_token ,char name_of_token[],int val_of_token, int level_of_token, int addr_of_token){
    assembly[table_index].kind = kind_of_token;
    strcpy(assembly[table_index].name,name_of_token);
    assembly[table_index].val = val_of_token;
    assembly[table_index].level = level_of_token;
    assembly[table_index].addr = addr_of_token;
    assembly[table_index].mark = 0;  //initilize equal = 0 for now

    table_index++;
}


//return idx if found, otherwise return -1
int tableCheck(char string[], int string_kind){
    // printf("\n symbol table size; %d\n", table_index);

    int symbol_index = table_index - 1;

    //linear search through the symbol table looking at name
    while(symbol_index != 0){
        // printf("\n %s\t\t %d", assembly[symbol_index].name, assembly[symbol_index].kind);
        if((strcmp(assembly[symbol_index].name, string) == 0) && assembly[symbol_index].kind == string_kind){
            // printf("\n return symbol index value %d\n", symbol_index);
            return symbol_index;
        }
        symbol_index--;
    }

    return 0;
}


//go through symbol table and print 
void symbol_table_print(){
    int i = 1;

    printf("\nSymbol Table: ");
    printf("\nKind\t\tName\t\tValue\t\tLevel\t\tAddress\t\tMark");
    while(i < table_index){
        printf("\n%d\t\t%s\t\t%d\t\t%d\t\t%d\t\t%d",assembly[i].kind, assembly[i].name, 
        assembly[i].val, assembly[i].level, 
        assembly[i].addr, assembly[i].mark);
        i++;
    }
}


void emit(int op, int l, int m){
    if(codeIndex > CODE_SIZE){
        printf("Error: Code index exceeds code max size");
        exit(1);
    }
    else{
        codeStack[codeIndex].op = op;
        codeStack[codeIndex].l = l;
        codeStack[codeIndex].m = m;
        codeIndex++;
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
            //if (tableCheck(tokenArray[idx)) != -1)   //NOTE: ASK PROF HERE
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
            if(table_index == 0 || tableCheck(indentName,1) == 0) {
                symbol_table_add(1,indentName,tokenArray[idx].val,0,0);
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


//return numbers of variables
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

            //if (tableCheck(tokenArray[idx)) != -1)   //NOTE: ASK PROF HERE
                //printf('Error: ');
                //exit(0);

            //if table is empty or it is not in the list, add it
            //otherwise it is error
            if(table_index == 0 || tableCheck(tokenArray[idx].type,2) == 0){ 
                symbol_table_add(2, tokenArray[idx].type, 0,0,numVars + 2);  
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


//FIXME: maybe there is something wrong with updated pseudocode
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

//Note: next token is updated
void factor(token tokenArray[]){
    if(tokenArray[idx].token == identsym){
        /*Debug*/
        // printf("\nCurrent token at factor at identisym: %s\n", tokenArray[idx].type);
        
        int symIdx = 0;

        //check for undeclared
        for(int i = table_index - 1; i > 0 ; i--){
            if(strcmp(tokenArray[idx].type, assembly[i].name) == 0){
                symIdx = i;
            }
        }

        if(symIdx == 0){
            // printf("Bug is location: a");
            printf("\nError: Undeclared identifier");
            exit(1);
        }

        //var or const
        if(assembly[symIdx].kind == 1){         
            //emit LID(M = table[symIdx].value), opcode = 1
            emit(1 , 0, assembly[symIdx].val); 
        }
        else if(assembly[symIdx].kind == 2){
            //emit LOD (M = table[symIdx].address), opcode = 3
            emit(3 , 0, assembly[symIdx].addr);
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


//FIXME: check with TA for sure
void statement(token tokenArray[]){
    if(tokenArray[idx].token == identsym){
        /*Debug*/
        // printf("\nCurrent identsym token is %s\n", tokenArray[idx].type);

        int symIdx = 0;
        
        //check if identifier has been declared
        for(int i = table_index - 1; i > 0; i--){
            if(strcmp(tokenArray[idx].type, assembly[i].name) == 0){  //found
                if(assembly[i].kind == 1){ //constant
                    
                    /*Debugging*/
                    printf("\nBug is at location :b");
                    for(int j = idx; j >= 0; j--){
                        printf("\nCurrent index of token array is %d\n", j);
                        printf("Current symbol is %s\n", tokenArray[j].type);
                    }
                    printf("\nCurrent index of token array is %d\n", idx);
                    printf("Current symbol is %s\n", assembly[i].name);


                    printf("\nError: Symbol cannot be a constant in statment");
                    exit(1);
                }
                else if(assembly[i].kind == 2){ //var
                    symIdx = i;
                }
            }
        }
        

        if(assembly[symIdx].kind != 2){
           printf("\nError: Only variable values may be altered");
           exit(1); 
        }

        //get the next token
        idx++;
        /*Debug*/
        // printf("\nCurrent identsym token is %s\n", tokenArray[idx].type);

        if(tokenArray[idx].token != becomessym){
            printf("\nError: Assignment statements must use :=");
            exit(1);
        }
        //get the next token
        idx++;
        /*Debug*/
        // printf("\nCurrent identsym token is %s\n", tokenArray[idx].type);
        expression(tokenArray);

        //emit STO(M = table[symIdx].addr)
        emit(4,0,assembly[symIdx].addr);

        return;        
    }
    else if(tokenArray[idx].token == beginsym){
        do{
            /*Debug*/
            // printf("\nCurrent identsym token is %s\n", tokenArray[idx].type);

            //get next token
            idx++;

            /*Debug*/
            // printf("\nCurrent character is %s", tokenArray[idx].type);

            statement(tokenArray);

            

            /*Debug*/
            // printf("\nit will come here after token at the end of factor\n");
        }while (tokenArray[idx].token == semicolonsym);
        
        /*Debug*/
        // printf("\nCharacter after while loop is %s", tokenArray[idx].type);
        // printf("\nSecond Character after while loop is %s", tokenArray[idx+4].type);

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
        codeStack[jpc_idx].m = codeIndex;
        return; 
    }
    else if(tokenArray[idx].token == whilesym){
        /*Debug*/
        // printf("\nIt is at while");

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
        codeStack[jpc_idx].m = codeIndex;
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
            if(strcmp(tokenArray[idx].type, assembly[i].name) == 0){ //found
                symIdx = i;
            }
        }

        if(symIdx == 0){
            printf("\nError: Undeclared identifier");
            exit(1);
        }
        //if it is not a var
        if(assembly[symIdx].kind != 2){
            printf("\nError: Only variable values may be altered");
            exit(1);
        }

        //get next token
        idx++;

        //emit READ, OPR = 9 SYS, READ = 2 
        emit(9,0,2);

        //emit STO(M = table[symIdx].addr)
        emit(4,0,assembly[symIdx].addr);

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
        /*Debug*/
        // printf("\nCurrent token at xorsym: %s\n", tokenArray[idx].type);
        
        //get next token
        idx++;

        /*Debug*/
        // printf("\nCurrent token at xorsym: %s\n", tokenArray[idx].type);

        condition(tokenArray);
        int jpcIdx = codeIndex;
        //emit JPC, OPR = 7, M = 0 for now
        emit(7,0,0);
        if(tokenArray[idx].token != thensym){
            printf("\nError: XOR must be followed by then");
            exit(1);
        }

        //get next token
        idx++;

        /*Debug*/
        // printf("\nCurrent token at z: %s\n",tokenArray[idx].type);

        statement(tokenArray);

        /*Debug*/
        // printf("\nCurrent token at ;: %s\n",tokenArray[idx].type);

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
        codeStack[jpcIdx].m = codeIndex;

        /*Debug*/
        // printf("\nThe token at the end of xor is : %s\n",tokenArray[idx].type);

        

    }

}


//FIXME: check with TA: is the while loop ok
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


void program(token tokenArray[], int size){
    
    block(tokenArray, size);

    if(strcmp(tokenArray[idx-1].type, ".") != 0){
        printf("Error: program must end with period \n");   
        exit(0);
    } 
    printf("\nNo errors, program is grammarly correct");

    //emit end of program: SYS 03
    emit(9,0,3);

    
}


//Print assymbly code
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


void print(){
    printf("\nAssembly Code");
    printf("\nLine\t\tOP\t\tL\t\tM");
    int line = 0;

    for(int i = 0; i < codeIndex; i++){
        char string[100];
        /*Debug*/
        // printf("\nCurrent opcode is %d\n", codeStack[i].op);

        if (codeStack[i].op == 2)
            typeOPR(string, codeStack[i].m);
        else 
        typeOpCode(string,codeStack[i].op);

        printf("\n%d\t\t%s\t\t%d\t\t%d ",line,string,codeStack[i].l,codeStack[i].m);
        line++;
    }
}
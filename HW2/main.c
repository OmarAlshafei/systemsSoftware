// Hung Tran and Omar Alshafei
// COP-3402 Summer 2023
// HW2 Lexical Analyzer

/* FIXME
        - check if code can handle identifier start with number 
DONE    - print Lexeme in list form
DONE    - add code to ignore comments
        - fprintf
        - test edge cases and error handling
*/


// pre-processor directives
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


// constants
#define IDENT_MAX 11
#define NUM_MAX 5


// token values
typedef enum {
skipsym = 1, identsym, numbersym, plussym, minussym,
multsym, slashsym, xorsym, eqsym, neqsym, lessym, leqsym,
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomessym, beginsym, endsym, ifsym, thensym,
whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
readsym , elsesym} token_type;


// token struct 
typedef struct token {
    int token;   
    int  val;   
    char type[15];  
}token;


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
    char reservedWords[14][20] = {"const", "var", "procedure", "call", "begin", "end", "if", "then"
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
    

    fp = fopen("output.txt", "w");
    
    printf("Source Program:\n%s\n\n", inputStr);
    fprintf(fp, "Source Program:\n%s\n\n", inputStr);
    
    printf("Lexeme Table:\n\n");
    fprintf(fp, "Lexeme Table:\n\n");
    
    printf("lexeme\t\ttoken type\n");
    fprintf(fp, "lexeme\t\ttoken type\n");

    // run through the input by char to determine the token 
    cur = 0; 
    while(cur < len){

        // check if current char is a white space
        if(isspace(inputStr[cur]) != 0){
            cur++; 
            continue;
        }
        
        else if (inputStr[cur] == '/' && inputStr[cur + 1] == '/' ){
            while(inputStr[cur] != '\n')
                cur++;
        }
        
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
                    fprintf(fp, "Error: Identifier names cannot exceed 11 characters\n");                    
                    cur++;
                    continue;
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

            // identifier 
            else{
                strcpy(tokenArr[tokenIdx].type, buffer);
                tokenArr[tokenIdx].token = identsym; 
            }

            printf("%s\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);
            if(strcmp(tokenArr[tokenIdx].type, "begin") == 0)
                fprintf(fp, "%s\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);   
            else
                fprintf(fp, "%s\t\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);            
            
                     
            tokenIdx++;
            bufferIdx = 0;
            cur--;
        }

        // check if current char is a digit
        else if(isdigit(inputStr[cur]) != 0 && isspace(inputStr[cur]) == 0){

            while(isdigit(inputStr[cur]) != 0 && isspace(inputStr[cur]) == 0){

                // check if digit is over 5 char, error handling
                if(bufferIdx >= NUM_MAX){
                    printf("Error: Numbers cannot exceed 5 characters\n");
                    fprintf(fp, "Error: Numbers cannot exceed 5 characters\n");                    
                    cur++;
                    continue;
                }

                else{
                    buffer[bufferIdx++] = inputStr[cur++];
                }

            }

            buffer[bufferIdx] = '\0';

            // store info into array
            strcpy(tokenArr[tokenIdx].type, "3");
            tokenArr[tokenIdx].val = atoi(buffer);
            tokenArr[tokenIdx].token = numbersym;

            printf("%d\t\t%d\n", tokenArr[tokenIdx].val, tokenArr[tokenIdx].token);
            fprintf(fp, "%d\t\t\t%d\n", tokenArr[tokenIdx].val, tokenArr[tokenIdx].token);            
            tokenIdx++;
            bufferIdx = 0;
            cur--; 
        }

        
        // check if current char is a special symbol
        else if(isSpecialSymbol(inputStr[cur]) != -1){

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
                strcpy(tokenArr[tokenIdx].type, "()");
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
                strcpy(tokenArr[tokenIdx].type, "<");
                tokenArr[tokenIdx].token = lessym; 
            }

            // >
            else if(inputStr[cur] == '>'){
                strcpy(tokenArr[tokenIdx].type, ">");
                tokenArr[tokenIdx].token = gtrsym; 
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

            printf("%s\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);
            fprintf(fp, "%s\t\t\t%d\n", tokenArr[tokenIdx].type, tokenArr[tokenIdx].token);            
            tokenIdx++;
            bufferIdx = 0;

        }
        

        else{
            printf("Error: Invalid Symbol\n");
            fprintf(fp, "Error: Invalid Symbol\n");            
            cur++;
            continue;
        }

    cur++;
    }
    
    printf("\nLexeme List:\n");
    fprintf(fp, "\nLexeme List:\n");
    
    tokenIdx = 0;
    cur = 0;
    while(cur < len){
        printf("%d ", tokenArr[tokenIdx].token);
        fprintf(fp, "%d ", tokenArr[tokenIdx].token);        
        if(tokenArr[tokenIdx].token == 2){
            printf("%s ", tokenArr[tokenIdx].type);
            fprintf(fp, "%s ", tokenArr[tokenIdx].type);
        }    
        else if(tokenArr[tokenIdx].token == 19){
            printf("\n");
            fprintf(fp, "\n");
            break;
        }                
        cur++;
        tokenIdx++;
    }
    

}

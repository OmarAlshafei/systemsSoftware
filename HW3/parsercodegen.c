// Omar Alshafei and Hung Tran
// Hw 3

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CODE_LENGTH 500

// constants
#define IDENT_MAX 11
#define NUM_MAX 5

typedef struct instruction {
	int op;
	int l;
	int m;
} instruction;

instruction *tokenVar;

// token values
typedef enum {
oddsym = 1, identsym, numbersym, plussym, minussym,
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
    char reservedWords[14][20] = {"const", "var", "call", "begin", "end", "if", "then",
                                  "xor", "while", "do", "read", "write"};
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
	/*
    // printing Token List
    for(int i = 0; i < tokenIdx; i++){

		strcat(, (char)tokenArr[i].token);

        if(tokenArr[i].token == 3){
			strcat(, (char)tokenArr[i].val);
        }
        if(tokenArr[i].token == 2){
			strcat(, (char)tokenArr[i].type);
        }          
    }

	printf("%s", );

	int lineNum = 0;
	// char opCode =
	// int lexLevel =
	// int opNum = 

	printf("Line\t\tOP\t\tL\t\tM");
	
	while ( != null){
		printf("%d\t\t%s\t\t%d\t\t%d" lineNum);		
		lineNum++;
		
	}



    fclose(fp);
    free(inputStr);
	free();
	*/
}
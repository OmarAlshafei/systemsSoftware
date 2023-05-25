// Omar Alshafei HW1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 500

int pas[ARRAY_SIZE];

int base(int BP, int L) {
    int arb = BP; // arb = activation record base
    while ( L > 0) { //find base L levels down
        arb = pas[arb];
        L--;
    }
return arb;
}

int main(int argc, char *argv[]) {

    FILE *fp = fopen(argv[1], "r");
        
    for (int i = 0; i < ARRAY_SIZE; i++)
        pas[i] = 0;
        
    int x = 0;
    while (fscanf(fp,"%d", &(pas[x])) != EOF)
        x += 1;
    
    int BP = x;
    int SP = x - 1;
    int PC = 0;  

    printf("                   PC   BP   SP   stack\n");
    printf("Initial values:    %d    %d   %d\n\n", PC, BP, SP);
    
    int op;
    int L;
    int M;
    char opName[4];
    int barCount = 0;
    int bar[x/3];
    int halt = 1;
    
    while (halt != 0){
    
    op = pas[PC];
    L = pas[PC + 1];
    M = pas[PC + 2];
    PC += 3;
    
    switch(op) {

    case 1:
        SP += 1;
        pas[SP] = M;
        strcpy(opName, "LIT");
        break;

    case 2:
        switch(M){
            case 0:
                SP = BP - 1;
                BP = pas[SP + 2];
                PC = pas[SP + 3];
                strcpy(opName, "LIT");        
                break;    

            case 1:
                pas[SP-1] = pas[SP-1] + pas[SP];
                SP = SP - 1;
                strcpy(opName, "ADD");        
                break;    

            case 2:
                pas[SP-1] = pas[SP-1] - pas[SP];
                SP = SP - 1;
                strcpy(opName, "SUB");        
                break;   
                 
            case 3:
                pas[SP-1] = pas[SP-1] * pas[SP];
                SP = SP - 1;            
                strcpy(opName, "MUL");        
                break;    

            case 4:
                pas[SP-1] = pas[SP-1] / pas[SP];
                SP = SP - 1;            
                strcpy(opName, "DIV");        
                break;    

            case 5:
                pas[SP-1] = pas[SP-1] == pas[SP];
                SP = SP - 1;            
                strcpy(opName, "EQL");        
                break;    

            case 6:
                pas[SP-1] = pas[SP-1] != pas[SP];
                SP = SP - 1;            
                strcpy(opName, "NEQ");        
                break;    

            case 7:
                pas[SP-1] = pas[SP-1] < pas[SP];
                SP = SP - 1;                       
                strcpy(opName, "LSS");        
                break; 
                   
            case 8:
                pas[SP-1] = pas[SP-1] <= pas[SP];
                SP = SP - 1;                                   
                strcpy(opName, "LEQ");        
                break;    

            case 9:
                pas[SP-1] = pas[SP-1] > pas[SP];
                SP = SP - 1;                                   
                strcpy(opName, "GTR");        
                break;    

            case 10:
                pas[SP-1] = pas[SP-1] >= pas[SP];
                SP = SP - 1;                                   
                strcpy(opName, "GEQ");        
                break;
                
            default :
                printf("ERROR");
        }
        break;
    
    case 3:
        SP += 1;
        pas[SP] = pas[base(BP, L) + M];
        strcpy(opName, "LOD");
        break;  

    case 4:
        pas[base(BP, L) + M] = pas[SP];
        SP -= 1;
        strcpy(opName, "STO");
        break;      
	
    case 5:
        pas[SP + 1] = base(BP, L);
        pas[SP + 2] = BP;
        pas[SP + 3] = PC;
        BP = SP + 1;
        PC = M;
        bar[barCount] = BP;
        barCount++;
        strcpy(opName, "CAL");
        break;      
        
    case 6:
        SP += M;
        strcpy(opName, "INC");
        break;              
        
    case 7:
        PC = M;
        strcpy(opName, "JMP");
        break;              

    case 8:
        if (pas[SP] == 0){
        PC = M;
        }
        SP -= 1;
        strcpy(opName, "JPC");
        break;              

    case 9:
        if (M == 1){
            printf("Output result is: %d\n", pas[SP]);
            SP -= 1;
        }
        if (M == 2){
            SP += 1;
            printf("Please Enter an Integer: ");
            scanf("%d", &pas[SP]);
        }
        if (M == 3){
            halt = 0;
        }
        strcpy(opName, "SYS");
        break;
        
   default :
       printf("ERROR");
    }
   
    printf("          %s  %d  %2d  %2d  %d  %d  ", opName, L, M, PC, BP, SP);

    for (int i = x; i <= SP; i++){
        for (int j = 0; j <= barCount; j++){
            if (bar[j] == i)
                printf("|  ");
        }
            printf("%d  ", pas[i]);        
    }
    printf("\n");
    }    
    return 0;
}
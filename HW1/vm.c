// Omar Alshafei HW1

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 500
int pas[ARRAY_SIZE];

int base(int BP, int L)
{
int arb = BP; // arb = activation record base
while ( L > 0) //find base L levels down
{
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
    printf("Initial values:    %d    %d   %d\n", PC, BP, SP);
    
    int OP;
    int L;
    int M;
    char OPname[3];
    int halt = 1;
    while (halt != 0){
    
    OP = pas[PC];
    L = pas[PC + 1];
    M = pas[PC + 2];
    PC += 3;
    
    switch(OP) {

    case 1:
        SP += 1;
        M = pas[SP];
        printf("LIT %d  %d  %d  %d", L, M, PC, BP, SP);
        break;

    case 2:
        switch(M){
            case 0:
                SP = BP - 1
                BP = pas(SP + 2)
                break;    

            case 1:

                break;    

            case 2:

                break;    

            case 3:

                break;    

            case 4:

                break;    

            case 5:

                break;    

            case 6:

                break;    

            case 7:

                break;    
            case 8:

                break;    

            case 9:

                break;    

            case 10:

                break;

            
        }
        break;
    
    case 3:
        SP += 1;
        pas[SP] = pas[base(BP, L) + M];
        printf("LOD %d  %d  %d  %d", L, M, PC, BP, SP);
        break;  

    case 4:
        pas[base(BP, L) + M] = pas[SP];
        SP -= 1;
        printf("STO %d  %d  %d  %d", L, M, PC, BP, SP);
        break;      
	
    case 5:
        pas[SP + 1] = base(BP, L);
        pas[SP + 2] = BP;
        pas[SP + 3] = PC;
        BP = SP + 1;
        SP += M;
        printf("CAL %d  %d  %d  %d", L, M, PC, BP, SP);
        break;      
        
    case 6:
        SP += M;
        printf("INC %d  %d  %d  %d", L, M, PC, BP, SP);
        break;              
        
    case 7:
        PC = M;
        printf("JMP %d  %d  %d  %d", L, M, PC, BP, SP);
        break;              

    case 8:
        if (pas[SP] == 0){
        PC = M;
        }
        SP = SP-1;
        printf("JPC %d  %d  %d  %d", L, M, PC, BP, SP);
        break;              

    case 9:
        if (M = 1){
            printf("Output result is: %d", pas[SP]);
            SP -= 1;
        }
        if (M = 1){
            SP += 1;
            scanf("Please Enter an Integer: ")
        }
        if (M = 1){
            halt = 0;
        }    
        break;
          
   default :
   
    for (int i = x; i <= SP; i++){
        for (int j = 0; j <= k; j++)
            if (i == bar[j])
                printf("| ");
            printf("%d ", pas[i]);
    }

    printf("\n");
  }
}
    }

}
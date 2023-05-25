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
        
         
    int OP;
    int L;
    int M;
        
    int x = 0;
    while (fscanf(fp,"%d %d %d", &(pas[x]), &(pas[x+1]), &(pas[x+2])) != EOF)
        x += 3;
    
    int BP = x;
    int SP = x - 1;
    int PC = 0;  

    printf("                    PC   BP   SP   stack\n");
    printf("Initial values:     %d    %d   %d\n", PC, BP, SP);
}
// Omar Alshafei
// 5/24/2023
// HW1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constant max array size
#define ARRAY_SIZE 500
// declare global array that stores the input
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
    // open input file
    FILE *fp = fopen(argv[1], "r");
    // initalize array    
    for (int i = 0; i < ARRAY_SIZE; i++)
        pas[i] = 0;
    // scan input into array and calculate BP
    int ogBP = 0;
    while (fscanf(fp,"%d", &(pas[ogBP])) != EOF)
        ogBP += 1;
    // define base pointer, stack pointer, and program counter
    int BP = ogBP;
    int SP = ogBP - 1;
    int PC = 0;  
    // print initial values
    printf("\t\t\tPC\tBP\tSP\tstack\n");
    printf("Initial values:\t\t%d\t%d\t%d\n\n", PC, BP, SP);
    
    // define char array to store OP name
    char opName[4];
    // define bar array and bar count variable to track bar location and quantity
    int barCount = 0;
    int bar[ogBP/3];
    // loop to determine when program is halted
    int halt = 1;
    while (halt != 0){
        // define OP, L, and M values
        int OP = pas[PC];
        int L = pas[PC + 1];
        int M = pas[PC + 2];
        // increment program counter by 3
        PC += 3;
        
        // switch used for instuctions
        switch(OP) {
            // literal instruction
            case 1:
                SP += 1;
                pas[SP] = M;
                strcpy(opName, "LIT");
                break;
            // operation instruction
            case 2:
                switch(M){
                    // return
                    case 0:
                        SP = BP - 1;
                        BP = pas[SP + 2];
                        PC = pas[SP + 3];
                        strcpy(opName, "RTN");
                        break;
                    // add
                    case 1:
                        pas[SP-1] = pas[SP-1] + pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "ADD");
                        break;
                    //subtract
                    case 2:
                        pas[SP-1] = pas[SP-1] - pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "SUB");
                        break;
                    // multiply
                    case 3:
                        pas[SP-1] = pas[SP-1] * pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "MUL");
                        break;
                    // divide
                    case 4:
                        pas[SP-1] = pas[SP-1] / pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "DIV");
                        break;
                    // equal
                    case 5:
                        pas[SP-1] = pas[SP-1] == pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "EQL");
                        break;
                    // not equal
                    case 6:
                        pas[SP-1] = pas[SP-1] != pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "NEQ");
                        break;
                    // less than
                    case 7:
                        pas[SP-1] = pas[SP-1] < pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "LSS");
                        break;
                    // less than or equal to
                    case 8:
                        pas[SP-1] = pas[SP-1] <= pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "LEQ");
                        break;
                    // greater than
                    case 9:
                        pas[SP-1] = pas[SP-1] > pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "GTR");
                        break;
                    // greater than or equal to
                    case 10:
                        pas[SP-1] = pas[SP-1] >= pas[SP];
                        SP = SP - 1;
                        strcpy(opName, "GEQ");
                        break;
                    // default case
                    default :
                        printf("ERROR");
                }
                break;
            // load instruction
            case 3:
                SP += 1;
                pas[SP] = pas[base(BP, L) + M];
                strcpy(opName, "LOD");
                break;
            // store instruction
            case 4:
                pas[base(BP, L) + M] = pas[SP];
                SP -= 1;
                strcpy(opName, "STO");
                break;
            // call instruction
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
            // increment intructrion
            case 6:
                SP += M;
                strcpy(opName, "INC");
                break;
            // jump instruction
            case 7:
                PC = M;
                strcpy(opName, "JMP");
                break;
            // conditional jump instruction
            case 8:
                if (pas[SP] == 0){
                PC = M;
                }
                SP -= 1;
                strcpy(opName, "JPC");
                break;
            // SYS intructions
            case 9:
                // write to the screen
                if (M == 1){
                    printf("Output result is: %d\n", pas[SP]);
                    SP -= 1;
                }
                // read in input
                if (M == 2){
                    SP += 1;
                    printf("Please Enter an Integer: ");
                    scanf("%d", &pas[SP]);
                }
                // set halt to zero and end program
                if (M == 3){
                    halt = 0;
                }
                strcpy(opName, "SYS");
                break;
            // default case
            default:
                printf("ERROR");
        }
        // print operation name, L, M, program counter, base pointer, and stack pointer
        printf("\t%s  %d\t%d\t%d\t%d\t%d\t", opName, L, M, PC, BP, SP);
            // for loop used to traverse through activation record
        for (int i = ogBP; i <= SP; i++){
            // inner loop to locate bar
            for (int j = 0; j <= barCount; j++){
                // print bar if it is found
                if (bar[j] == i)
                    printf("| ");
            }
            // print array value at position i
            printf("%d ", pas[i]);
        }
        printf("\n");
    }    
    // close file pointer
    fclose(fp);
    return 0;
}
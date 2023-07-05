// Omar Alshafei and Hung Tran
// Hw 3

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CODE_LENGTH 500

typedef struct symbol
{
	int kind;
	char name[12];
	int val;
	int level;
	int addr;
}symbol;

typedef struct instruction
{
	int op;
	int l;
	int m;
} instruction;

// token type for parser
typedef enum
{
	nulsym = 1, identsym, numbersym, plussym, minussym,
	multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
	gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
	periodsym, becomessym, beginsym, endsym, ifsym, thensym,
	whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
	readsym , elsesym
} token_type;
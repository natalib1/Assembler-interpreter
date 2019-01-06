#ifndef BINARYCOMPILER_INCLUDED
#define BINARYCOMPILER_INCLUDED

#include "parser.h"

/*In case that both parameters are of addressing type special index (with offset)
  Each parameter requires two additional bytes, giving a total of 4 bytes.*/
#define MAX_PARAMETER_WORDS 4

/*Max parameters size + 1 is the max action instruction size */
#define MAX_ACTION_WORDS (MAX_PARAMETER_WORDS + 1)

/*Represents the first word in each action instruction*/
typedef struct
{
	int combination;
	int registerDestination;
	int addressingDestination;
	int registerSource;
	int addressingSource;
	int opcode;
	int type;
	int dbl;
} MachineCommandWord;

/*Numerically represents an action statement*/
typedef struct MachineInstruction
{
	MachineCommandWord opcode;
	long parameters[MAX_PARAMETER_WORDS];
	int length; /*The length of parameters*/
} MachineInstruction;

/*Creates the actual words to be rendered; Returns how many words or -1 on failure*/
int CreateBinaryWords(ActionInstructionDescription statement, unsigned long *fill);

/* Calculates the length of the assembled Instruction */
int CalculateInstructionWordLength(Instruction instruction);

/*output must be of size 8 atleast
minSize will be implemented by adding zeros*/
void ToBaseEightMinSize(unsigned long value, int minSize, char *output);

/*Just generates the string in base six*/
void ToBaseEight(unsigned long value, char *output);

#endif
/* BINARYCOMPILER_INCLUDED */

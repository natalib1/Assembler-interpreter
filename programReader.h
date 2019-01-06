#ifndef PROGRAMREADER_INCLUDED
#define PROGRAMREADER_INCLUDED
#include "parser.h"

/*The initial size of the program*/
#define PROGRAM_STARTING_SIZE 10

/*The growing modificator of the array*/
#define PROGRAM_DYNAMIC_FACTOR 2

/* The read progarm as an expanding array of Instruction lines */
typedef struct
{
	Instruction *instructions;	/* An expanding array of read program lines */
	int currentLength;			/* The current length of the array */
	int maxLength;				/* The allocated length for the array */
} ReadProgramStruct;
typedef ReadProgramStruct *ReadProgram;

/* Frees all memory held by the given ReadProgram structure */
void* FreeProgram(ReadProgram program);

/* Create a new array of instructions to read the program into */
ReadProgram NewReadProgram(void);

/* Gets the specified instruction from the program */
/* Returns a pointer to instruction of the given index in the given program, or null on failure */
Instruction* GetInstruction(ReadProgram program, int instructionIndex);

/* Adds the given instruction to the given program. */
/* Returns a pointer to the instruction in the program if succeeded or null on failure */
Instruction* AddInstruction(ReadProgram program, Instruction *instruction);

#endif
/* PROGRAMREADER_INCLUDED */

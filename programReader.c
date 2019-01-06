#include <stdlib.h>
#include "programReader.h"

void* FreeProgram(ReadProgram program)
{
	if (program != NULL)
	{
		if (program->instructions != NULL) { free(program->instructions); }
		free(program);
	}
	return NULL;
}

ReadProgram NewReadProgram(void)
{
	/* get memory */
	ReadProgram program = (ReadProgram)malloc(sizeof(ReadProgramStruct));
	/* if allocation failed, break */
	if (program == NULL) { return NULL; }
	/* assign basic parameters */
	program->maxLength = PROGRAM_STARTING_SIZE;
	program->currentLength = 0;
	/* get memory for program instructions */
	program->instructions = (Instruction*)malloc(sizeof(Instruction) * PROGRAM_STARTING_SIZE);
	/* if allocation failed, break */
	if (program->instructions == NULL) { return FreeProgram(program); }
	return program;
}

Instruction* GetInstruction(ReadProgram program, int instructionIndex)
{
	if (instructionIndex < 0 || instructionIndex >= program->currentLength) { return NULL; }
	return &(program->instructions[instructionIndex]);
}

Instruction* AddInstruction(ReadProgram program, Instruction* instruction)
{
	/* check if read program memory is full */
	if (program->currentLength >= program->maxLength)
	{
		/* define new size */
		int newLength = program->maxLength * PROGRAM_DYNAMIC_FACTOR;
		/* get new memory for instructions */
		Instruction *newInstructions = (Instruction*)realloc(program->instructions, sizeof(Instruction) * newLength);
		/* if allocation failed, break everything */
		if (newInstructions == NULL) { return FreeProgram(program); }
		/* if all is well, reassign */
		program->instructions = newInstructions;
		program->maxLength = newLength;
	}
	/* add new instruction */
	program->instructions[program->currentLength] = *instruction;
	program->currentLength++;
	/* return a pointer to the new location */
	return &(program->instructions[program->currentLength - 1]);
}

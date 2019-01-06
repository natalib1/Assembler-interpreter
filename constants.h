#ifndef CONSTANTS_INCLUDED
#define CONSTANTS_INCLUDED
#include "parser.h"

/*STRUCTS*/
/*Legal addressing modes for opcode*/
typedef struct
{
	int sourceOperand[4]; /*The legal addressing modes for the source (first) operand*/
	int destinationOperand[4]; /*The legal addressing modes for the destination (second) operand*/
} LegalAddressingMode;

/*CONSTANTS*/
/*Opcodes. The index is the value*/
extern char *(Opcodes[]);
/*Directives. The index is the value*/
extern char *(Directives[]);
/*Registers. The index is the value*/
extern char *(Registers[]);
/*Array of all opcodes with amount of parameters*/
extern int ParametersCount[];
/*Array of all opcodes with their legal addressing modes*/
extern LegalAddressingMode LegalAddressingModes[];

/*FUNCTIONS*/
/*Returns the value of the opcode into opcode, returns ERROR_DOES_NOT_EXIST if opcode not found*/
ErrorCode FindOpcode(char *s, int startIndex, int *opcode);

/*Returns the value of the directive Or ERROR_DOES_NOT_EXIST*/
ErrorCode FindDirective(char *str, int startIndex, int *directive);

/*Returns the value of the directive Or ERROR_DOES_NOT_EXIST*/
ErrorCode FindRegister(char *str, int startIndex, int *regIndex);

/*Returns true if the opcode given has atleast 1 parameter
  Returns false if not, or opcode is invalid*/
int AtleastOneParameter(int opcode);

/*Returns true if the opcode given has 2 parameters exactly
  Returns false if not, or opcode is invalid*/
int TwoParameters(int opcode);

/*Returns 0 if its legal
  Returns 1 if the first parameter is illegal
  Returns 2 if the second paramter is illegal
  Returns 3 if both parameters are illegal*/
int IsLegalAddressingMode(ActionInstructionDescription *instruction);

#endif
/* CONSTANTS_INCLUDED */

#include <string.h>
#include "constants.h"
#include "helperFunctions.h"

char *(Opcodes[]) =
{
	"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc",
	"dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"
};

int ParametersCount[] = 
{
	2, 2, 2, 2, 1, 1, 2, 1,
	1, 1, 1, 1, 1, 1, 0, 0
};

char *(Directives[]) = { "data", "string", "entry", "extern" };

char *(Registers[]) = { "r0", "r1", "r2", "r3",	"r4", "r5", "r6", "r7" };

LegalAddressingMode LegalAddressingModes[] = 
{
	{ { 1, 1, 1, 1 }, { 0, 1, 1, 1 } }, /*mov*/
	{ { 1, 1, 1, 1 }, { 1, 1, 1, 1 } }, /*cmp*/
	{ { 1, 1, 1, 1 }, { 0, 1, 1, 1 } }, /*add*/
	{ { 1, 1, 1, 1 }, { 0, 1, 1, 1 } }, /*sub*/
	{ { 0, 0, 0, 0 }, { 0, 1, 1, 1 } }, /*not*/
	{ { 0, 0, 0, 0 }, { 0, 1, 1, 1 } }, /*clr*/
	{ { 0, 1, 1, 1 }, { 0, 1, 1, 1 } }, /*lea*/
	{ { 0, 0, 0, 0 }, { 0, 1, 1, 1 } }, /*inc*/
	{ { 0, 0, 0, 0 }, { 0, 1, 1, 1 } }, /*dec*/
	{ { 0, 0, 0, 0 }, { 0, 1, 1, 1 } }, /*jmp*/
	{ { 0, 0, 0, 0 }, { 0, 1, 1, 1 } }, /*bne*/
	{ { 0, 0, 0, 0 }, { 0, 1, 1, 1 } }, /*red*/
	{ { 0, 0, 0, 0 }, { 1, 1, 1, 1 } }, /*prn*/
	{ { 0, 0, 0, 0 }, { 0, 1, 0, 0 } }, /*jsr*/
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }, /*rts*/
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }  /*stop*/
};

/*returns true if str starts with substr, else false*/
int StartsWith(const char *str, const char *substr)
{
	int length = strlen(substr);

	if (strlen(str) < length) { return FALSE; }

	return strncmp(substr, str, length) == 0;
}

ErrorCode FindOpcode(char *s, int startIndex, int *opcode)
{
	/*Amount of opcodes*/
	int length = sizeof(Opcodes) / sizeof(char*);
	int i;

	for (i = 0; i < length; ++i)
		if (StartsWith(&(s[startIndex]), Opcodes[i])){ /*If s, from startIndex, starts with the opcode*/
				*opcode = i;
				return ERROR_NONE;
			}

	return ERROR_DOES_NOT_EXIST;
}

ErrorCode FindDirective(char *s, int startIndex, int *directive)
{
	/*Amount of directives*/
	int length = sizeof(Directives) / sizeof(char*);
	int i;

	for (i = 0; i < length; ++i)
		if (StartsWith(&(s[startIndex]), Directives[i])
			&& !IsAlphanumeric(s[startIndex + strlen(Directives[i])])){
				*directive = i;
				return ERROR_NONE;
			}

	return ERROR_DOES_NOT_EXIST;
}

ErrorCode FindRegister(char *s, int startIndex, int *regIndex)
{
	/*Amount of registers*/
	int length = sizeof(Registers) / sizeof(char*);
	int i;

	for (i = 0; i < length; ++i)
		if (StartsWith(&(s[startIndex]), Registers[i])
			&& !IsAlphanumeric(s[startIndex + strlen(Registers[i])])){
				*regIndex = i;
				return ERROR_NONE;
			}

	return ERROR_DOES_NOT_EXIST;
}

int AtleastOneParameter(int opcode)
{
	if (opcode >= 0 && opcode < sizeof(ParametersCount) / sizeof(int))
	{ return ParametersCount[opcode] >= 1; }
	return FALSE;
}

int TwoParameters(int opcode)
{
	if (opcode >= 0 && opcode < sizeof(ParametersCount) / sizeof(int))
	{ return ParametersCount[opcode] == 2; }
	return FALSE;
}

int IsLegalAddressingMode(ActionInstructionDescription *instruction)
{
	int returnValue = 0;
	/*first parameter*/
	/*If is illegal*/
	if (instruction->parametersCount == 1 &&
		!LegalAddressingModes[instruction->opcode].destinationOperand[instruction->secondAddressingType]) 
		{ returnValue += 1; }
	/*Second parameter*/
	if (instruction->parametersCount == 2)
	{
		if (!LegalAddressingModes[instruction->opcode].sourceOperand[instruction->firstAddressingType]) /*If is illegal*/
		{ returnValue += 1; }
		if (!LegalAddressingModes[instruction->opcode].destinationOperand[instruction->secondAddressingType]) /*If is illegal*/
		{ returnValue += 2; }
	}
	return returnValue;
}

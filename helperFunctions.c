#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helperFunctions.h"
#include "constants.h"

char *errors[] = { "ERROR_NONE", "ERROR_UNKNOWN", "ERROR_BAD_PARAMETERS",
               "ERROR_IO", "ERROR_OUT_OF_BOUNDS", "ERROR_ALLOCATION_FAILED",
               "ERROR_ALREADY_EXISTS", "ERROR_DOES_NOT_EXIST" };

unsigned long MakeTwosComplement(long value)
{
	unsigned long mask = 0x7FFFF;
	if (value >= 0) { return value; }
	return (~((-value) & mask))+(unsigned long)1;
}

/*Returns index of next non empty char, else end of string */
int SkipWhiteSpace(char *line, int startIndex)
{
	while (line[startIndex] == ' ' || line[startIndex] == '\t') { startIndex++; }
	
	return startIndex;
}

/*Returns 1 if is char, else 0*/
int IsChar(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/*Returns 1 if is digit, else 0*/
int IsDigit(char c)
{
	return (c >= '0' && c <= '9');
}

/*Returns 1 if is digit or char, else 0*/
int IsAlphanumeric(char c)
{
	return IsChar(c) || IsDigit(c);
}

/*Returns the length of the next alphanumber word*/
int NextWordsLength(char *line)
{
	int i;
	int len = strlen(line);
	for (i = 0; i < len; i++) { if (!IsAlphanumeric(line[i])) {	return i; } }
	return i;
}

/*Returns error if not a label, if it is - sets labelName and startStatement
Assuming: non-empty line
Assuming: labelName is allocated*/
ErrorCode DetectLabel(char *line, char *labelName, int *startStatement)
{
	int i, opcode, regIndex, directive;

    /*First char must be a letter NOT A LABEL*/
	if (!IsChar(line[*startStatement])) { return ERROR_NONE; }

    /*Skip alphanumerics*/
	for (i = *startStatement; IsAlphanumeric(line[i]); i++);

	if (i - *startStatement >= LABEL_MAX_LENGTH) { return PrintError(ERROR_OUT_OF_BOUNDS, line, "Label name is too large"); }

    /*If the last char isn't ':', its not a label*/
	if (line[i] != ':') { return PrintError(ERROR_BAD_PARAMETERS, line, "Label must end with ':' "); }

    /*Copies the name only*/
	strncpy(labelName, line + *startStatement, i);
	/*Terminate string*/
	labelName[i + *startStatement] = '\0';

	/* check if label is a register name */
	if (FindRegister(labelName, 0, &regIndex) != ERROR_DOES_NOT_EXIST)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Label may not be the name of a Register"); }
	/* check if label is an opcode */
	if (FindOpcode(labelName, 0, &opcode) != ERROR_DOES_NOT_EXIST)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Label may not be an Opcode"); }
	if (FindDirective(labelName, 0, &directive) != ERROR_DOES_NOT_EXIST)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Label may not be a directive."); }

	/*plus one to skip the ';' char*/
	*startStatement = SkipWhiteSpace(line, i + 1);

	return ERROR_NONE;
}

ErrorCode PrintError(ErrorCode errorCode, char* line, char* message)
{
	printf("Error %s in line: \"%s\" (%s)\n", errors[(int)errorCode], line, message);
	return errorCode;
}
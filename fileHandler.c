#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fileHandler.h"
#include "dynamicTable.h"
#include "parser.h"
#include "binaryCompiler.h"

static char currentReadFileName[FILENAME_MAX_LENGTH];
static char currentWriteFileName[FILENAME_MAX_LENGTH];
static FILE* currentReadFile;
static FILE* currentWriteFile;

/*
    Attempts to open the given file for reading
    fileName: The name of the file to read
    Returns: Appropriate error code for failure of success
*/
ErrorCode OpenFileAS(char* fileName)
{
    /* check file name is not too long for our array */
	if (strlen(fileName) > FILENAME_MAX_LENGTH - 4)
	{
		printf("FileName: '%s' name is too long and not allowed. Skipping...\n", fileName);
		return ERROR_OUT_OF_BOUNDS;
	}

	/* copy the file name to the local array */
	strcpy(currentReadFileName, fileName);
	/* make sure it's an assembly file */
	strcat(currentReadFileName, AS_EXTENSION);
	/* open file*/
	currentReadFile = fopen(currentReadFileName, "r");
	/* if file fails to open, return error */
	if (currentReadFile == NULL)
	{
		printf("Error opening file '%s'\n", currentReadFileName);
		return ERROR_IO;
	}
    return ERROR_NONE;
}

ErrorCode ReadLine(char* line)
{
	line = fgets(line, LINE_MAX_LENGTH, currentReadFile);
	if (line == NULL) { fclose(currentReadFile); return ERROR_IO; }
	return ERROR_NONE;
}

/* Opens the given file name as a file for writing with the given extension */
ErrorCode OpenWriteFile(char* fileName, char* extension)
{
	/* get the file name for entries */
	strcpy(currentWriteFileName, fileName);
	strcat(currentWriteFileName, extension);
	/* open file for writing */
	currentWriteFile = fopen(currentWriteFileName, "w");
	if (currentWriteFile == NULL)
	{ return PrintError(ERROR_IO, currentWriteFileName, "Couldn't open file for writing."); }
	return ERROR_NONE;
}

/* Writes the given table into the currently active write file */
void WriteTable(Table table)
{
	int index;
	char tmp[20] = "";

	if (currentWriteFile == NULL) { return; }

	for (index = 0; index < table->rowsAmount; index++)
	{
		ToBaseEight(table->rows[index].address, tmp);
		fprintf(currentWriteFile, "%s\t%s", table->rows[index].name, tmp);
		if (index != table->rowsAmount - 1) { fprintf(currentWriteFile, "\n"); }
	}
}

void WriteProgram(AssembledProgram assembledProgram, char *fileName)
{
	/* go over the entries first if available */
	if (assembledProgram->entriesTable->rowsAmount > 0)
	{
		/* open the entries file for writing */
		if (OpenWriteFile(fileName, ENT_EXTENSION) != ERROR_NONE) { return; }
		/* write the entries table into the entries file */
		WriteTable(assembledProgram->entriesTable);
		/* and close the file */
		fclose(currentWriteFile);
	}
	/* go over the externals next if available */
	if (assembledProgram->externalsTable->rowsAmount > 0)
	{
		/* open the externals file for writing */
		if (OpenWriteFile(fileName, EXT_EXTENSION) != ERROR_NONE) { return; }
		/* write the externals table into the externals file */
		WriteTable(assembledProgram->externalsTable);
		/* and close the file */
		fclose(currentWriteFile);
	}

	int i;
	char tmp[20];

	/* open the object file for writing */
	if (OpenWriteFile(fileName, OB_EXTENSION) != ERROR_NONE) { return; }

	unsigned long markedExtern = MakeTwosComplement(EXTERNAL_ADDRESS);
	int totalLength = assembledProgram->values->currentLength - assembledProgram->dataLength;
	fprintf(currentWriteFile, "%20s\t%20s\t%35s\t\n", "Base 8 address","Base 8 machine code","Absolute, relocatable or external");
	
	ToBaseEight(totalLength,tmp);
	fprintf(currentWriteFile, "%40s %s ","",tmp);
	ToBaseEight(assembledProgram->dataLength,tmp);
	fprintf(currentWriteFile, "%s",tmp);
	
	for (i = 0; i < assembledProgram->values->currentLength; ++i)
	{
		ToBaseEight(i+100, tmp);
		fprintf(currentWriteFile, "\n%20s\t", tmp);
		/* if the value is external, the word is 0 */
		ToBaseEightMinSize(assembledProgram->values->values[i] == markedExtern ? 0 : 
							assembledProgram->values->values[i], 7, tmp);
		fprintf(currentWriteFile, "%20s\t", tmp);
		/* if there's an associated linker data entry */
		if (i < assembledProgram->linkerData->currentLength)
		{ fprintf(currentWriteFile, "%20c", (char)assembledProgram->linkerData->values[i]); }
	}
	fclose(currentWriteFile);
}

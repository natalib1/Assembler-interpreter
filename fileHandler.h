#ifndef FILEHANDLER_INCLUDED
#include "helperFunctions.h"
#include "assembler.h"
#define FILEHANDLER_INCLUDED

#define FILENAME_MAX_LENGTH 100

/* File Extension */
#define AS_EXTENSION ".as"
#define OB_EXTENSION ".ob"
#define ENT_EXTENSION ".ent"
#define EXT_EXTENSION ".ext"

/*
    Attempts to open the given file for reading
    fileName: The name of the file to read
    Returns: Appropriate error code for failure of success
*/
ErrorCode OpenFileAS(char* fileName);

/*
	Get a new line from the currently open file
	line: the character array to read the line into
	Returns: Appropriate error code for failure of success
*/
ErrorCode ReadLine(char* line);

/*
	Writes the given program to files of the given name
	assembledProgram: a compiled assembly program, output of the assembler.AssembleFile function
	fileName: the name of the file to write to
	Returns: Appropriate error code for failure of success
*/
void WriteProgram(AssembledProgram assembledProgram, char *fileName);

#endif 
/* FILEHANDLER_INCLUDED */

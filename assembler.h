#ifndef ASSEMBLER_INCLUDED
#define ASSEMBLER_INCLUDED

#include "helperFunctions.h"
#include "programReader.h"
#include "dynamicArray.h"
#include "dynamicTable.h"

/*Internally used*/
#define EXTERNAL_ADDRESS -666

/* Data collected on the currently read program */
typedef struct ReadProgramDataStruct
{
	Table labelsTable;		/* The read labels table */
	Table externalsTable;	/* The read externals table */
	ReadProgram program;		/* The read program */
} ReadProgramData;

/* Final assembler output */
typedef struct 
{
	Table entriesTable;			/* The final entries table */
	Table externalsTable;		/* The final externals table */
	DynamicArray values;		/* The binary values to write to disk */
	DynamicArray linkerData;
	ErrorCode success;			/* Whether the assembly went ok */
	int dataLength;
} AssembledProgramStruct;
typedef AssembledProgramStruct* AssembledProgram;

/* Entry point to the Assembler
	Renders all files through the Assembler. */
void AssembleFiles(char **files, int filesCount);

#endif
/* ASSEMBLER_INCLUDED */

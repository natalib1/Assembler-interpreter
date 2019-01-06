#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "fileHandler.h"
#include "binaryCompiler.h"

/* Frees all the data structures contained in the ReadProgramData structure */
void* FreeReadProgramData(ReadProgramData *values)
{
	FreeProgram(values->program);
	FreeTable(values->externalsTable);
	FreeTable(values->labelsTable);

	return NULL;
}

/* Creates a new ReadProgramData structure */
ReadProgramData* NewReadProgramData(void)
{
	ReadProgramData *values = (ReadProgramData*)malloc(sizeof(ReadProgramData));
	if (values == NULL) { return NULL; }

	values->externalsTable = NewTable();
	if (values->externalsTable == NULL) { return FreeReadProgramData(values); }
	values->labelsTable = NewTable();
	if (values->labelsTable == NULL) { return FreeReadProgramData(values); }
	values->program = NewReadProgram();
	if (values->program == NULL) { return FreeReadProgramData(values); }
	return values;
}

/* Remove unnecessary characters tacked on to a read line */
void RemoveLineBreaks(char *line)
{
	/* get line length */
	int length = strlen(line);
	int i;

	/* find a new line and terminate, save the location  */
	for (i = 0; i < length; i++) { if (line[i] == '\r' || line[i] == '\n') { line[i] = '\0'; break; } }

	for (; i < length; i++) { line[i] = '\0'; }
}

/* Add the line and label to the table */
ErrorCode AddToTable(Table table, char *labelName, long value, char *line)
{
	ErrorCode errorCode = AddRow(table, labelName, value);
	if (errorCode == ERROR_ALREADY_EXISTS)
	{ return PrintError(ERROR_ALREADY_EXISTS, line, "This row already exists in the table."); }
	else if (errorCode == ERROR_ALLOCATION_FAILED)
	{ return PrintError(ERROR_ALLOCATION_FAILED, line, "Allocation of new row failed."); }
	else if (errorCode == ERROR_OUT_OF_BOUNDS)
	{ return PrintError(ERROR_OUT_OF_BOUNDS, line, "The label name is too long."); }
	return ERROR_NONE;
}

/* Read the file, parse each instruction and fill up the ReadProgramData structure */
ErrorCode FirstPass(ReadProgramData *values)
{
	/* initialise reading parameters */
	int DataCounter = 0;
	int InstructionCounter = 100;

	/* also get ready to read a line */
	char line[LINE_MAX_LENGTH];
	int lineNumber = 0;
	ErrorCode errorCode;

	/*Read line by line*/
	while (ReadLine(line) == ERROR_NONE)
	{
		/* zero the line from the line break on */
		RemoveLineBreaks(line);
		/* prepare to read instruction */
		Instruction instruction = {{{0}}};

		/* parse the instruction */
		errorCode = ParseInstruction(line, lineNumber, &instruction);

		if (errorCode == ERROR_NONE) 
		{
			/* if it's empty or a comment, ignore it */
			if (instruction.type == EmptyInstruction || instruction.type == CommentInstruction)
			{ lineNumber++; continue; }
			/* if a label was found */
			if (instruction.labelExists)
			{
				/* check if it's an action */
				if (instruction.type == ActionInstruction)
				{
					errorCode = AddToTable(values->labelsTable, instruction.label, InstructionCounter, line);
				}
				else if (instruction.type == DirectiveInstruction && (instruction.InstructionDescription.directive.type == StringDirective || instruction.InstructionDescription.directive.type == DataDirective))
				{
					/* Counting data "backwards" so it will be added properly when combining with the instruction table*/
					errorCode = AddToTable(values->labelsTable, instruction.label, DataCounter * -1 - 1, line);
				}
				/* if allocation has failed, execution can not continue */
				if (errorCode == ERROR_ALLOCATION_FAILED) { return errorCode; }
			}

			if (instruction.type == ActionInstruction)
			{
				InstructionCounter += CalculateInstructionWordLength(instruction);
			}
			else
			{
				DataCounter += CalculateInstructionWordLength(instruction);
				DirectiveInstructionDescription *directive = &(instruction.InstructionDescription.directive);
				if (directive->type == ExternDirective)
				{
					errorCode = AddToTable(values->externalsTable, directive->Data.label.Name, 0, line);
					/* if allocation has failed, execution can not continue */
					if (errorCode == ERROR_ALLOCATION_FAILED) { return errorCode; }
				}
			}
			if (AddInstruction(values->program, &instruction) == NULL)
			{ return ERROR_ALLOCATION_FAILED; }
		}
		++lineNumber;
	}
	int i;
	/*Separation of data and code*/
	/*Adding the instructions size to the position of each data address*/
	for (i = 0; i < values->labelsTable->rowsAmount; i++)
	{
		if (values->labelsTable->rows[i].address < 0)
		/*This fixes the "backwards" count from earlier*/
		{ values->labelsTable->rows[i].address = (values->labelsTable->rows[i].address * -1) + InstructionCounter - 1; }
	}

	return errorCode;
}

/* searches the labels and externals table for the label name and updates its address */
ErrorCode FindAndUpdateLabel(ReadProgramData *values, Label *label)
{
	/* find in the labels table */
	TableRow* row = FindRow(values->labelsTable, label->Name);
	/* if not found... */
	if (row == NULL)
	{
		/* find in the externals */
		row = FindRow(values->externalsTable, label->Name);
		/* if not found, it's the wrong label */
		if (row == NULL)
		{
			return PrintError(ERROR_BAD_PARAMETERS, label->Name, "Label not found in labels or externals table.");
		}
		/* if found in the externals, mark it */
		label->Address = EXTERNAL_ADDRESS;
	}
	/* if found in labels, save the address */
	else { label->Address = row->address; }
	
	return ERROR_NONE;
}

/* find the instruction in the given program that has the given label and return the length of that instruction*/
long FindCommandLengthAtLabel(ReadProgramData *values, Label *label)
{
	int i;
	for (i = 0; i < values->program->currentLength; i++)
	{
		if (values->program->instructions[i].labelExists &&
			!strcmp(values->program->instructions[i].label, label->Name))
		{ return CalculateInstructionWordLength(values->program->instructions[i]); }
	}
	return -1;
}

ErrorCode UpdateParameterLabel(ReadProgramData *values, Parameter *parameter)
{
	Label *label;
	/* if it's direct addressing */
	if (parameter->type == DirectAddressing)
	{
		label = &(parameter->Info.labelIndex); 
		FindAndUpdateLabel(values, label);
	}
	/* if it's index addressing */
	if (parameter->type == IndexAddressing)
	{ 
		label = &(parameter->Info.indexAddressing.labelIndex);
		FindAndUpdateLabel(values, label);
		/*find the offset length*/
		label = &(parameter->Info.indexAddressing.labelOffset);
		parameter->Info.indexAddressing.labelOffsetLength = FindCommandLengthAtLabel(values, label);
	}
	return ERROR_NONE;
}

/* Updates instruction referencing labels */
ErrorCode SecondPass(ReadProgramData *values)
{
	ErrorCode errorCode = ERROR_NONE;
	int i;
	/* go over all instructions */
	for (i = 0; i < values->program->currentLength; i++)
	{
		Instruction *instruction = GetInstruction(values->program, i);
		if (instruction->type == ActionInstruction)
		{
			ActionInstructionDescription *actionDescription = &(instruction->InstructionDescription.action);
			/* if found one parameter, it's saved to the second parameter because it's the destination */
			if (actionDescription->parametersCount == 1)
			{ errorCode = UpdateParameterLabel(values, &(actionDescription->secondParameter));	}
			/* if found two parameters, update them both */
			if (actionDescription->parametersCount == 2)
			{
				errorCode = UpdateParameterLabel(values, &(actionDescription->firstParameter));
				if (errorCode == ERROR_NONE) { errorCode = UpdateParameterLabel(values, &(actionDescription->secondParameter)); }
			}
		}
	}

	return errorCode;
}

void* FreeAssembledProgram(AssembledProgram assembledProgram)
{
	if (assembledProgram == NULL) { return NULL; }

	if (assembledProgram->entriesTable != NULL) { FreeTable(assembledProgram->entriesTable); }
	if (assembledProgram->externalsTable != NULL) { FreeTable(assembledProgram->externalsTable); }

	if (assembledProgram->values != NULL) 
	{ free(assembledProgram->values->values); free(assembledProgram->values); }
	if (assembledProgram->linkerData != NULL)
	{ free(assembledProgram->linkerData->values); free(assembledProgram->linkerData); }

	return NULL;
}

AssembledProgram NewAssembledProgram(void)
{
	AssembledProgram assembledProgram = (AssembledProgram)malloc(sizeof(AssembledProgramStruct));
	if (assembledProgram == NULL) { return NULL; }

	assembledProgram->entriesTable = NewTable();
	if (assembledProgram->entriesTable == NULL) { return FreeAssembledProgram(assembledProgram); }
	assembledProgram->externalsTable = NewTable(); 
	if (assembledProgram->externalsTable == NULL) { return FreeAssembledProgram(assembledProgram); }
	assembledProgram->externalsTable->allowDuplicates = 1;

	assembledProgram->values = CreateArray(); 
	if (assembledProgram->values == NULL) { return FreeAssembledProgram(assembledProgram); }
	assembledProgram->linkerData = CreateArray();
	if (assembledProgram->linkerData == NULL) { return FreeAssembledProgram(assembledProgram); }

	assembledProgram->success = ERROR_NONE;

	return assembledProgram;
}

/*Writes the data to dataSegment for .string and .data
  Adds the appropriate values for .entry*/
ErrorCode SaveDirectives(ReadProgramData *values, AssembledProgram out, 
						 Instruction *instruction, DynamicArray dataArea)
{
	DirectiveInstructionDescription *directive = &(instruction->InstructionDescription.directive);

	/* Add .data to the data area */
	if (directive->type == DataDirective)
	{
		if (InsertRange(dataArea, directive->Data.values.values, directive->Data.values.length) == NULL)
		{ return 
			out->success = 
				PrintError(ERROR_ALLOCATION_FAILED, "Data Area", 
						   "Array allocation failed when adding .data."); }
	}
	/* or add .string to the data area */
	else if (directive->type == StringDirective)
	{
		int j;
		int len = strlen(directive->Data.string);

		/* add the string value */
		for (j = 0; j < len; ++j)
		{
			if (Insert(dataArea, (long)directive->Data.string[j]) == NULL)
			{
				return
					out->success =
						PrintError(ERROR_ALLOCATION_FAILED, "Data Area", 
								   "Array allocation failed when adding .string.");
			}
		}
		/* and terminate */
		if (Insert(dataArea, 0) == NULL)
		{
			return
				out->success =
					PrintError(ERROR_ALLOCATION_FAILED, "Data Area", 
							   "Array allocation failed when addind terminating character.");
		}
	}
	/* or try to add an entry */
	else if (directive->type == EntryDirective)
	{
		/* find the label row */
		TableRow *row = FindRow(values->labelsTable, directive->Data.label.Name);
		if (row == NULL)
		{
			return 
				out->success = 
					PrintError(ERROR_DOES_NOT_EXIST, directive->Data.label.Name,
							   "Label not found in labels table.");
		}
		/* and add to entries */
		else
		{
			out->success = 
				AddToTable(out->entriesTable, directive->Data.label.Name, row->address, instruction->indexes.text);
		}
	}
	/* no need to handle externs, they're already taken care of */

	return ERROR_NONE;
}


/* returns linkerData into A and B (both optional)
representing the first and second parameters' values
when A = absolute, E = external, R = relocatable
according to addressing type */
ErrorCode ProcessParameter(ReadProgramData *values, AssembledProgram assembledProgram, 
					 Instruction *instruction, Parameter *parameter, DynamicArray dataArea,
					 char *A, char *B, int indexDirect, int indexIndex)
{
	if (parameter->type == ImmediateAddressing)	{ *A = 'A';	}
	else if (parameter->type == DirectAddressing)
	{
		/* if it has an external address and direct addressing - external */
		if (parameter->Info.labelIndex.Address == EXTERNAL_ADDRESS)
		{
			*A = 'E';
			if (AddRow(assembledProgram->externalsTable, parameter->Info.labelIndex.Name, indexDirect) == ERROR_ALLOCATION_FAILED)
			{
				return
					assembledProgram->success = 
						PrintError(ERROR_ALLOCATION_FAILED, parameter->Info.labelIndex.Name,
								   "Allocation error when adding to externals table.");
			}
		}
		/* otherwise, it's relative */
		else { *A = 'R'; }
	}
	/* if it's index address */
	else if (parameter->type == IndexAddressing)
	{
		/* second parameter is absolute */
		*B = 'A';
		/* if it has an external address and direct addressing - external */
		if (parameter->Info.indexAddressing.labelIndex.Address == EXTERNAL_ADDRESS)
		{
			*A = 'E';
			if (AddRow(assembledProgram->externalsTable, parameter->Info.indexAddressing.labelIndex.Name, indexIndex) == ERROR_ALLOCATION_FAILED)
			{
				return
					assembledProgram->success =
						PrintError(ERROR_ALLOCATION_FAILED, parameter->Info.labelIndex.Name,
								   "Allocation error when adding to externals table.");
			}
		}
		/* otherwise, it's relative */
		else { *A = 'R'; }
	}

	return ERROR_NONE;
}

/*Final run. Creates the final compiler output*/
AssembledProgram Compile(ReadProgramData *values, AssembledProgram assembledProgram)
{
	int i;

	DynamicArray dataArea = CreateArray();

	for (i = 0; i < values->program->currentLength; i++)
	{
		Instruction *instruction = GetInstruction(values->program, i);
		if (instruction->type == DirectiveInstruction)
		{ SaveDirectives(values, assembledProgram, instruction, dataArea); }
	}

	int IC = 100;
	for (i = 0; i < values->program->currentLength; i++)
	{
		Instruction *instruction = GetInstruction(values->program, i);
		int instructionWordLength = CalculateInstructionWordLength(*instruction);

		if (instruction->type == ActionInstruction)
		{
			ActionInstructionDescription *actionDescription = &(instruction->InstructionDescription.action);

			/*These represent the A(bsolute),R(elocatable),E(xternal) of the parameters*/
			char firstParameterFirstOperand = '\0', firstParameterSecondOperand = '\0';
			char secondParameterFirstOperand = '\0', secondParameterSecondOperand = '\0';

			/* if one parameter, process the second, the destination */
			if (actionDescription->parametersCount == 1)
			{
				/* finds linker data according to parameter */
				if (ProcessParameter(values, assembledProgram, instruction,
									 &(instruction->InstructionDescription.action.secondParameter), 
									 dataArea, &firstParameterFirstOperand, &firstParameterSecondOperand, IC + 1, IC + 1) != ERROR_NONE)
				{ return assembledProgram; }
			}
			/* if two, process both */
			if (actionDescription->parametersCount == 2)
			{
				if (ProcessParameter(values, assembledProgram, instruction,
									 &(instruction->InstructionDescription.action.firstParameter), 
									 dataArea, &firstParameterFirstOperand, &firstParameterSecondOperand, IC + 1, IC + 1) != ERROR_NONE)
				{ 
					return assembledProgram; }

				if (ProcessParameter(values, assembledProgram, instruction,
									 &(instruction->InstructionDescription.action.secondParameter), 
									 dataArea, &secondParameterFirstOperand, &secondParameterSecondOperand,
									 IC + 1, IC + (instructionWordLength / (actionDescription->dbl + 1)) - 2) != ERROR_NONE)
				{ return assembledProgram; }
			}
			/* creates the binary words that will be written to object file */
			unsigned long storeWords[MAX_ACTION_WORDS];
			int length = CreateBinaryWords(*actionDescription, storeWords);
			/* the linker data for the command word is always absolute */
			Insert(assembledProgram->linkerData, (long)'A');
			/* if there's a first parameter, save its data */
			if (firstParameterFirstOperand != '\0')
			{
				Insert(assembledProgram->linkerData, (long)firstParameterFirstOperand);
				if (firstParameterSecondOperand != '\0')
				{
					Insert(assembledProgram->linkerData, (long)firstParameterSecondOperand);
				}
			}
			if (secondParameterFirstOperand != '\0')
			{
				Insert(assembledProgram->linkerData, (long)secondParameterFirstOperand);
				if (secondParameterSecondOperand != '\0')
				{
					Insert(assembledProgram->linkerData, (long)secondParameterSecondOperand);
				}
			}
			/* and save the binary words to the array */
			InsertRange(assembledProgram->values, storeWords, length);
			IC += length;
		}
	}

	/* and add the data at the end */
	InsertRange(assembledProgram->values, dataArea->values, dataArea->currentLength);
	assembledProgram->dataLength = dataArea->currentLength;
	
	return assembledProgram;
}

/*
	Handles the assembly of one file
	file: a character array containing the name of one file to be assembled
*/
ErrorCode AssembleFile(char *fileName, AssembledProgram program)
{
    if (OpenFileAS(fileName) == ERROR_IO) { return ERROR_IO; }

	/* Prepare a data structure to read the program into */
	ReadProgramData* values = NewReadProgramData();
	if (values == NULL) 
	{ PrintError(ERROR_ALLOCATION_FAILED, "No Line Number", "Allocation failed when attempting to create data structure to read program."); }

	ErrorCode firstErrorCode = FirstPass(values);

	ErrorCode secondErrorCode = SecondPass(values);

	Compile(values, program);

	FreeReadProgramData(values);

	program->success = program->success && firstErrorCode && secondErrorCode;

	if (program->success != ERROR_NONE)
	{
		printf("Some errors occurred while parsing '%s', skipping...\n", fileName);
		return program->success;
	}

    return ERROR_NONE;
}

/*
	Assembling all input files, the entry point to the assembler
	files: pointer to array of file names as received from main
	filesLength: the length of the files array
*/
void AssembleFiles(char **fileNames, int fileNamesLegnth)
{
	int i;
	/* send each file to be assembled */
	for (i = 0; i < fileNamesLegnth; i++)
	{
	    AssembledProgram program = NewAssembledProgram();
		if (program == NULL) { return; }
	    if (AssembleFile(fileNames[i], program) != ERROR_NONE) { continue; };
		WriteProgram(program, fileNames[i]);
    }
}

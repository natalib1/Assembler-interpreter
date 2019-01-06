#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

#include "helperFunctions.h"

/*Max size for a command without the label*/
#define COMMAND_MAX_LENGTH 200

/*To be put in Label.Address before all labels are found*/
#define ADDRESS_NA 0

#define LINE_MAX_LENGTH 500

/*ENUMS*/
/*Addressing methods*/
typedef enum { ImmediateAddressing = 0, DirectAddressing = 1, 
			   IndexAddressing = 2, RegisterAddressing = 3 } AddressingType;
/*Directives*/
typedef enum { DataDirective = 0, StringDirective = 1, 
			   EntryDirective = 2, ExternDirective = 3 } DirectiveType;
/*Each instruction is of instruction type*/
typedef enum { EmptyInstruction, CommentInstruction, 
			   DirectiveInstruction, ActionInstruction } InstructionType;
/*When giving each command an address, we need to count data and instructions separetly*/
typedef enum { DataCounter, InstructionCounter } PointerType;


/*STRUCTS*/
/*Used in DirectiveInstructionDescription*/
typedef struct LongArrayStruct
{
	unsigned long values[LABEL_MAX_LENGTH];
	int length;
} LongArray;

/*Description of a label for Direct or Index addressing*/
typedef struct
{
	long Address; /*Address (or ADDRESS_NA)*/
	char Name[LABEL_MAX_LENGTH + 1]; /*The name of the label and terminator*/
} Label;

/*Description of a parameter*/
typedef struct
{
	union Info 
	{
		long value; /*Immediate Addressing*/
		int registerIndex; /*Register Addressing*/
		Label labelIndex; /*Direct Addressing*/
		struct IndexAddressing 
		{
			Label labelIndex; /*Base ptr*/
			Label labelOffset; /*Offset Label*/
			int labelOffsetLength; /* the length of the command at label Offset*/
		} indexAddressing; /*Index Addressing*/
	} Info; /*Polymorphic behavior*/
	AddressingType type; /*Polymorphic behavior*/
} Parameter;

/*Stores information about an action statement*/
typedef struct ActionLineStruct
{
	int firstArgument;
	int secondArgument;
} ActionLine;

/*Description of an action statement.
  If theres only one parameter, its the SECOND parameter*/
typedef struct ActionInstructionDescriptionStruct
{
	int opcode; /*Opcode index (according to constants -> Opcodes*/
	int type; /*Decides whether HIGH or LOW bits*/
	int comb; /*Matters only if type = 1; defines which bits to operate on*/
	int dbl; /*Whether this action should be doubled or not*/
	int parametersCount; /*Either 0, 1 or 2*/
	AddressingType firstAddressingType; /*Addressing type of the first parameter*/
	AddressingType secondAddressingType; /*Addressing type of the second parameter*/
	Parameter firstParameter; /*First parameter is the source parameter*/
	Parameter secondParameter; /*Second parameter is the destination parameter*/
	ActionLine indexes;
} ActionInstructionDescription;

/*Description of a directive statement*/
typedef struct DirectiveInstructionDescriptionStruct
{
	union Data
	{
		LongArray values;					/* number values in .data */
		char string[COMMAND_MAX_LENGTH];	/* string in .string */
		Label label;						/* the label in .entry or .extern */
	} Data; /*Polymorphic behavior*/
	int indexArg; /*The index of the argument*/
	DirectiveType type; /*The type of the directive; Polymorphic behavior*/
} DirectiveInstructionDescription;

/*Stores general indexes about a statement*/
typedef struct 
{
	char text[LINE_MAX_LENGTH]; /*For easy accessing*/
	int lineNumber; /*For error display*/
	int instructionStart; /*First non whitespace which is of the command itself*/
} InstructionLineValues;

/*Represents a whole instruction in the file
  If its a comment of empty line, Statement is left untouched.*/
typedef struct
{
	union InstructionDescription
	{
		ActionInstructionDescription action; /*If its an action*/
		DirectiveInstructionDescription directive; /*If its a directive*/
	} InstructionDescription; /*Polymorphic: The info*/
	InstructionType type; /*The type of the statement: Polymorphic behaviour*/
	int labelExists;
	char label[LABEL_MAX_LENGTH]; /*If a label exists*/
	InstructionLineValues indexes;
} Instruction;

/*FUNCTIONS*/
/*Returns error message on failure.
  Sets statement to parsed statement from line*/
ErrorCode ParseInstruction(char *line, int lineNumber, Instruction *instruction);

#endif
/* PARSER_INCLUDED */

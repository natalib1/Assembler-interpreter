#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "constants.h"
#include "dynamicArray.h"


/*Analyze the instruction, returns the type.
  Sets label to be the name (else untouched)
  Sets instructionStart to the first char of the instruction,
  post white-space and post label.*/
ErrorCode GetInstructionType(char *line, char *label, int *instructionStart, InstructionType *type)
{
	int length;

	/* find the beginning of the instruction */
	*instructionStart = SkipWhiteSpace(line, *instructionStart);
	
	/* get length of line */
	length = strlen(&line[*instructionStart]);

	/* if length is zero or it's all white space, it's not  */
	if (length == 0)
	{
		*type = EmptyInstruction;
		return ERROR_NONE;
	}

	
	/*now, its safe to assume len > 0*/
	if (line[*instructionStart] == ';')
		/*By definition, if and only if
		the first char is ';', its a comment*/
	{
		*type = CommentInstruction;
		return ERROR_NONE;
	}

	/* empty string, just in case */
	label[0] = '\0';
	/*Skip the label and whitespace if exists*/
	ErrorCode res = DetectLabel(line, label, instructionStart);

	if (res != ERROR_NONE) { return res; }
	/* if there's a period after the label, it's a directive (.data, .string, ...) */
	if (line[*instructionStart] == '.')
	{
		*type = DirectiveInstruction;
		return ERROR_NONE;
	}

	*type = ActionInstruction;
	return ERROR_NONE; /*Defaults to this*/
}

/*Parses a long number according to the rules in the assembly.*/
ErrorCode ParseNumber(char *line, int index, char **postNumber, long *value, int checkNextChar)
{
	/* try ton convert the string at given index to a long */
	*value = strtol(&line[index], postNumber, 0); 

	/*the first char isn't a digit*/
	if (*postNumber == &(line[index])) 
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "NaN given to number parsing"); }

	/* if checking is on, assert that the next character is a delimiter */
	if (checkNextChar && (**postNumber != '\0' && **postNumber != ' ' &&
						  **postNumber != '\t' && **postNumber != ',')) 
	{
		return PrintError(ERROR_BAD_PARAMETERS, line, "The character after the number is illegal. The legal characters are the space, tab and comma characters.");
	}
	return ERROR_NONE;
}

/*Parses a long number according to the rules in the assembly.*/
ErrorCode ParseNumberAutoCheck(char *line, int index, char **postNumber, long *value)
{
	return ParseNumber(line, index, postNumber, value, 1);
}

/*Advances the index to after the parameter.*/
ErrorCode SetIndexToNextParameter(char *line, int *index)
{
	*index = SkipWhiteSpace(line, *index);

	if (!line[*index]) return ERROR_NONE; /*End*/
	
	if (line[*index] == ',')
	{
		(*index)++;
		*index = SkipWhiteSpace(line, *index);
		return ERROR_NONE;
	}
	
	return PrintError(ERROR_BAD_PARAMETERS, line, "Parameter separator should be a comma.");
}

ErrorCode CreateImmediateAddressParameter(char *line, int *index, Parameter *parameter)
{
	/* go to the next character */
	(*index)++;

	char *postNumber;
	ErrorCode errorCode;
	parameter->type = ImmediateAddressing;

	/* parse the number */
	errorCode = ParseNumberAutoCheck(line, *index, &postNumber, &(parameter->Info.value));
	if (errorCode != ERROR_NONE) { return errorCode; }

	/*Now index is after the parameter*/
	*index += postNumber - &(line[*index]);
	return SetIndexToNextParameter(line, index);
}

ErrorCode CreateRegisterAddressParameter(char *line, int *index, Parameter *parameter, int regIndex)
{
	parameter->type = RegisterAddressing;
	parameter->Info.registerIndex = regIndex;

	/*Advance to after name of register*/
	*index += strlen(Registers[regIndex]);

	return SetIndexToNextParameter(line, index);
}

ErrorCode CreateIndexAddressParameter(char *line, int *index, Parameter *parameter, int length)
{
	parameter->type = IndexAddressing;

	Label base;
	Label offset;
	base.Address = ADDRESS_NA; /*We don't have an address yet*/
	offset.Address = ADDRESS_NA;
	strncpy(base.Name, &line[*index], length); /*Copy the string*/
	base.Name[length] = '\0'; /*strncpy doesn't NULL terminate*/
	parameter->Info.indexAddressing.labelIndex = base;

	*index += length + 1; /*Skip word and parentheses*/

	if (line[*index] != '!')
	{
		return PrintError(ERROR_BAD_PARAMETERS, line, "Must use ! before Index Addressing.");
	}

	(*index)++; /*Skip '!'*/
	
	strncpy(offset.Name, &line[*index], NextWordsLength(&line[*index])); /* copy the label in the braces */
	offset.Name[NextWordsLength(&line[*index])] = '\0';
	parameter->Info.indexAddressing.labelOffset = offset;

	/*Now index is after the parameter*/
	*index += NextWordsLength(&line[*index]) + 1;
	return SetIndexToNextParameter(line, index);
}

ErrorCode CreateDirectAddressParameter(char *line, int *index, Parameter *parameter, int length)
{
	Label addr;
	addr.Address = ADDRESS_NA; /*We don't have an address yet*/
	strncpy(addr.Name, &line[*index], length); /*Copy the string*/
	addr.Name[length] = '\0'; /*strncpy doesn't NULL terminate*/

	if (length == 0)
	{ return PrintError(ERROR_OUT_OF_BOUNDS, line, "Label length is too short. That is, zero."); }
	if (!IsChar(addr.Name[0]))
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Label starts with an invalid character."); }
	*index += length; /*Skip word*/
	parameter->type = DirectAddressing;
	parameter->Info.labelIndex = addr;

	return SetIndexToNextParameter(line, index);
}

/*Parse the next parameter.*/
ErrorCode CreateParameter(char *line, int *index, Parameter *parameter)
{
	*index = SkipWhiteSpace(line, *index);

	/* if this is the end of the line */
	if (!line[*index])
	{
		/* Mark it as not a parameter */
		*index = -1;
		return ERROR_NONE;
	}

	/* if it's an immmediate address */
	if (line[*index] == '#') { return CreateImmediateAddressParameter(line, index, parameter); }

	int regIndex;
	/* if it's a register address */
	if ((FindRegister(line, *index, &regIndex)) != ERROR_DOES_NOT_EXIST)
	{ return CreateRegisterAddressParameter(line, index, parameter, regIndex); }

	/*Or it's either Direct or Index addressing*/
	int length = NextWordsLength(&line[*index]);

	if (length >= LABEL_MAX_LENGTH)
	{ return PrintError(ERROR_OUT_OF_BOUNDS, line, "The label is too long."); }

	/*Index Address*/
	if (line[*index + length] == '{') 
	{ return CreateIndexAddressParameter(line, index, parameter, length); }

	/*If all else fails, it must be Direct Address*/
	return CreateDirectAddressParameter(line, index, parameter, length);
}

/* convert an instruction line into an action instruction */
ErrorCode CreateDescriptionActionInstruction(char *line, ActionInstructionDescription *description)
{
	int opcode;
	ErrorCode errorCode = FindOpcode(line, 0, &opcode);

	description->opcode = opcode;

	if (errorCode == ERROR_DOES_NOT_EXIST)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Wrong Opcode used in instruction."); }

	int currentIndex = strlen(Opcodes[opcode]);
	/* an action word must be followed with a slash */
	if (line[currentIndex] != '/')
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "An action must be followed by a slash."); }
	/* skip the slash */
	++currentIndex;
	/* if type bit is 0, set the proper fields */
	if (line[currentIndex] == '0') /*No combination supplied*/
	{
		description->type = 0;
		description->comb = 0;

		++currentIndex;
	}
	/* and if it's 1, check the next parameters */
	else if (line[currentIndex] == '1') /*Combination supplied*/
	{
		description->type = 1;
		description->comb = 0;
		++currentIndex;
		/* check the comb parameters */
		if (line[currentIndex] != '/')
		{ return PrintError(ERROR_BAD_PARAMETERS, line, "A slash must follow a type 1."); }
		/* skip the slash */
		++currentIndex;
		/* if the first bit is 0, comb is already set */
		if (line[currentIndex] == '0');
		/* the first number affects the left bit */
		else if (line[currentIndex] == '1')
		{ description->comb |= 2; }
		else { return PrintError(ERROR_BAD_PARAMETERS, line, "comb parameter may be 0 or 1 only."); }
		++currentIndex;

		if (line[currentIndex] != '/')
		{ return PrintError(ERROR_BAD_PARAMETERS, line, "A slash must follow a comb number."); }
		/* skip the slash */
		++currentIndex;
		/* if the second bit is 0, no need to further modify comb */
		if (line[currentIndex] == '0');
		/* the second number affects the right bit */
		else if (line[currentIndex] == '1')
		{ description->comb |= 1; }
		else { PrintError(ERROR_BAD_PARAMETERS, line, "comb parameter may be 0 or 1 only."); }
		++currentIndex; /*Skipping number*/
	}
	else { PrintError(ERROR_BAD_PARAMETERS, line, "type parameter may be 0 or 1 only."); }

	if (line[currentIndex] != ',')
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "a type specification must be followed by a comma and dbl parameter."); }
	/* skip the comma */
	++currentIndex;
	if (line[currentIndex] == '0') description->dbl = 0;
	else if (line[currentIndex] == '1') description->dbl = 1;
	else
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "dbl parameter can only be 0 or 1"); }
	++currentIndex;

	/*If there's atleast one parameter (according to definition)*/
	if (AtleastOneParameter(opcode))
	{
		description->indexes.firstArgument = currentIndex = SkipWhiteSpace(line, currentIndex);
		ErrorCode errorCode = CreateParameter(line, &currentIndex, &(description->firstParameter));
		if (errorCode != ERROR_NONE) { return errorCode; }

		if (currentIndex == -1) /*no parameter*/
		{
			if (TwoParameters(opcode))
			{ return PrintError(ERROR_BAD_PARAMETERS, line, "Opcode requires two parameters but there's none"); }
			else { return PrintError(ERROR_BAD_PARAMETERS,line,"Opcode requires one parameter but there's none"); }
		}

		description->firstAddressingType = description->firstParameter.type;
		description->parametersCount = 1;

		if (!TwoParameters(opcode))
		{
			/*If theres only one parameter, its the destination parameter*/
			description->secondParameter = description->firstParameter;
			description->secondAddressingType = description->firstAddressingType;

			description->firstAddressingType = 0;
		}
	}

	/*If there are exactly two parameters (according to definition)*/
	if (TwoParameters(opcode))
	{
		description->indexes.secondArgument = currentIndex;

		ErrorCode errorCode = CreateParameter(line, &currentIndex, &(description->secondParameter));
		if (errorCode != ERROR_NONE) { return errorCode; }

		if (currentIndex == -1)
		{ return PrintError(ERROR_BAD_PARAMETERS, line, "Opcode requires two parameters but there's only one."); }

		description->secondAddressingType = description->secondParameter.type;
		description->parametersCount = 2;
	}

	int validateOperandType = IsLegalAddressingMode(description);

	if (validateOperandType == 0) { return ERROR_NONE; }
	else if (validateOperandType == 1)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "First operand has wrong addressing mode."); }
	else if (validateOperandType == 2)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Second operand has wrong addressing mode."); }
	else if (validateOperandType == 3)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Both operands have wrong addressing mode."); }
	else { return PrintError(ERROR_UNKNOWN, line, "Can't determine addressing modes of operands."); }
}

/* Parses the numbers in the .data directive */
ErrorCode ParseDataParameters(char *line, LongArray *lArray)
{
	int i = 0;

	char *ptr = line;
	/* start parse numbers */
	while (ptr != NULL)
	{
		/* skip white space before the number */
		ptr += SkipWhiteSpace(ptr, 0);
		if (i >= COMMAND_MAX_LENGTH) 
		{ return PrintError(ERROR_OUT_OF_BOUNDS, line, "The line is too long"); }

		char* postNumber = 0;
		lArray->length = i + 1;
		/* parse the number into the array */
		ErrorCode errorCode = ParseNumberAutoCheck(ptr, 0, &postNumber, (long *) &(lArray->values[i]));
		if (errorCode != ERROR_NONE) { return errorCode; }

		/* skip any white space after the number */
		ptr = postNumber;
		ptr += SkipWhiteSpace(ptr, 0);

		/* we've reached the end of the line */
		if (*ptr == '\0') { return ERROR_NONE; }

		/* if after a number there's no whitespace, no end and no comma, this is a bad .data instruction */
		if (*ptr != ',')
		{ return PrintError(ERROR_BAD_PARAMETERS, line, ".data instruction may only contain numbers and commas."); }

		/* skip the comma */
		++ptr;
		++i;
	}

	return ERROR_NONE;
}

/* Parses the the string in the .string directive */
ErrorCode ParseStringParameters(char *line, char *str)
{
	/* skip any whitespace before the string  */
	int currentIndex = SkipWhiteSpace(line, 0);

	/* only whitespace... */
	if (line[currentIndex] == '\0')
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "No data in the .string directive."); }
	/* string must start with "  */
	if (line[currentIndex] != '\"')
	{ return PrintError(ERROR_BAD_PARAMETERS, line, ".string directive data must start with \""); }

	/* start the string */
	currentIndex++;

	/* find the closing " */
	char* tmp = strchr(&line[currentIndex], '\"');
	/* if not found */
	if (tmp == NULL) { return PrintError(ERROR_BAD_PARAMETERS, line, ".string directive must end with \""); }

	/* get the length of the string */
	int length = tmp - &(line[currentIndex]); 
	/* save the string */
	strncpy(str, &line[currentIndex], length);
	/* move the index */
	currentIndex += length + 1;
	
	/* skip whitespaces */
	currentIndex = SkipWhiteSpace(line, currentIndex);

	/* make sure nothing after string but whitespace */
	if (line[currentIndex] != '\0')
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "There should be nothing after the string in the .string directive."); }

	return ERROR_NONE;
}

/* Parses the label in the .entry or .extern directives */
ErrorCode ParseEntryExternParameters(char *line, char *label)
{
	int currentIndex = SkipWhiteSpace(line, 0);

	/* if already at the end */
	if (line[currentIndex] == '\0')
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "No data in the .entry directive."); }
	/* get the length of the entry word */
	int length = NextWordsLength(&line[currentIndex]);
	/* if the length is somehow zero */
	if (length == 0)
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "The label in the .entry directive must be of positive length."); }
	/* if it's bigger than permitted */
	if (length >= LABEL_MAX_LENGTH)
	{ return PrintError(ERROR_OUT_OF_BOUNDS, line, "The label in the .entry directive is too long."); }
	/* get the label */
	strncpy(label, &line[currentIndex], length);
	/* terminate the string because strncpy doesn't */
	label[length] = '\0';
	/* the label must start with a character */
	if (!IsChar(label[0]))
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "The label in the .entry directive must start with a character."); }

	return ERROR_NONE;
}

/* Parses the Directive Instruction */
ErrorCode CreateDescriptionDirectiveInstruction(char *line, DirectiveInstructionDescription *description)
{
	int directive;
	/* directive instruction must start with a . */
	if (line[0] != '.')
	{ return PrintError(ERROR_BAD_PARAMETERS, line, "Directive Instruction must start with a ."); }
	/* find the directive type */
	if (FindDirective(line, 1, &directive) == ERROR_DOES_NOT_EXIST)
	{ return PrintError(ERROR_DOES_NOT_EXIST, line, "No directive found in instruction line."); }
	/* convert to DirectiveType */
	description->type = (DirectiveType)directive;
	int currentIndex = 1 + strlen(Directives[directive]);

	/* skip any whitespace to the data */
	currentIndex = SkipWhiteSpace(line, currentIndex);

	/* if .data directive */
	if (description->type == DataDirective)
	{ return ParseDataParameters(&(line[currentIndex]), &(description->Data.values)); }
	/* if .string directive */
	if (description->type == StringDirective)
	{ return ParseStringParameters(&(line[currentIndex]), description->Data.string); }
	/* if .entry or .extern directive */
	if (description->type == EntryDirective || description->type == ExternDirective)
	{ return ParseEntryExternParameters(&(line[currentIndex]), description->Data.label.Name); }
	/* if, for some reason, nothing was found */
	else { return PrintError(ERROR_BAD_PARAMETERS, line, "No directive found in directive instruction."); }
}

ErrorCode ParseInstruction(char *line, int lineNumber, Instruction *instruction)
{
	/* save the entire line of the instruction */
	strcpy(instruction->indexes.text, line);
	int instructionStart = 0;
	ErrorCode errorCode;

	/* save the instruction label, if any, where it actually starts and its type */
	errorCode = GetInstructionType(line, instruction->label, &instructionStart, &instruction->type);
	if (errorCode != ERROR_NONE) { return errorCode; }

	/* save basic instruction information */
	instruction->indexes.instructionStart = instructionStart;
	instruction->indexes.lineNumber = lineNumber;
	if (strlen(instruction->label) != 0) { instruction->labelExists = 1; }

	/* if no further parsing required, return */
	if (instruction->type == CommentInstruction || instruction->type == EmptyInstruction) 
		{ return ERROR_NONE; }
	/* if this is an Action instruction, create the description */
	else if (instruction->type == ActionInstruction)
	{ return CreateDescriptionActionInstruction(&(line[instructionStart]), &(instruction->InstructionDescription.action)); }
	/* if it's a directive, create the description for that */
	else if (instruction->type == DirectiveInstruction)
	{
		errorCode = CreateDescriptionDirectiveInstruction(&(line[instructionStart]), &(instruction->InstructionDescription.directive));
		/* check if the directive has a label - which is wrong */
		if ((instruction->InstructionDescription.directive.type == ExternDirective || instruction->InstructionDescription.directive.type == EntryDirective) && !(errorCode != ERROR_NONE)
			&& instruction->labelExists)
		{ return PrintError(ERROR_BAD_PARAMETERS, line, "Extern or Entry directives may not have a label."); }
		return errorCode;
	}
	
	return PrintError(ERROR_UNKNOWN, line, "Could not parse instruction type.");
}

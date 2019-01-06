#include <string.h>
#include "binaryCompiler.h"
#include "helperFunctions.h"


/*Adds the appropriate bytes to the given code*/
void AddParameter(Parameter parameter, MachineInstruction *machineInstruction)
{
	if (parameter.type == ImmediateAddressing)
	{
		machineInstruction->parameters[machineInstruction->length] = parameter.Info.value;
		(machineInstruction->length)++; /*Just one byte required*/
	}
	else if (parameter.type == DirectAddressing)
	{
		machineInstruction->parameters[machineInstruction->length] = parameter.Info.labelIndex.Address;
		(machineInstruction->length)++; /*Just one byte required*/
	}
	else if (parameter.type == IndexAddressing)
	{
		machineInstruction->parameters[machineInstruction->length] = 
			parameter.Info.indexAddressing.labelIndex.Address;
		(machineInstruction->length)++; /*To make next parameter*/
		machineInstruction->parameters[machineInstruction->length] = 
			parameter.Info.indexAddressing.labelOffsetLength;
		(machineInstruction->length)++; /*Two bytes required*/
	}
}

/* translates the collected data of the action into the bits of a machine instruction */
MachineInstruction GenerateMachineInstruction(ActionInstructionDescription action)
{
	MachineInstruction output = {{0}}; /*Initializing to zero*/
	output.opcode.type = action.type;
	output.opcode.dbl = action.dbl;
	if (action.type) { output.opcode.combination = action.comb; }

	if (action.parametersCount == 1 && action.secondParameter.type == RegisterAddressing) /*The first is the dest reg*/
	{ output.opcode.registerDestination = action.secondParameter.Info.registerIndex; }

	if (action.parametersCount == 2)
	{
		if (action.firstParameter.type == RegisterAddressing)
		{ output.opcode.registerSource = action.firstParameter.Info.registerIndex; }
		if (action.secondParameter.type == RegisterAddressing)
		{ output.opcode.registerDestination = action.secondParameter.Info.registerIndex; }
	}

	output.opcode.addressingSource = (unsigned int)action.firstAddressingType;
	output.opcode.addressingDestination = (unsigned int)action.secondAddressingType;

	output.opcode.opcode = action.opcode;

	if (action.parametersCount == 1) { AddParameter(action.secondParameter, &output); }
	if (action.parametersCount >= 2)
	{
		AddParameter(action.firstParameter, &output);
		AddParameter(action.secondParameter, &output);
	}

	return output;
}

/* calculates how many machine words will this instruction take */
int CalculateInstructionWordLength(Instruction instruction)
{
	if (instruction.type == CommentInstruction || instruction.type == EmptyInstruction) { return 0; }
	if (instruction.type == ActionInstruction)
	{
		MachineInstruction code = GenerateMachineInstruction(instruction.InstructionDescription.action);
		return (code.length + 1);
	}
	if (instruction.type == DirectiveInstruction)
	{
		if (instruction.InstructionDescription.directive.type == DataDirective)
		{
			return instruction.InstructionDescription.directive.Data.values.length;
		}
		if (instruction.InstructionDescription.directive.type == StringDirective)
		{
			return strlen(instruction.InstructionDescription.directive.Data.string) + 1; /*Plus one for the null terminator*/
		}

		return 0; /*Entry and Extern are just definitions*/
	}

	return 0;
}

/* creates the binary word to write from the collected machine command word data */
long CreateBinaryWordFromOpcode(MachineCommandWord machineCommandWord)
{
	long val = 0;
	val ^= machineCommandWord.combination;
	val ^= machineCommandWord.registerDestination << 2;
	val ^= machineCommandWord.addressingDestination << 5;
	val ^= machineCommandWord.registerSource << 7;
	val ^= machineCommandWord.addressingSource << 10;
	val ^= machineCommandWord.opcode << 12;
	val ^= machineCommandWord.type << 16;
	val ^= machineCommandWord.dbl << 17;
	return val;
}

int CreateBinaryWords(ActionInstructionDescription action, unsigned long* fill)
{
	MachineInstruction code = GenerateMachineInstruction(action);

	fill[0] = CreateBinaryWordFromOpcode(code.opcode);
	int i;
	for (i = 0; i < code.length; i++) { fill[i + 1] = code.parameters[i]; }
	return i + 1;
}

void ToBaseEight(unsigned long value, char* output)
{
	int i;
	int length = 0;
	unsigned long copy = value;
	while (copy) /*Finding the size of the number*/
	{
		length++;
		copy /= 8;
	}
	output[length] = '\0'; /*NULL terminating*/
	for (i = length - 1; i >= 0; i--)
	{
		output[i] = (value % 8) + '0'; /*length % 8 is the right-most digit; adding a '0' gives the ASCII value of it*/
		value /= 8; /*removing the right-most digit*/
	}
}

void ToBaseEightMinSize(unsigned long value, int minSize, char* output)
{
	int i;
	int length = 0;
	unsigned long copy = value & 0xFFFFF;
	/*Finding the size of the number*/
	while (copy) { length++; copy /= 8;	}

	int padding = minSize - length;
	if (padding < 0) { return; }

	for (i = minSize - 1; i >= minSize - length; i--)
	{
		output[i] = (value % 8) + '0'; /*length % 8 is the right-most digit; adding a '0' gives the ASCII value of it*/
		value /= 8; /*removing the right-most digit*/
	}
	output[minSize] = '\0'; /*Terminate string*/
	for (i = minSize - length - 1; i >= 0; --i) { output[i] = '0'; /*Add padding*/ }
	return;
}

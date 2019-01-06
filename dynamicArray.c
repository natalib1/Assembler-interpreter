#include <stdlib.h>
#include "dynamicArray.h"
#include "helperFunctions.h"

DynamicArray CreateArray(void)
{
    /* allocate array */
	DynamicArray dArray = (DynamicArray)malloc(sizeof(ArrayStruct));
	if (dArray == NULL) { return NULL; }

    /* set basic fields */
	dArray->currentLength = 0;
	dArray->maxLength = ARRAY_STARTING_SIZE;

    /* allocate values */
	dArray->values = (unsigned long*)malloc(sizeof(unsigned long) * ARRAY_STARTING_SIZE);
	if (dArray->values == NULL) { free(dArray); return NULL; }

	return dArray;
}

ErrorCode ReallocateArray(DynamicArray dArray)
{
    dArray->maxLength = dArray->currentLength * ARRAY_DYNAMIC_FACTOR;
	unsigned long *tmp = (unsigned long*)realloc(dArray->values, dArray->maxLength * sizeof(unsigned long));
	if (tmp == NULL)
	{
		free(dArray->values);
		free(dArray);
		return PrintError(ERROR_ALLOCATION_FAILED, "ReallocateArray", "Couldn't reallocate.");
	}
	dArray->values = tmp;
	return ERROR_NONE;
}

unsigned long* Insert(DynamicArray dArray, unsigned long value)
{
    /* if array is full, reallocate */
    if (dArray->currentLength >= dArray->maxLength)
	{ if (ReallocateArray(dArray) == ERROR_ALLOCATION_FAILED) { return NULL;} }

    /* add value to array */
	dArray->values[dArray->currentLength] = MakeTwosComplement(value);
	dArray->currentLength++;
	return &dArray->values[dArray->currentLength - 1];
}

unsigned long* InsertRange(DynamicArray dArray, unsigned long *range, int length)
{
	int index;
	unsigned long *tmp = NULL;

    /* go over the range of values and add them one by one */
	for (index = 0; index < length; ++index)
    { if ((tmp = Insert(dArray, range[(long)index])) == NULL) { return NULL; } }

	return tmp;
}

#ifndef DYNAMICARRAY_INCLUDED
#define DYNAMICARRAY_INCLUDED

#define ARRAY_STARTING_SIZE 10
#define ARRAY_DYNAMIC_FACTOR 2

typedef struct
{
	unsigned long *values;
	int currentLength;
	int maxLength;
} ArrayStruct;
typedef ArrayStruct *DynamicArray;

/*
    Creates a new dynamic array
    Returns: a new array or null if failed
*/
DynamicArray CreateArray(void);

/*
    Inserts a value into the array
    dArray: the array to insert the value into
    value: the new value to insert into the array
    Returns a pointer to the value in the array, null if failure
*/
unsigned long* Insert(DynamicArray dArray, unsigned long value);

/*
    Inserts a range of values into the array
    dArray: the array to insert the values into
    range: a pointer to an array of values to insert
    length: the length of the values array
    Returns a pointer to the last value inserted, null if failure
*/
unsigned long* InsertRange(DynamicArray dArray, unsigned long *range, int length);

#endif
/* DYNAMICARRAY_INCLUDED */

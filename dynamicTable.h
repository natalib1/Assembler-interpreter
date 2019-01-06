#ifndef DYNAMICTABLE_INCLUDED
#define DYNAMICTABLE_INCLUDED

/* Maximal length for the data in the name column */
#define ROW_NAME_COLUMN_MAX_LENGTH 50
#define TABLE_STARTING_SIZE 10
#define TABLE_DYNAMIC_FACTOR 2

#include "helperFunctions.h"

/*A row in the table*/
typedef struct
{
	char name[ROW_NAME_COLUMN_MAX_LENGTH];
	long address;
} TableRow;

struct TableStruct
{
	TableRow *rows;
	int rowsAmount;
	int rowsMax;
	/* Should duplicate versions of the same row be allowed */
	int allowDuplicates;
};
typedef struct TableStruct* Table;

/*
Deletes a given table and releases its resources
table: The table to delete
*/
Table FreeTable(Table table);

/*
    Creates a new table from scratch
*/
Table NewTable(void);

/*
    Finds row in a given table based on the name column.
    table: The table to search in
    name: the name of the row to find
    Returns the row found or NULL if failed
*/
TableRow* FindRow(Table table, char *name);

/*
    Adds a row to the given table
    table: The table to add the row to.
    name: a character array with the name field of the row
    address: the address field of the row
    Returns ErrorCode according to convention
*/
ErrorCode AddRow(Table table, char *name, long address);

#endif 
/*DYNAMICTABLE_INCLUDED*/

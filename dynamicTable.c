#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dynamicTable.h"


Table FreeTable(Table table)
{
	if (table != NULL)
	{
		if (table->rows != NULL) { free(table->rows); }
		free(table);
	}
	return NULL;
}

Table NewTable(void)
{
    /* allocate table */
	Table table = (Table)malloc(sizeof(struct TableStruct));
	if (table == NULL) { return FreeTable(table); }

    /* set basic fields */
	table->allowDuplicates = 0;
	table->rowsMax = TABLE_STARTING_SIZE;
	table->rowsAmount = 0;

    /* allocate rows */
	table->rows = (TableRow*)malloc(sizeof(TableRow)* TABLE_STARTING_SIZE);
	if (table->rows == NULL) { return FreeTable(table); }

	return table;
}

TableRow* FindRow(Table table, char *name)
{
    /* can't actually find a specific row if there could be more than one */
	if (table->allowDuplicates) { return NULL; }

	int i;
	for (i = 0; i < table->rowsAmount; ++i)
    { if (strcmp(table->rows[i].name, name) == 0) {	return &(table->rows[i]); } }

	return NULL; /*if not found*/
}

/*
    Reallocates the table using the dynamic factor
    table: the table to reallocate
*/
Table ReallocateTable(Table table)
{
	/* attempt toreallocate */
	TableRow* tmpRows = (TableRow*)realloc(table->rows, table->rowsMax * TABLE_DYNAMIC_FACTOR * sizeof(TableRow));
	if (tmpRows == NULL) {
		FreeTable(table);
		return NULL;
	}
	/* assign new rows */
	table->rows = tmpRows;
	table->rowsMax *= TABLE_DYNAMIC_FACTOR;
	return table;
}

ErrorCode AddRow(Table table, char *name, long address)
{
    /* if it exceeds max length of name, chuck it */
	if (strlen(name) > ROW_NAME_COLUMN_MAX_LENGTH) { return ERROR_OUT_OF_BOUNDS; }
    /* if we don't allow duplicates, and the row name exists, chuck it */
	if ((!table->allowDuplicates) && FindRow(table, name) != NULL) { return ERROR_ALREADY_EXISTS; }
    /* if the table is maxed on rows, we need to reassign */
	if (table->rowsAmount >= table->rowsMax)
	{ if (ReallocateTable(table) == NULL) { return ERROR_ALLOCATION_FAILED; } }

    /* create a new row */
	TableRow row;
	row.address = address;
	strcpy(row.name, name);

    /* add row to table */
	table->rows[table->rowsAmount] = row;
	table->rowsAmount++;

	return ERROR_NONE;
}

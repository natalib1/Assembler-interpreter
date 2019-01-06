#include <stdio.h>
#include "assembler.h"
#include "helperFunctions.h"

/* program entry point */
int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		printf("Illegal number of parameters.\nDo: %s [files]\n", argv[0]);
		printf("Where [files] are one or more names of files with .as extension.\n");
		return ERROR_BAD_PARAMETERS;
	}

	AssembleFiles(argv + 1, argc - 1);
	
	return 0;
}
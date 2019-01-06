assembler: assembler.o binaryCompiler.o constants.o dynamicArray.o dynamicTable.o fileHandler.o helperFunctions.o main.o parser.o programReader.o
	gcc -g -ansi -Wall assembler.o binaryCompiler.o constants.o dynamicArray.o dynamicTable.o fileHandler.o helperFunctions.o main.o parser.o programReader.o -o assembler

assembler.o: assembler.c assembler.h fileHandler.h binaryCompiler.h programReader.h dynamicArray.h dynamicTable.h helperFunctions.h
	gcc -c -ansi -Wall assembler.c -o assembler.o
	
binaryCompiler.o: binaryCompiler.c binaryCompiler.h helperFunctions.h parser.h
	gcc -c -ansi -Wall binaryCompiler.c -o binaryCompiler.o
	
constants.o: constants.c constants.h helperFunctions.h parser.h
	gcc -c -ansi -Wall constants.c -o constants.o

dynamicArray.o: dynamicArray.c dynamicArray.h helperFunctions.h
	gcc -c -ansi -Wall dynamicArray.c -o dynamicArray.o

dynamicTable.o: dynamicTable.c dynamicTable.h helperFunctions.h
	gcc -c -ansi -Wall dynamicTable.c -o dynamicTable.o
	
fileHandler.o: fileHandler.c fileHandler.h helperFunctions.h dynamicTable.h parser.h binaryCompiler.h assembler.h
	gcc -c -ansi -Wall fileHandler.c -o fileHandler.o
	
helperFunctions.o: helperFunctions.c helperFunctions.h constants.h
	gcc -c -ansi -Wall helperFunctions.c -o helperFunctions.o
	
main.o: main.c assembler.h helperFunctions.h
	gcc -c -ansi -Wall main.c -o main.o
	
parser.o: parser.c parser.h helperFunctions.h constants.h dynamicArray.h
	gcc -c -ansi -Wall parser.c -o parser.o
	
programReader.o: programReader.c programReader.h parser.h
	gcc -c -ansi -Wall programReader.c -o programReader.o
#ifndef HELPERFUNCTIONS_INCLUDED
#define HELPERFUNCTIONS_INCLUDED

#define TRUE 1
#define FALSE 0

#define LABEL_MAX_LENGTH 40

typedef enum { ERROR_NONE, ERROR_UNKNOWN, ERROR_BAD_PARAMETERS,
               ERROR_IO, ERROR_OUT_OF_BOUNDS, ERROR_ALLOCATION_FAILED,
               ERROR_ALREADY_EXISTS, ERROR_DOES_NOT_EXIST } ErrorCode;

extern char *errors[];

/* Transforms a number into two's complement representation */
unsigned long MakeTwosComplement(long value);

/*Returns index of next non empty char, else end of string*/
int SkipWhiteSpace(char *line, int startIndex);

/* Returns 1 if is char, else 0*/
int IsChar(char c);

/*Returns 1 if is digit, else 0*/
int IsDigit(char c);

/*Returns 1 if is digit or char, else 0*/
int IsAlphanumeric(char c);

/*Returns the length of the next alphanumeric word*/
int NextWordsLength(char *line);

/*Returns 0 if not a label. ElseReturns post-label and sets labelName*/
ErrorCode DetectLabel(char *line, char *labelName, int *startStatement);

ErrorCode PrintError(ErrorCode errorCode, char* line, char* message);
#endif
/* HELPERFUNCTIONS_INCLUDED */

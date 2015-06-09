/*
 * Lexical Analyzer
 * Anton Ridgway
 * CS4013 (Compilers)
 * September 23, 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//lexical analyzer size restrictions
#define BUFF_LEN 72
#define MAX_ID_LEN 10
#define MAX_INT_LEN 10
#define MAX_R_NUM_LEN 5
#define MAX_R_DEC_LEN 5
#define MAX_R_EXP_LEN 2

#define START_NEWLINE 2

//tokenNode types
#define LEXERR_TYPE 0
#define ADDOP_TYPE 1
#define MULOP_TYPE 2
#define RELOP_TYPE 3
#define ASSIGNOP_TYPE 4
#define WS_TYPE 5
#define ID_TYPE 6
#define COLON_TYPE 7
#define SEMICOLON_TYPE 8
#define DOT_TYPE 9
#define DOTDOT_TYPE 10
#define LPARENS_TYPE 11
#define RPARENS_TYPE 12
#define LBRACKET_TYPE 13
#define RBRACKET_TYPE 14
#define COMMA_TYPE 15
#define INT_TYPE 16
#define REAL_TYPE 17
#define LONGREAL_TYPE 18
#define EOF_TYPE 19

#define NULL_ATT 0
#define RELOP_LE 1
#define RELOP_NE 2
#define RELOP_LT 3
#define RELOP_GE 4
#define RELOP_GT 5
#define RELOP_EQ 6
#define ADDOP_PLUS 1
#define ADDOP_MINUS 2
#define MULOP_MULT 1
#define MULOP_DIV 2

//ID is longer than 10 characters
#define ID_TOOLONG 0
//int has leading zeroes
#define NUM_ZEROSTART 1
//int is longer than 10 characters
#define INT_TOOLONG 2
//real has more than 5 number values
#define R_TOOMANY_X 3
//real has more than 5 fraction values
#define R_TOOMANY_Y 4
//longreal has more than 2 exponent values
#define R_TOOMANY_Z 5
//longreal no exponent values following E
#define R_NO_Z 6
//undefined characters
#define UNDEF_CHAR 7

struct symbolNode
{
    char* name;
	int tokenType;
    int attribute;
    struct symbolNode* next;
};
struct tokenNode
{
    char* lexeme;
    int tokenType;
    intptr_t attribute;
    int lineNo;
	struct tokenNode* next;
};
typedef struct symbolNode symbolNode;
typedef struct tokenNode tokenNode;

symbolNode* loadReservedWords();
symbolNode* searchSymbolList(char* sName, symbolNode *currSym);
symbolNode* checkAddSymbol(char* sName, symbolNode *currSym);

int relopMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum);
int idresMachine(char* buffer, int *f, int *b, tokenNode* lastToken, symbolNode* idList, symbolNode* rWordsList, int lineNum);
int wsMachine(char* buffer, int *f, int *b);
int catchallMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum);
int longrealMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum);
int realMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum);
int intMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum);

char* getTokenMeaning(int tType, symbolNode* resList);
char* getErrorMeaning(int eType);
void makeTokenFile(tokenNode* firstToken, symbolNode* resList);
void makeSymbolFile();
void cleanUp(symbolNode* resTable, tokenNode* tokenList, symbolNode* idList);

int main(int argc, char *argv[])
{
    printf("Loading reserved words from reserved.txt...\n");
    symbolNode* rWordsList = loadReservedWords();

    //open source file for reading, listing for writing
    FILE *sourcePtr, *listPtr;
	if(argc < 1)
	{
		printf("No input file given to compile.\n");
		return 0;
	}

	printf("Opening files %s and listing.txt\n", argv[1]);
	sourcePtr = fopen(argv[1], "r");
    if(sourcePtr == NULL)
    {
        fprintf(stderr,"File-open has failed. Is the source file present?\n");
        return 0;
    }
    listPtr = fopen("listing.txt", "w");
    if(listPtr == NULL)
    {
        fprintf(stderr,"Output-file open has failed.\n");
        fclose(sourcePtr);
        return 0;
    }

	//start the token list, with a dummy token
	tokenNode* tokenList = malloc(sizeof(tokenNode));
	tokenList->next = NULL;
	tokenNode* lastToken = tokenList;
    symbolNode* idList = malloc(sizeof(symbolNode));
    idList->next = NULL;

    char* lineBuffer = malloc((BUFF_LEN+1)*sizeof(char)); //72 chars
    int currLine = 0;
    int* forward = malloc(sizeof(int));
    int* back = malloc(sizeof(int));
    *forward = 0;
    *back = 0;
	int returnVal = START_NEWLINE; //moves through the larger lexical analyzer machine.
    int doneReading = 0;

	printf( "Parsing source file...\n" );
    while(!doneReading)
	{
	    if(returnVal == START_NEWLINE || *forward == BUFF_LEN)
		{
		    printf("Loading next line:\n");
            int i;
            for(i = 0; i < BUFF_LEN; i++) lineBuffer[i] = ' ';
            *forward = 0; *back = 0; currLine++;
            fgets(lineBuffer, BUFF_LEN+1, sourcePtr);
            printf("%d: %s\n",currLine,lineBuffer);
            fprintf(listPtr, "%d: %s", currLine, lineBuffer);
		}

		//make into tokenNodes, including error tokens
		returnVal = wsMachine(lineBuffer, forward, back);
		if(returnVal == 0)
		{
            returnVal = idresMachine(lineBuffer, forward, back, lastToken, idList, rWordsList, currLine);
            if(returnVal == 0)
            {
                returnVal = catchallMachine(lineBuffer, forward, back, lastToken, currLine);
                if(returnVal == 0)
                {
                    returnVal = longrealMachine(lineBuffer, forward, back, lastToken, currLine);
                    if(returnVal == 0)
                    {
                        returnVal = realMachine(lineBuffer, forward, back, lastToken, currLine);
                        if(returnVal == 0)
                        {
                            returnVal = intMachine(lineBuffer, forward, back, lastToken, currLine);
                            if(returnVal == 0)
                            {
                                returnVal = relopMachine(lineBuffer, forward, back, lastToken, currLine);
                                if(returnVal == 0)
                                {
                                    // EOF is an unknown char outside the usual ASCII character set
                                    if(feof(sourcePtr) && (lineBuffer[*forward] < ' ' || lineBuffer[*forward] > '~') )
                                    {
                                        returnVal = 1;
                                        doneReading = 1;
                                        printf("End of file.\n\n");
                                        tokenNode* newToken = malloc(sizeof(tokenNode));
                                        newToken->lexeme = malloc(2*sizeof(char));
                                        newToken->lexeme[0] = '$';
                                        newToken->lexeme[1] = '\0';
                                        newToken->tokenType = EOF_TYPE;
                                        newToken->attribute = NULL_ATT;
                                        newToken->lineNo = currLine;
                                        newToken->next = NULL;
                                        lastToken->next = newToken;
                                    }
                                    else
                                    {
                                        printf("Error: character '%c' does not belong to the grammar.\n",lineBuffer[*forward]);
                                        tokenNode* newToken = malloc(sizeof(tokenNode));
                                        newToken->lexeme = malloc(2*sizeof(char));
                                        newToken->lexeme[0] = lineBuffer[*forward];
                                        newToken->lexeme[1] = '\0';
                                        newToken->tokenType = LEXERR_TYPE;
                                        newToken->attribute = UNDEF_CHAR;
                                        newToken->lineNo = currLine;
                                        newToken->next = NULL;
                                        lastToken->next = newToken;
                                        *forward = *forward + 1; *back = *back + 1;
                                    }
                                }
                            }//end int
                        }//end real
                    }//end longreal
                }//end catchall
            }//end idres
		}//end wspace
		while(lastToken->next != NULL) //catch up the pointer; write error tokens to listing.txt
		{
            lastToken = lastToken->next;
            if(lastToken->tokenType == LEXERR_TYPE)
            {
                fprintf(listPtr, "%s: %s: %s\n", "LEXERR", getErrorMeaning(lastToken->attribute), lastToken->lexeme);
            }
            printf("Added token %s\n",lastToken->lexeme);
		}
    }// end while
    free(forward); free(back);
    free(lineBuffer);
    fclose(sourcePtr);
    fclose(listPtr);
	if(tokenList->next == NULL) //only dummy token
	{
		printf("No tokens found.\n");
		free(tokenList);
	}
	else //remove dummy token to expose the real list
	{
		tokenNode* dummyToken = tokenList;
		tokenList = tokenList->next;
		free(dummyToken);
		printf("Printing tokens to file.\n");
		makeTokenFile(tokenList,rWordsList);
	}

    if(idList->next == NULL) //only dummy id
	{
		printf("No tokens found.\n");
		free(idList);
	}
	else //remove dummy token to expose the real list
	{
		symbolNode* dummyNode = idList;
		idList = idList->next;
		free(dummyNode);
		printf("Printing symbols to file.\n");
		makeSymbolFile(idList);
	}

	printf( "Cleaning up heap memory...\n" );
	cleanUp(rWordsList, tokenList, idList);
	printf( "Lexical analysis is done!\n" );
    return 0;
};

symbolNode* loadReservedWords()
{
    FILE *wordsFile;
    wordsFile = fopen("reserved.txt","r");
	char fileFormat[] = "%s\t%d\t%d";
    char nameIn [BUFF_LEN];
    int typeIn;
    int attIn;
    symbolNode* firstWord = NULL;

    if(fscanf(wordsFile, fileFormat, nameIn, &typeIn, &attIn) == 3)
    {
        firstWord = malloc(sizeof(symbolNode));
        int len = 0;
        while(nameIn[len] != '\0')
            len++;
        len++; //count the null character as well
        firstWord->name = malloc(len*sizeof(char));
        int i;
        for(i = 0; i < len; i++)
            firstWord->name[i] = nameIn[i];

        firstWord->tokenType = typeIn;
        firstWord->attribute = attIn;
        symbolNode* currWord = firstWord;
        symbolNode* prevWord = currWord;
        printf("Got '%s\t%d\t%d'",currWord->name,currWord->tokenType,currWord->attribute);
        printf("\n");

        while(fscanf(wordsFile, fileFormat, nameIn, &typeIn, &attIn) == 3)
        {
            currWord = malloc(sizeof(symbolNode));

            len = 0;
            while(nameIn[len] != '\0')
                len++;
            len++; //count the null character as well
            currWord->name = malloc(len*sizeof(char));
            for(i = 0; i < len; i++)
                currWord->name[i] = nameIn[i];

            currWord->tokenType = typeIn;
            currWord->attribute = attIn;
            printf("Got '%s\t%d\t%d'",currWord->name,currWord->tokenType,currWord->attribute);
            printf("\n");
            prevWord->next = currWord;
            prevWord = currWord;
        }
        currWord->next = NULL;
    }
    printf("\n");
    fclose(wordsFile);
    return firstWord;
};

symbolNode* searchSymbolList(char *sName, symbolNode *currSym)
{
    while(currSym != NULL)
    {
        printf(".");
        if(strcmp(currSym->name, sName) == 0)
            return currSym;
        else
            currSym = currSym->next;
    }
    return NULL;
}

//name must be malloc'ed on the heap; it will be freed internally if it matches.
symbolNode* checkAddSymbol(char *sName, symbolNode *currSym)
{
    symbolNode* prevNode = currSym;
    currSym = currSym->next; //skip the dummy node
    if(currSym == NULL) //List is empty; create a first entry.
    {
        symbolNode* newSym = malloc(sizeof(symbolNode));
        newSym->name = sName;
        newSym->tokenType = ID_TYPE;
        newSym->attribute = NULL_ATT;
        newSym->next = NULL;
        prevNode->next = newSym;
        return newSym;
    }
    //otherwise, the list has at least one element
    while(currSym->next != NULL)
    {
        if(strcmp(currSym->name, sName) == 0)
        {
            free(sName);
            return currSym;
        }
		currSym = currSym->next;
    }
    //check the last item
    if(strcmp(currSym->name, sName) == 0)
    {
        free(sName);
        return currSym;
    }
    else
    {
        //entry is not in list, so create it
        symbolNode* newSym = malloc(sizeof(symbolNode));
        newSym->name = sName;
        newSym->tokenType = ID_TYPE;
        newSym->attribute = NULL_ATT;
        newSym->next = NULL;
        currSym->next = newSym;
        return newSym;
    }
};

int relopMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum)
{
	if(buffer[*f] == '<')
    {
		*f = *f + 1;
		tokenNode* nextToken = malloc(sizeof(tokenNode));
		lastToken->next = nextToken;
        nextToken->tokenType = RELOP_TYPE;
		nextToken->lineNo = lineNum;
		nextToken->next = NULL;
		if(buffer[*f] == '=')
		{
			nextToken->lexeme = "<=";
			nextToken->attribute = RELOP_LE;
			*f = *f + 1;
		}
		else if (buffer[*f] == '>')
		{
			nextToken->lexeme = "<>";
			nextToken->attribute = RELOP_NE;
			*f = *f + 1;
		}
		else
		{
			nextToken->lexeme = "<";
			nextToken->attribute = RELOP_LT;
		}
		*b = *f;
		return -1;
    }
    else if (buffer[*f] == '>')
    {
		*f = *f + 1;
		tokenNode* nextToken = malloc(sizeof(tokenNode));
		lastToken->next = nextToken;
        nextToken->tokenType = RELOP_TYPE;
		nextToken->lineNo = lineNum;
		nextToken->next = NULL;
		if(buffer[*f] == '=')
		{
			nextToken->lexeme = ">=";
			nextToken->attribute = RELOP_GE;
			*f = *f + 1;
		}
		else
		{
			nextToken->lexeme = ">";
			nextToken->attribute = RELOP_GT;
		}
		*b = *f;
		return -1;
    }
    else if (buffer[*f] == '=')
    {
		*f = *f + 1;
		tokenNode* nextToken = malloc(sizeof(tokenNode));
		lastToken->next = nextToken;
		nextToken->lineNo = lineNum;
		nextToken->lexeme = "=";
		nextToken->tokenType = RELOP_TYPE;
		nextToken->attribute = RELOP_EQ;
		nextToken->next = NULL;
		*b = *f;
        printf("Got relop %s\n",nextToken->lexeme);
		return -1;
    }
	*f = *b;
	return 0;
};
int idresMachine(char* buffer, int *f, int *b, tokenNode* lastToken, symbolNode* idList, symbolNode* rWordsList, int lineNum)
{
	if((buffer[*f] >= 'A' && buffer[*f] <= 'Z') || (buffer[*f] >= 'a' && buffer[*f] <= 'z'))
	{
        printf("\n[%c]",buffer[*f]);
		tokenNode* nextToken = malloc(sizeof(tokenNode));
		lastToken->next = nextToken;

		*f = *f + 1;
		while(((buffer[*f] >= 'A' && buffer[*f] <= 'Z') ||
			(buffer[*f] >= 'a' && buffer[*f] <= 'z') ||
			(buffer[*f] >= '0' && buffer[*f] <= '9')))
		{
            printf("[%c]",buffer[*f]);
			*f = *f + 1;
		}

		if(*f-*b > MAX_ID_LEN)
		{
            nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
            int i;
            for(i = 0; i < *f-*b; i++)
                nextToken->lexeme[i] = buffer[*b+i];
            nextToken->lexeme[*f-*b] = '\0';

            nextToken->tokenType = LEXERR_TYPE;
            nextToken->attribute = ID_TOOLONG;
            nextToken->lineNo = lineNum;
            nextToken->next = NULL;
            *b = *f;
            return -1;
		}

		char* actualId = malloc((*f-*b+1)*sizeof(char));
		int i;
		for(i = 0; i < *f-*b; i++) actualId[i] = buffer[*b+i];
		actualId[*f-*b] = '\0';

        printf("\nChecking reserved words list");
        symbolNode* symbolAddr = searchSymbolList(actualId,rWordsList);
        if(symbolAddr != NULL) //is it a reserved word?
        {
            nextToken->tokenType = symbolAddr->tokenType;
            nextToken->attribute = symbolAddr->attribute;
            printf("\n    Reserved word is %s\n",actualId);
        }
        else
        {
            symbolAddr = checkAddSymbol(actualId, idList);
            nextToken->tokenType = ID_TYPE;
            nextToken->attribute = (intptr_t)symbolAddr; //pointer to symbolNode is the attribute
            printf("\n    Checkaddsym for %s\n", symbolAddr->name);
        }
        nextToken->lexeme = symbolAddr->name;
        nextToken->lineNo = lineNum;
        nextToken->next = NULL;
        *b = *f;
        return -1;
	}
    return 0;
};
int wsMachine(char* buffer, int *f, int *b)
{
    if(*f < BUFF_LEN && (buffer[*f] == ' ' || buffer[*f] == '\t' || buffer[*f] == '\n'))
    {
        while(*f < BUFF_LEN && (buffer[*f] == ' ' || buffer[*f] == '\t'))
        {
            *f = *f + 1;
        }
        *b = *f;
        if(buffer[*f] == '\n')
        {
            return START_NEWLINE;
        }
        return 0;
    }
    else if (*f == BUFF_LEN)
        return START_NEWLINE;
    else
        return 0;
};
int catchallMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum)
{
	tokenNode* nextToken = malloc(sizeof(tokenNode));
	nextToken->lineNo = lineNum;
	if(buffer[*f] == '+')
	{
		nextToken->lexeme = "+";
		nextToken->tokenType = ADDOP_TYPE;
		nextToken->attribute = ADDOP_PLUS;
	}
	else if (buffer[*f] == '-')
	{
		nextToken->lexeme = "-";
		nextToken->tokenType = ADDOP_TYPE;
		nextToken->attribute = ADDOP_MINUS;
	}
	else if (buffer[*f] == '*')
	{
		nextToken->lexeme = "*";
		nextToken->tokenType = MULOP_TYPE;
		nextToken->attribute = MULOP_MULT;
	}
	else if (buffer[*f] == '/')
	{
		nextToken->lexeme = "/";
		nextToken->tokenType = MULOP_TYPE;
		nextToken->attribute = MULOP_DIV;
	}
	else if (buffer[*f] == ':')
	{

		if(*f < BUFF_LEN-1 && buffer[*f+1] == '=')
		{
            *f = *f + 1;
            nextToken->lexeme = ":=";
			nextToken->tokenType = ASSIGNOP_TYPE;
			nextToken->attribute = NULL_ATT;
		}
		else
		{
			nextToken->lexeme = ":";
			nextToken->tokenType = COLON_TYPE;
			nextToken->attribute = NULL_ATT;
		}
	}
	else if (buffer[*f] == '.')
	{
		if(*f < BUFF_LEN-1 && buffer[*f+1] == '.')
		{
            *f = *f + 1;
			nextToken->lexeme = "..";
			nextToken->tokenType = DOTDOT_TYPE;
			nextToken->attribute = NULL_ATT;
		}
		else
		{
			nextToken->lexeme = ".";
			nextToken->tokenType = DOT_TYPE;
			nextToken->attribute = NULL_ATT;
		}
	}
	else if (buffer[*f] == ';')
	{
		nextToken->lexeme = ";";
		nextToken->tokenType = SEMICOLON_TYPE;
		nextToken->attribute = NULL_ATT;
	}
	else if (buffer[*f] == '(')
	{
		nextToken->lexeme = "(";
		nextToken->tokenType = LPARENS_TYPE;
		nextToken->attribute = NULL_ATT;
	}
	else if (buffer[*f] == ')')
	{
		nextToken->lexeme = ")";
		nextToken->tokenType = RPARENS_TYPE;
		nextToken->attribute = NULL_ATT;
	}
	else if (buffer[*f] == '[')
	{
		nextToken->lexeme = "[";
		nextToken->tokenType = LBRACKET_TYPE;
		nextToken->attribute = NULL_ATT;
	}
	else if (buffer[*f] == ']')
	{
		nextToken->lexeme = "]";
		nextToken->tokenType = RBRACKET_TYPE;
		nextToken->attribute = NULL_ATT;
	}
	else if (buffer[*f] == ',')
    {
        nextToken->lexeme = ",";
        nextToken->tokenType = COMMA_TYPE;
        nextToken->attribute = NULL_ATT;
    }
	else
	{
		free(nextToken);
		return 0;
	}
	*f = *f + 1;
	*b = *f;
	lastToken->next = nextToken;
	nextToken->next = NULL;
	printf("\nGot the punctuation %s\n",nextToken->lexeme);
	return 1;
};

//xxxxx.yyyyyE+-zz
int longrealMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum)
{
    if(buffer[*f] >= '0' && buffer[*f] <= '9')
    {
        tokenNode* currToken = lastToken;
        tokenNode* nextToken = malloc(sizeof(tokenNode));
        nextToken->lineNo = lineNum;
        nextToken->next = NULL;
        while(*f < BUFF_LEN && buffer[*f] >= '0' && buffer[*f] <= '9')
        {
            *f = *f + 1;
        }
        if (*f-*b > MAX_R_DEC_LEN)
        {
            *f=*b;
            free(nextToken);
            return 0; //block to real machine.
        }
        if(buffer[*b] == '0' && *f-*b > 1)
        {
            while(buffer[*f] >= '0' && buffer[*f] <= '9')
                *f = *f + 1;
            if (buffer[*f] == '.')
                *f = *f + 1;
            while(buffer[*f] >= '0' && buffer[*f] <= '9')
                *f = *f + 1;
            if(buffer[*f] == 'E')
                *f = *f + 1;
            if(buffer[*f] == '+' || buffer[*f] == '-')
                *f = *f + 1;
            while(buffer[*f] >= '0' && buffer[*f] <= '9')
                *f = *f + 1;

            nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
            int i;
            for(i = 0; i < *f-*b; i++)
                nextToken->lexeme[i] = buffer[*b+i];
            nextToken->lexeme[*f-*b] = '\0';

            nextToken->tokenType = LEXERR_TYPE;
            nextToken->attribute = NUM_ZEROSTART;
            nextToken->next = NULL;
            currToken->next = nextToken;
            *b = *f;
            return -1;
        }

        //FIRST PART OK

        if(buffer[*f] == '.')
        {
            *f = *f + 1;
            //must have 1-5 trailing digits
            if(buffer[*f] >= '0' && buffer[*f] <= '9')
            {
                int firstDec = *f;
                while(*f < BUFF_LEN && buffer[*f] >= '0' && buffer[*f] <= '9')
                {
                    *f = *f + 1;
                }
                if (*f-firstDec > MAX_R_DEC_LEN) //too many digits after the decimal. Not a real.
                {
                    if(buffer[*f] == 'E')
                        *f = *f + 1;
                    if(buffer[*f] == '+' || buffer[*f] == '-')
                        *f = *f + 1;
                    while(buffer[*f] >= '0' && buffer[*f] <= '9')
                        *f = *f + 1;

                    nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
                    int i;
                    for(i = 0; i < *f-*b; i++)
                        nextToken->lexeme[i] = buffer[*b+i];
                    nextToken->lexeme[*f-*b] = '\0';

                    nextToken->tokenType = LEXERR_TYPE;
                    nextToken->attribute = R_TOOMANY_Y;
                    nextToken->next = NULL;
                    currToken->next = nextToken;
                    *b = *f;
                    return -1;
                }

                if(buffer[*f] == 'E')
                {
                    *f = *f + 1;
                    //must have optional +-, and 1-2 trailing digits
                    if(buffer[*f] == '+' || buffer[*f] == '-')
                    {
                        *f = *f + 1;
                    }
                    if(buffer[*f] >= '0' && buffer[*f] <= '9')
                    {
                        int firstExp = *f;
                        while(*f < BUFF_LEN && buffer[*f] >= '0' && buffer[*f] <= '9')
                        {
                            *f = *f + 1;
                        }
                        if (*f-firstExp > MAX_R_EXP_LEN) //too many exponent digits
                        {
                            nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
                            int i;
                            for(i = 0; i < *f-*b; i++)
                                nextToken->lexeme[i] = buffer[*b+i];
                            nextToken->lexeme[*f-*b] = '\0';

                            nextToken->tokenType = LEXERR_TYPE;
                            nextToken->attribute = R_TOOMANY_Z;
                            nextToken->next = NULL;
                            currToken->next = nextToken;
                            *b = *f;
                            return -1;
                        }
                    }
                    else //an E and maybe a sign, but no digits; this is not a real.
                    {
                        *f=*b;
                        free(nextToken);
                        return 0; //block to real machine.
                    }
                }
                else //No E; this is not a longreal
                {
                    *f=*b;
                    free(nextToken);
                    return 0; //block to real machine.
                }
            }
            else //a period with no trailing numbers
            {
                *f=*b;
                free(nextToken);
                return 0; //block to int machine.
            }
        }
        else if(buffer[*f] == 'E')
        {
            *f = *f + 1;
            //must have optional +-, and 1-2 trailing digits
            if(buffer[*f] == '+' || buffer[*f] == '-')
            {
                *f = *f + 1;
            }
            if(buffer[*f] >= '0' && buffer[*f] <= '9')
            {
                int firstExp = *f;
                while(*f < BUFF_LEN && buffer[*f] >= '0' && buffer[*f] <= '9')
                {
                    *f = *f + 1;
                }
                if (*f-firstExp > MAX_R_EXP_LEN) //too many digits? not a longreal
                {
                    nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
                    int i;
                    for(i = 0; i < *f-*b; i++)
                        nextToken->lexeme[i] = buffer[*b+i];
                    nextToken->lexeme[*f-*b] = '\0';

                    nextToken->tokenType = LEXERR_TYPE;
                    nextToken->attribute = R_TOOMANY_Z;
                    nextToken->next = NULL;
                    currToken->next = nextToken;
                    *b = *f;
                    return -1;
                }
            }
            else //an E and maybe a sign, but no digits; this is not a real.
            {
                *f=*b;
                free(nextToken);
                return 0;
            }
        }
        else //No . or E; this is an int
        {
            *f=*b;
            free(nextToken);
            return 0; //block; this is not a real.
        }

        //Tests passed; we have a longreal.
        currToken->next = nextToken;
        nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
        int i;
        for(i = 0; i < *f-*b; i++)
            nextToken->lexeme[i] = buffer[*b+i];
        nextToken->lexeme[*f-*b] = '\0';
        nextToken->tokenType = LONGREAL_TYPE;
        nextToken->attribute = NULL_ATT;
        printf("Got longreal %s\n",nextToken->lexeme);
        return 1;
    }
    else
        return 0; //block; not a num.
};

//xxxxx.yyyyy
int realMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum)
{
    if(buffer[*f] >= '0' && buffer[*f] <= '9')
    {
        tokenNode* currToken = lastToken;
        tokenNode* nextToken = malloc(sizeof(tokenNode));

        while(*f < BUFF_LEN && buffer[*f] >= '0' && buffer[*f] <= '9')
        {
            *f = *f + 1;
        }
        if (*f-*b > MAX_R_NUM_LEN)
        {
            *f = *b;
            free(nextToken);
            return 0; //block, this might be an integer, which allows more consecutive digits
        }
        if(buffer[*b] == '0' && *f-*b > 1)
        {
            if(buffer[*f] == '.')
            {
                *f = *f+1;
                while(buffer[*f] >= '0' && buffer[*f] <= '9')
                    *f = *f + 1;
            }
            nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
            int i;
            for(i = 0; i < *f-*b; i++)
                nextToken->lexeme[i] = buffer[*b+i];
            nextToken->lexeme[*f-*b] = '\0';

            nextToken->tokenType = LEXERR_TYPE;
            nextToken->attribute = NUM_ZEROSTART;
            nextToken->next = NULL;
            currToken->next = nextToken;
            *b = *f;
            return -1;
        }

        //FIRST PART OK

        if(buffer[*f] == '.')
        {
            *f = *f + 1;
            //must have 1-5 trailing digits
            if(buffer[*f] >= '0' && buffer[*f] <= '9')
            {
                int firstDec = *f;
                while(*f < BUFF_LEN && buffer[*f] >= '0' && buffer[*f] <= '9')
                {
                    *f = *f + 1;
                }
                if (*f-firstDec > MAX_R_DEC_LEN) //too many digits after the decimal. Not a real.
                {
                    nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
                    int i;
                    for(i = 0; i < *f-*b; i++)
                        nextToken->lexeme[i] = buffer[*b+i];
                    nextToken->lexeme[*f-*b] = '\0';

                    nextToken->tokenType = LEXERR_TYPE;
                    nextToken->attribute = R_TOOMANY_Y;
                    nextToken->next = NULL;
                    currToken->next = nextToken;
                    *b = *f;
                    return -1;
                }
            }
            else //an int with a period
            {
                *f = *b;
                free(nextToken);
                return 0; //block to int
            }
        }
        else //this is just an int
        {
            *f = *b;
            free(nextToken);
            return 0;
        }

        //Tests passed; we have a real.
        currToken->next = nextToken;
        nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
        int i;
        for(i = 0; i < *f-*b; i++)
            nextToken->lexeme[i] = buffer[*b+i];
        nextToken->lexeme[*f-*b] = '\0';
        nextToken->tokenType = REAL_TYPE;
        nextToken->attribute = NULL_ATT;
        nextToken->lineNo = lineNum;
        nextToken->next = NULL;
        *b = *f;
        printf("Got real %s\n",nextToken->lexeme);
        return 1;
    }
    else
        return 0; //not a num; block to another machine
};

int intMachine(char* buffer, int *f, int *b, tokenNode* lastToken, int lineNum)
{
    if(buffer[*f] >= '0' && buffer[*f] <= '9')
    {
        tokenNode* currToken = lastToken;
        tokenNode* nextToken = malloc(sizeof(tokenNode));
        while(*f < BUFF_LEN && buffer[*f] >= '0' && buffer[*f] <= '9')
            *f = *f + 1;

        if(buffer[*b] == '0' && *f-*b > 1)
        {
            nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
            int i;
            for(i = 0; i < *f-*b; i++)
                nextToken->lexeme[i] = buffer[*b+i];
            nextToken->lexeme[*f-*b] = '\0';

            nextToken->tokenType = LEXERR_TYPE;
            nextToken->attribute = NUM_ZEROSTART;
            nextToken->next = NULL;
            currToken->next = nextToken;
            *b = *f;
            return -1;
        }
        else if(*f-*b > MAX_INT_LEN)
        {
            nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
            int i;
            for(i = 0; i < *f-*b; i++)
                nextToken->lexeme[i] = buffer[*b+i];
            nextToken->lexeme[*f-*b] = '\0';

            nextToken->tokenType = LEXERR_TYPE;
            nextToken->attribute = INT_TOOLONG;
            nextToken->next = NULL;
            currToken->next = nextToken;
            *b = *f;
            return -1;
        }

        currToken->next = nextToken;
        nextToken->lexeme = malloc((*f-*b+1)*sizeof(char));
        int i;
        for(i = 0; i < *f-*b; i++)
        {
            nextToken->lexeme[i] = buffer[*b + i];
        }
        nextToken->lexeme[*f-*b] = '\0';
        nextToken->tokenType = INT_TYPE;
        nextToken->attribute = NULL_ATT;
        nextToken->lineNo = lineNum;
        nextToken->next = NULL;
        *b = *f;
        printf("Got int %s\n",nextToken->lexeme);
        return 1;
    }
    return 0; //block to another machine
};

char* getTokenMeaning(int tType, symbolNode* resList)
{
	switch(tType)
	{
		case LEXERR_TYPE:
			return "LexErr";
		case ADDOP_TYPE:
			return "AddOp";
		case MULOP_TYPE:
			return "MulOp";
		case RELOP_TYPE:
			return "RelOp";
		case ASSIGNOP_TYPE:
            return "AssignOp";
		case WS_TYPE:
			return "WhiteSpace";
		case ID_TYPE:
			return "Id";
		case COLON_TYPE:
			return "Colon";
		case SEMICOLON_TYPE:
			return "Semicolon";
		case DOT_TYPE:
			return "Dot";
		case DOTDOT_TYPE:
			return "DotDot";
		case LPARENS_TYPE:
			return "Left Parens";
		case RPARENS_TYPE:
			return "Right Parens";
		case LBRACKET_TYPE:
			return "Left Bracket";
		case RBRACKET_TYPE:
			return "Right Bracket";
        case COMMA_TYPE:
            return "Comma";
		case INT_TYPE:
			return "Int";
		case REAL_TYPE:
			return "Real";
		case LONGREAL_TYPE:
			return "LongReal";
        case EOF_TYPE:
            return "EOF";
		default:
			while (resList != NULL)
			{
				if(resList->tokenType == tType)
				{
					return resList->name;
				}
				else
					resList = resList->next;
			}

			//If all else fails...
			return "Unknown Token Type";
	}
};

char* getErrorMeaning(int eType)
{
    switch(eType)
    {
        case ID_TOOLONG:
            return "ID Too Long";
        case NUM_ZEROSTART:
            return "Leading Zero(s)";
        case INT_TOOLONG:
            return "Int Too Long";
        case R_TOOMANY_X:
            return "Dec. Too Long";
        case R_TOOMANY_Y:
            return "Fract. Too Long";
        case R_TOOMANY_Z:
            return "Exp. Too Long";
        case R_NO_Z:
            return "No Exp. Digits";
        case UNDEF_CHAR:
            return "Undefined Character";
        default:
            return "Unknown Error Type";
    }
};

void makeTokenFile(tokenNode* firstToken, symbolNode* resList)
{
    //output tokenNode file, with all erroneous lexemes
    FILE *tokenPtr; FILE *tokenFilePtr;
    tokenPtr = fopen("tokenListing.txt", "w");
    tokenFilePtr = fopen("tokenFile.txt", "w");
    fprintf(tokenPtr, "%*s\t|\t%*s\t|\t%*s\t|\t%*s\n", 10, "Line No.", 10, "Lexeme", 13, "TokenType (meaning)", 10, "Attribute");
    fprintf(tokenPtr,"----------------------------------------------------------------------------------------------------\n");
    while (firstToken != NULL)
    {
		char* tokenMeaning = getTokenMeaning(firstToken->tokenType, resList);
        fprintf (tokenPtr, "%*d\t\t%*s\t%*d (%*s)\t\t",
						5, firstToken->lineNo,
                        20, firstToken->lexeme,
						10, firstToken->tokenType,
						13, tokenMeaning);
        fprintf (tokenFilePtr, "%d\t%s\t%d\t",
						firstToken->lineNo,
                        firstToken->lexeme,
						firstToken->tokenType);
        if(firstToken->tokenType == ID_TYPE)
        {
            fprintf(tokenPtr, "%p (ptr to sym tab)\n", (void*)firstToken->attribute);
            fprintf(tokenFilePtr, "%p\n", (void*)firstToken->attribute);
        }
        else if (firstToken->tokenType == LEXERR_TYPE)
        {
            fprintf(tokenPtr, "%d (%s)\n", firstToken->attribute, getErrorMeaning(firstToken->attribute));
            fprintf(tokenFilePtr, "%d\n", firstToken->attribute);
        }
        else if (firstToken->attribute == NULL_ATT)
        {
            fprintf(tokenPtr, "%d (%s)\n", NULL_ATT, "NULL");
            fprintf(tokenFilePtr, "%d\n", NULL_ATT);
        }
        else
        {
            fprintf(tokenPtr, "%d\n", firstToken->attribute);
            fprintf(tokenFilePtr, "%d\n", firstToken->attribute);
        }
		firstToken = firstToken->next;
    }
    fclose(tokenPtr);
	return;
};

//Prints a file with user-defined symbols (id's)
void makeSymbolFile(symbolNode* currNode)
{
    FILE *symbolPtr;
    symbolPtr = fopen("symbolTable.txt", "w");
    while (currNode != NULL)
	{
		fprintf(symbolPtr, "%*s\t%p\n", 10, currNode->name, currNode);
		currNode = currNode->next;
	}
    fclose(symbolPtr);
};

void cleanUp(symbolNode* currNode, tokenNode* currToken, symbolNode* idList)
{
	symbolNode* nextNode;
	while(currNode != NULL)
	{
		nextNode = currNode->next;
		free(currNode->name);
		free(currNode);
		currNode = nextNode;
	}

	tokenNode* nextToken;
	while(currToken != NULL)
	{
		nextToken = currToken->next;
		free(currToken);
		currToken = nextToken;
	}

	while(currNode != NULL)
	{
	    nextNode = idList->next;
	    free(idList);
	    idList = nextNode;
	}
};

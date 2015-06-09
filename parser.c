/*
 * Parser with Syntax Analysis
 * Anton Ridgway
 * CS4013 (Compilers)
 * December 18, 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define BUFFLEN 72

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
#define NUM_TYPE 20

#define ARRAY_TYPE 50
#define BEGIN_TYPE 51
#define DO_TYPE 52
#define ELSE_TYPE 53
#define END_TYPE 54
#define FUNCTION_TYPE 55
#define IF_TYPE 56
#define INTWORD_TYPE 57
#define NOT_TYPE 58
#define OF_TYPE 59
#define PROCEDURE_TYPE 60
#define PROGRAM_TYPE 61
#define REALWORD_TYPE 62
#define THEN_TYPE 63
#define VAR_TYPE 64
#define WHILE_TYPE 65

#define NULL_ATT 0
#define RELOP_LE 1
#define RELOP_NE 2
#define RELOP_LT 3
#define RELOP_GE 4
#define RELOP_GT 5
#define RELOP_EQ 6
#define ADDOP_PLUS 1
#define ADDOP_MINUS 2
#define ADDOP_OR 3
#define MULOP_MULT 1
#define MULOP_DIV 2
#define MULOP_DIVWORD 3
#define MULOP_MOD 4
#define MULOP_AND 5

//productions' numbering
#define PGM_NUM 0
#define PROGRAMFACT_NUM 1
#define PROGRAMFACT2_NUM 2
#define IDLST_NUM 3
#define IDLSTTAIL_NUM 4
#define DECLS_NUM 5
#define DECLSTAIL_NUM 6
#define TYPE_NUM 7
#define STDTYPE_NUM 8
#define SUBDECLS_NUM 9
#define SUBDECLSTAIL_NUM 10
#define SDECL_NUM 11
#define SDECLFACT_NUM 12
#define SDECLFACT2_NUM 13
#define SUBHEAD_NUM 14
#define SUBHEADFACT_NUM 15
#define ARGUMENTS_NUM 16
#define PARALIST_NUM 17
#define PARALISTTAIL_NUM 18
#define CPDSTMT_NUM 19
#define CPDSTMTFACT_NUM 20
#define STMTLIST_NUM 21
#define STMTLISTTAIL_NUM 22
#define STMT_NUM 23
#define STMTTAIL_NUM 24
#define VARIABLE_NUM 25
#define VARTAIL_NUM 26
#define EXPRLIST_NUM 27
#define EXPRLISTTAIL_NUM 28
#define EXPR_NUM 29
#define EXPRTAIL_NUM 30
#define SMPEXPR_NUM 31
#define SMPEXPRTAIL_NUM 32
#define TERM_NUM 33
#define TERMTAIL_NUM 34
#define FACTOR_NUM 35
#define FACTORFACT_NUM 36
#define SIGN_NUM 37

//type-checking variables
#define T_PGNAME 0
#define T_PGPARM 1
#define T_FNAME 2
#define T_FPINT 3
#define T_FPREAL 4
#define T_FPAINT 5
#define T_FPAREAL 6
#define T_INT 7
#define T_REAL 8
#define T_AINT 9
#define T_AREAL 10
#define T_BOOL 11
#define T_ERR 12

//memory management definitions
#define INT_SIZE 4
#define REAL_SIZE 8

struct tokenNode
{
    char* lexeme;
    int tokenType;
    intptr_t attribute;
    int lineNo;
	struct tokenNode* next;
};
struct symbolNode //scoping symbol table nodes
{
    char* name;
    int type;
    int memLocation;
    struct symbolNode* nextSameNode; //for elements in the same scope
    struct symbolNode* nextLowerNode; //for elements in a lower scope
    struct symbolNode* prevNode;

    //green nodes only
    int returnType; //-1 if blue node
    int numParams;
};
struct gsNode //green node stack for scope analysis
{
    char* name;
    int prevOffset;
    struct gsNode* prevNode; //previous node on the stack
    struct symbolNode* thisNode; //associated green node in the symbol table
};
struct typeAtt
{
    int type;
    int width;
};
typedef struct tokenNode tokenNode;
typedef struct symbolNode symbolNode;
typedef struct gsNode gsNode;
typedef struct typeAtt typeAtt;

tokenNode* loadTokens(FILE* wordsFile);
void printListing();
void cleanUp();
void freeTree(symbolNode* rootNode);
void synerr(char* expectedTok, char* currVar, char* follows);
tokenNode* match(int matchType, char* currVar, char* follows);
void parse();
void pgm();
void programFact(symbolNode* pfIn);
void programFact2(symbolNode* pf2In);
symbolNode* idlst(symbolNode* idlstIn);
symbolNode* idlstTail(symbolNode* idlstTailIn);
symbolNode* decls(symbolNode* declsIn);
symbolNode* declsTail(symbolNode* declsTailIn);
typeAtt* type();
typeAtt* stdType();
symbolNode* subdecls(symbolNode* subdeclsIn);
symbolNode* subdeclsTail(symbolNode* subdeclsTailIn);
symbolNode* sdecl(symbolNode* sdeclIn, int attachMode);
symbolNode* sdeclFact(symbolNode* sdeclFactIn);
symbolNode* sdeclFact2(symbolNode* sdeclFact2In);
symbolNode* subhead(symbolNode* subheadIn, int attachMode);
symbolNode* subheadFact(symbolNode* subheadFactIn);
symbolNode* arguments(symbolNode* argumentsIn);
symbolNode* paraList(symbolNode* paraListIn);
symbolNode* paraListTail(symbolNode* paraListTailIn);
void cpdstmt();
void cpdstmtFact();
void stmtList();
void stmtListTail();
void stmt();
void stmtTail();
int variable();
int varTail(tokenNode* varTailIn);
int exprList(symbolNode* exprListIn);
int exprListTail(int exprListTailIn);
int expr();
int exprTail(int exprTailIn);
int smpExpr();
int smpExprTail(int smpExprTailIn);
int term();
int termTail(int termTailIn);
int factor();
int factorFact(symbolNode* factorFactIn);
void sign();

char* typeToString(int typeIn);

symbolNode* findBlueNode(char* id, symbolNode* currNode);
symbolNode* findGreenNode(char* id, symbolNode* currNode);
symbolNode* checkAddBlueNode(char* nameIn, int typeIn, symbolNode* pNode, int memLoc);
symbolNode* checkAddGreenNode(char* nameIn, int typeIn, symbolNode* pNode, int attachMode);

symbolNode* popGreenStack();
void pushGreenStack(symbolNode* pushMe);
symbolNode* traceTree(symbolNode* root);
symbolNode* traceTreeBelow(symbolNode* sTN);
int getType(char* name);
int getNumParams(symbolNode* a);
int getParamType(int i, symbolNode* a);


tokenNode *currToken;
FILE *sourceLFile;
FILE *outputLFile;
int currLine = 0;
char* currLineS;

gsNode* gnStack = NULL; //scope checking; top node on the green node stack
FILE *symMemTableFile;//memory address computation
int memAddress = 0;
int numTabs = 0;
symbolNode* symbolTableEye = NULL; //type checking, declarations processing

int main(int argc, char *argv[])
{
    printf("Loading tokens from tokenFile.txt...\n");
    //open source file for reading, listing for writing
    FILE* tokenPtr;
	tokenPtr = fopen("tokenFile.txt", "r");
    if(tokenPtr == NULL)
    {
        fprintf(stderr,"File-open has failed. Is the token file present?\n");
        return 0;
    }
    currToken = loadTokens(tokenPtr);
    fclose(tokenPtr);

	printf( "Parsing source file...\n" );
    sourceLFile = fopen("listing.txt", "r");
    if(sourceLFile == NULL)
    {
        fprintf(stderr,"File-open has failed. Is the lexical analyzer's listing file present?\n");
        cleanUp();
        return 0;
    }
    outputLFile = fopen("listing2.txt", "w");
    if(outputLFile == NULL)
    {
        fprintf(stderr,"Listing output file-open has failed. Do you have write permissions?\n");
        fclose(sourceLFile);
        cleanUp();
        return 0;
    }
    symMemTableFile = fopen("symMemTable.txt", "w");
    if(symMemTableFile == NULL)
    {
        fprintf(stderr,"Symbol table output file-open failed. Do you have write permissions?\n");
        fclose(outputLFile);
        fclose(sourceLFile);
        cleanUp();
        return 0;
    }

    currLineS = malloc(BUFFLEN+1);
    printListing();
	parse();
	fclose(sourceLFile);
	fclose(outputLFile);
	fclose(symMemTableFile);
    free(currLineS);

    while(symbolTableEye->prevNode != NULL)
        symbolTableEye = symbolTableEye->prevNode;
    traceTree(symbolTableEye);

	printf( "Cleaning up heap memory...\n" );
	cleanUp();
	printf( "Parsing is done!\n" );
    return 0;
};

tokenNode* loadTokens(FILE* wordsFile)
{
	char fileFormat[] = "%d\t%s\t%d\t%p";
	int lineIn;
    char lexemeIn[BUFFLEN+1];
    int typeIn;
    int attIn;
    tokenNode* firstTok = NULL;

    if(fscanf(wordsFile, fileFormat, &lineIn, lexemeIn, &typeIn, &attIn) == 4)
    {
        firstTok = malloc(sizeof(tokenNode));
        int len = 0;
        while(lexemeIn[len] != '\0')
            len++;
        len++; //count the null character as well
        firstTok->lexeme = malloc(len*sizeof(char));
        int i;
        for(i = 0; i < len; i++)
            firstTok->lexeme[i] = lexemeIn[i];

        firstTok->lineNo = lineIn;
        firstTok->tokenType = typeIn;
        firstTok->attribute = attIn;
        tokenNode* newToken = firstTok;
        tokenNode* prevToken = newToken;
        printf("Got '%d\t%s\t%d\t%X'\n",newToken->lineNo,newToken->lexeme,newToken->tokenType,newToken->attribute);

        while(fscanf(wordsFile, fileFormat, &lineIn, lexemeIn, &typeIn, &attIn) == 4)
        {
            newToken = malloc(sizeof(tokenNode));

            len = 0;
            while(lexemeIn[len] != '\0')
                len++;
            len++; //count the null character as well
            newToken->lexeme = malloc(len*sizeof(char));
            for(i = 0; i < len; i++)
                newToken->lexeme[i] = lexemeIn[i];

            newToken->lineNo = lineIn;
            newToken->tokenType = typeIn;
            newToken->attribute = attIn;
            printf("Got '%d\t%s\t%d\t%X'\n",newToken->lineNo,newToken->lexeme,newToken->tokenType,newToken->attribute);
            prevToken->next = newToken;
            prevToken = newToken;
        }
        newToken->next = NULL;
    }
    printf("\n");
    return firstTok;
};

void printListing()
{
    if(currToken->lineNo >= currLine)
    {
        char currPrefix[10];
        if(currLine > 0) //already storing a string
        {
            fprintf(outputLFile, "%s", currLineS);
            //printf("%d %d print1 from line %d: %s\n",currLine,currToken->lineNo,currLine,currLineS);
        }
        do
        {
            if(!feof(sourceLFile))
            {
                fgets(currLineS,BUFFLEN+1,sourceLFile);
                sscanf(currLineS,"%s:%*s\n",currPrefix);
                if(currPrefix[0] > 47 && currPrefix[0] < 58) //digits
                {
                    currLine = atoi(currPrefix);
                    printf("Now printing up to and including line %d...\n",currLine);
                    if(currLine <= currToken->lineNo)
                    {
                        fprintf(outputLFile, "%s", currLineS);
                    }
                }
                else //error line
                {
                    fprintf(outputLFile, "%s", currLineS);
                }
            }
            else
            {
                currLine++; //end of file; stop printing
                fprintf(outputLFile, "\n");
            }
        } while(currLine <= currToken->lineNo && !feof(sourceLFile));
    }
};

void cleanUp()
{
    tokenNode* nextToken;
	while(currToken != NULL)
	{
		nextToken = currToken->next;
		free(currToken);
		currToken = nextToken;
	}
	if(symbolTableEye != NULL)
    {
        while(symbolTableEye->prevNode != NULL)
            symbolTableEye = symbolTableEye->prevNode;
        freeTree(symbolTableEye);
    }
    while(gnStack != NULL)
    {
        popGreenStack();
    }
};

//recursive tree traversal
void freeTree(symbolNode* rootNode)
{
    if(rootNode->nextLowerNode != NULL)
        freeTree(rootNode->nextLowerNode);
    if(rootNode->nextSameNode != NULL)
        freeTree(rootNode->nextSameNode);
    free(rootNode);
}

void synerr(char* expectedTok, char* currVar, char* follows)
{
    fprintf(outputLFile,"SYNERR: in %s, expecting token %s, received token %s; skipping to one of: %s\n", currVar, expectedTok, currToken->lexeme, follows);
    printf("SYNERR: in %s, expecting token %s, received token %s; skipping to one of: %s\n", currVar, expectedTok, currToken->lexeme, follows);
    //skip to the next synchronizing token
    while(!(currToken == NULL || currToken->tokenType == EOF_TYPE
            //pgm, programFact1, and programFact2 each skip to EOF, which is already considered
            || (strcmp(currVar, "idlst")==0 && currToken->tokenType == RPARENS_TYPE)
            || (strcmp(currVar, "idlstTail")==0 && currToken->tokenType == RPARENS_TYPE)
            || (strcmp(currVar, "decls")==0 && (currToken->tokenType == FUNCTION_TYPE || currToken->tokenType == BEGIN_TYPE))
            || (strcmp(currVar, "declsTail")==0 && (currToken->tokenType == FUNCTION_TYPE || currToken->tokenType == BEGIN_TYPE))
            || (strcmp(currVar, "type")==0 && currToken->tokenType == SEMICOLON_TYPE)
            || (strcmp(currVar, "stdType")==0 && currToken->tokenType == SEMICOLON_TYPE)
            || (strcmp(currVar, "subdecls")==0 && currToken->tokenType == BEGIN_TYPE)
            || (strcmp(currVar, "subdeclsTail")==0 && currToken->tokenType == BEGIN_TYPE)
            || (strcmp(currVar, "sdecl")==0 && currToken->tokenType == SEMICOLON_TYPE)
            || (strcmp(currVar, "sdeclFact")==0 && currToken->tokenType == SEMICOLON_TYPE)
            || (strcmp(currVar, "sdeclFact2")==0 && currToken->tokenType == SEMICOLON_TYPE)
            || (strcmp(currVar, "subhead")==0 && (currToken->tokenType == VAR_TYPE || currToken->tokenType == FUNCTION_TYPE || currToken->tokenType == BEGIN_TYPE))
            || (strcmp(currVar, "subheadFact")==0 && (currToken->tokenType == VAR_TYPE || currToken->tokenType == FUNCTION_TYPE || currToken->tokenType == BEGIN_TYPE))
            || (strcmp(currVar, "arguments")==0 && currToken->tokenType == COLON_TYPE)
            || (strcmp(currVar, "paraList")==0 && currToken->tokenType == RPARENS_TYPE)
            || (strcmp(currVar, "paraListTail")==0 && currToken->tokenType == RPARENS_TYPE)
            || (strcmp(currVar, "cpdstmt")==0 && (currToken->tokenType == DOT_TYPE || currToken->tokenType == SEMICOLON_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE))
            || (strcmp(currVar, "cpdstmtFact")==0 && (currToken->tokenType == DOT_TYPE || currToken->tokenType == SEMICOLON_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE))
            || (strcmp(currVar, "stmtList")==0 && currToken->tokenType == END_TYPE)
            || (strcmp(currVar, "stmtListTail")==0 && currToken->tokenType == END_TYPE)
            || (strcmp(currVar, "stmt")==0 && (currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "stmtTail")==0 && (currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "variable")==0 && currToken->tokenType == ASSIGNOP_TYPE)
            || (strcmp(currVar, "varTail")==0 && currToken->tokenType == ASSIGNOP_TYPE)
            || (strcmp(currVar, "exprList")==0 && currToken->tokenType == RPARENS_TYPE)
            || (strcmp(currVar, "exprListTail")==0 && currToken->tokenType == RPARENS_TYPE)
            || (strcmp(currVar, "expr")==0 && (currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "exprTail")==0 && (currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "smpExpr")==0 && (currToken->tokenType == RELOP_TYPE || currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "smpExprTail")==0 && (currToken->tokenType == RELOP_TYPE || currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "term")==0 && (currToken->tokenType == ADDOP_TYPE || currToken->tokenType == RELOP_TYPE || currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "termTail")==0 && (currToken->tokenType == ADDOP_TYPE || currToken->tokenType == RELOP_TYPE || currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "factor")==0 && (currToken->tokenType == MULOP_TYPE || currToken->tokenType == ADDOP_TYPE || currToken->tokenType == RELOP_TYPE || currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "factorFact")==0 && (currToken->tokenType == MULOP_TYPE || currToken->tokenType == ADDOP_TYPE || currToken->tokenType == RELOP_TYPE || currToken->tokenType == THEN_TYPE || currToken->tokenType == DO_TYPE || currToken->tokenType == RBRACKET_TYPE || currToken->tokenType == RPARENS_TYPE || currToken->tokenType == COMMA_TYPE || currToken->tokenType == ELSE_TYPE || currToken->tokenType == END_TYPE || currToken->tokenType == SEMICOLON_TYPE))
            || (strcmp(currVar, "sign")==0 && (currToken->tokenType == ID_TYPE || currToken->tokenType == INT_TYPE || currToken->tokenType == REAL_TYPE || currToken->tokenType == LONGREAL_TYPE || currToken->tokenType == LPARENS_TYPE || currToken->tokenType == NOT_TYPE))
            ))
    {
        currToken = currToken->next;
    }
    printListing();
};

void semerr(char* error)
{
    fprintf(outputLFile, error);
    printListing();
}

tokenNode* match(int matchType, char* currVar, char* follows)
{
	if((currToken->tokenType == matchType && currToken->tokenType != EOF_TYPE)
    ||((currToken->tokenType == INT_TYPE || currToken->tokenType == REAL_TYPE || currToken->tokenType == LONGREAL_TYPE) && matchType == NUM_TYPE))
	{
	    printListing();
	    tokenNode* matchedToken = currToken;
		currToken = currToken->next;
		printf("Matching '%s' in var '%s'\n",typeToString(matchType),currVar);
		return matchedToken;
	}
	else if (currToken->tokenType == EOF_TYPE && matchType == EOF_TYPE)
	{
	    tokenNode* matchedToken = currToken;
	    currToken = NULL;
	    printf("Matching '%s' in var '%s'\n",typeToString(matchType),currVar);
	    return matchedToken;
	    //end of parse
	}
	else //tok != t
	{
	    printf("Failed match of type '%s' in var '%s'; actually found '%s'\n",typeToString(matchType),currVar,currToken->lexeme);
		synerr(typeToString(matchType), currVar, follows);
		return NULL;
	}
};

void parse()
{
	pgm();
	while(currToken != NULL)
    {
        match(EOF_TYPE, "EOF", "EOF");
    }
};

//program			program			program -> program id ( idlst ) ; programFact
void pgm()
{
	if(currToken->tokenType == PROGRAM_TYPE) //if it goes to E, NOP
	{
	    symbolNode* idlstS;
	    if(match(PROGRAM_TYPE, "pgm", "EOF") != NULL)
        {
            tokenNode* pgmToken = match(ID_TYPE, "pgm", "EOF");
            if(pgmToken != NULL)
            {
                symbolTableEye = checkAddGreenNode(pgmToken->lexeme, T_PGNAME, NULL, -1);
                pushGreenStack(symbolTableEye);
                memAddress = 0;

                if(match(LPARENS_TYPE, "pgm", "EOF") != NULL)
                {
                    idlstS = idlst(symbolTableEye);
                    if(match(RPARENS_TYPE, "pgm", "EOF") != NULL)
                    {
                        if(match(SEMICOLON_TYPE, "pgm", "EOF") != NULL)
                        {
                            programFact(idlstS);
                            popGreenStack();
                        }
        }   }   }   }
    }
	else
    {
        synerr("program", "pgm", "EOF");
    }
};

//programFact		var				programFact -> decls programFact2
//programFact		function			programFact -> subdecls cpdstmt .
//programFact		begin			programFact -> cpdstmt .
void programFact(symbolNode* pfIn)
{
	if(currToken->tokenType == VAR_TYPE)
	{
        symbolNode* declsS = decls(pfIn);
        programFact2(declsS);
	}
    else if (currToken->tokenType == FUNCTION_TYPE)
    {
        subdecls(pfIn);
        cpdstmt();
        match(DOT_TYPE,"programFact", "EOF");
    }
	else if (currToken->tokenType == BEGIN_TYPE)
    {
        cpdstmt();
        match(DOT_TYPE,"programFact", "EOF");
    }
    else
    {
        synerr("var, function, or begin", "programFact", "EOF");
    }
};

//programFact2		function			programFact2 -> subdecls cpdstmt .
//programFact2		begin				programFact2 -> cpdstmt .
void programFact2(symbolNode* pf2In)
{
	if (currToken->tokenType == FUNCTION_TYPE)
	{
        subdecls(pf2In);
        cpdstmt();
        match(DOT_TYPE,"programFact2", "EOF");
	}
	else if (currToken->tokenType == BEGIN_TYPE)
    {
        cpdstmt();
        match(COMMA_TYPE,"programFact2", "EOF");
    }
	else
    {
        synerr("function or begin", "programFact2", "EOF");
	}
};

//idlst				id				idlst -> id idlstTail
symbolNode* idlst(symbolNode* idlstIn)
{
	if (currToken->tokenType == ID_TYPE)
	{
	    tokenNode* currId = match(ID_TYPE, "idlst", ")");
        if(currId != NULL)
        {
            symbolNode* addedId = checkAddBlueNode(currId->lexeme, T_PGPARM, idlstIn, -1);
            symbolNode* idlstTailS;
            if(addedId == NULL)
            {
                fprintf(outputLFile,"SEMERR: in idlst; program parameter have repeated names\n");
                idlstTailS = idlstTail(idlstIn);
            }
            else
            {
                gnStack->thisNode->numParams++;
                symbolTableEye = addedId;
                idlstTailS = idlstTail(addedId);
            }
            return idlstTailS;
        }
	}
	else
    {
        synerr("id", "idlst", ")");
	}
    return idlstIn;
};

//idlstTail			,				idlstTail -> , id idlstTail
//idlstTail			)				idlstTail -> E
symbolNode* idlstTail(symbolNode* idlstTailIn)
{
	if (currToken->tokenType == COMMA_TYPE)
	{
        if(match(COMMA_TYPE, "idlstTail", ")")!=NULL)
        {
            tokenNode* currId = match(ID_TYPE, "idlstTail", ")");
            if(currId != NULL)
            {
                symbolNode* addedId = checkAddBlueNode(currId->lexeme, T_PGPARM, idlstTailIn, -1);
                symbolNode* idlstTailTailS;
                if(addedId == NULL)
                {
                    fprintf(outputLFile,"SEMERR: in idlstTail, program parameters have repeated names\n");
                    idlstTailTailS = idlstTail( idlstTailIn );
                }
                else
                {
                    gnStack->thisNode->numParams++;
                    symbolTableEye = addedId;
                    idlstTailTailS = idlstTail( addedId );
                }
                return idlstTailTailS;
            }
        }
	}
	else if (currToken->tokenType == RPARENS_TYPE)
    {
        return idlstTailIn;
    }
    else
    {
        synerr(", or )", "idlstTail", ")");
    }
    return idlstTailIn;
};

//decls				var				decls -> var id : type ; declsTail
symbolNode* decls(symbolNode* declsIn)
{
	if (currToken->tokenType == VAR_TYPE)
	{
        if(match(VAR_TYPE, "decls", "function, begin")!=NULL)
        {
            tokenNode* addedId = match(ID_TYPE, "decls", "function, begin");
            if(addedId!=NULL)
            {
                if(match(COLON_TYPE, "decls", "function, begin")!=NULL)
                {
                    typeAtt* typeInfo = type();
                    symbolNode* declsTailIn = checkAddBlueNode(addedId->lexeme, typeInfo->type, declsIn, memAddress);
                    if(declsTailIn == NULL)
                    {
                        fprintf(outputLFile,"SEMERR: in decls, variable name %s is already declared\n", addedId->lexeme);
                        declsTailIn = declsIn;
                    }
                    else
                    {
                        symbolTableEye = declsTailIn;
                        memAddress += typeInfo->width;
                    }
                    free(typeInfo);
                    if(match(SEMICOLON_TYPE, "decls", "function, begin")!=NULL)
                    {
                        return declsTail(declsTailIn);
                    }
                    else
                        return declsTailIn;
        }   }   }
	}
	else
    {
        synerr("var", "decls", "function, begin");
	}
	return declsIn;
};

/*declsTail			var				declsTail -> var id : type ; declsTail
declsTail			E				declsTail -> E*/
symbolNode* declsTail(symbolNode* declsTailIn)
{
	if (currToken->tokenType == VAR_TYPE)
	{
        if(match(VAR_TYPE,"declsTail", "function, begin")!=NULL)
        {
            tokenNode* addedId = match(ID_TYPE,"declsTail", "function, begin");
            if(addedId!=NULL)
            {
                if(match(COLON_TYPE,"declsTail", "function, begin")!=NULL)
                {
                    typeAtt* typeInfo = type();
                    symbolNode* declsTailTailIn = checkAddBlueNode(addedId->lexeme, typeInfo->type, declsTailIn, memAddress);
                    if(declsTailTailIn == NULL)
                    {
                        fprintf(outputLFile,"SEMERR: in declsTail, variable name %s is already declared\n", addedId->lexeme);
                        declsTailTailIn = declsTailIn;
                    }
                    else
                    {
                        symbolTableEye = declsTailTailIn;
                        memAddress += typeInfo->width;
                    }
                    free(typeInfo);
                    if(match(SEMICOLON_TYPE,"declsTail", "function, begin")!=NULL)
                        return declsTail(declsTailTailIn);
                    else
                        return declsTailTailIn;
        }   }   }
	}
	else if(currToken->tokenType == FUNCTION_TYPE ||
            currToken->tokenType == BEGIN_TYPE)
    {
        return declsTailIn;
    }
    else
    {
        synerr("var, function, or begin", "declsTail", "function, begin");
    }
    return declsTailIn;
};

//type				integer			type -> stdType
//type				real			type -> stdType
//type				array			type -> array [ num .. num ] of stdType
typeAtt* type()
{
    typeAtt* myType = malloc(sizeof(typeAtt));
	if (currToken->tokenType == INTWORD_TYPE || currToken->tokenType == REALWORD_TYPE)
	{
	    typeAtt* typeInfo = stdType();
	    myType->type = typeInfo->type;
	    myType->width = typeInfo->width;
	    free(typeInfo);
	    return myType;
    }
	else if (currToken->tokenType == ARRAY_TYPE)
    {
        if(match(ARRAY_TYPE,"type", ";")!=NULL)
        {
            if(match(LBRACKET_TYPE,"type", ";")!=NULL)
            {
                tokenNode* num1 = match(NUM_TYPE,"type", ";");
                if(num1!=NULL)
                {
                    if(match(DOTDOT_TYPE,"type", ";")!=NULL)
                    {
                        tokenNode* num2 = match(NUM_TYPE,"type", ";");
                        if(num2!=NULL)
                        {
                            if(match(RBRACKET_TYPE,"type", ";")!=NULL)
                            {
                                if(match(OF_TYPE,"type", ";")!=NULL)
                                {
                                    typeAtt* typeInfo = stdType();
                                    typeAtt* myType = malloc(sizeof(typeAtt));
                                    //we can assume for our purposes that num.val is 1
                                    if(num1->tokenType == INT_TYPE && num2->tokenType == INT_TYPE && atoi(num2->lexeme) > 1)
                                    {
                                        if(typeInfo->type == T_INT)
                                            myType->type = T_AINT;
                                        else if (typeInfo->type == T_REAL)
                                            myType->type = T_AREAL;
                                        else //ERR thrown earlier
                                            myType->type = T_ERR;
                                        myType->width = atoi(num2->lexeme) * typeInfo->width;
                                    }
                                    else
                                    {
                                        if(num1->tokenType != INT_TYPE || num2->tokenType != INT_TYPE)
                                            fprintf(outputLFile,"SEMERR: in type, array indices are not integers\n");
                                        else //bad range
                                            fprintf(outputLFile,"SEMERR: in type, array indices are not a valid range\n");
                                        myType->type = T_ERR;
                                        myType->width = 0;
                                    }
                                    free(typeInfo);
                                    return myType;
        }   }   }   }   }   }   }
    }
    else
    {
        synerr("integer, real, or array", "type", ";");
    }
    myType->type = T_ERR;
    myType->width = 0;
    return myType;
};

//stdType			integer			stdType -> integer
//stdType			real				stdType -> real
typeAtt* stdType()
{
    typeAtt* myType = malloc(sizeof(typeAtt));
	if (currToken->tokenType == INTWORD_TYPE)
	{
        match(INTWORD_TYPE,"stdType", ";");
        myType->type = T_INT;
        myType->width = INT_SIZE;
	}
	else if (currToken->tokenType == REALWORD_TYPE)
    {
        match(REALWORD_TYPE,"stdType", ";");
        myType->type = T_REAL;
        myType->width = REAL_SIZE;
    }
	else
    {
        synerr("integer or real", "stdType", ";");
        myType->type = T_ERR;
        myType->width = 0;
    }
    return myType;
};

//subdecls			function			subdecls -> sdecl ; subdeclsTail
symbolNode* subdecls(symbolNode* subdeclsIn)
{
	if (currToken->tokenType == FUNCTION_TYPE)
	{
        symbolNode* sdeclS = sdecl(subdeclsIn, 0);
        if(match(SEMICOLON_TYPE, "subdecls", "begin")!=NULL)
        {
            return subdeclsTail(sdeclS);
        }
        return sdeclS;
	}
	else
    {
        synerr("function", "subdecls", "begin");
        return subdeclsIn;
    }
};

/*subdeclsTail		function			subdeclsTail -> sdecl ; subdeclsTail
subdeclsTail		begin			subdeclsTail -> sdecl ;*/
symbolNode* subdeclsTail(symbolNode* subdeclsTailIn)
{
	if (currToken->tokenType == FUNCTION_TYPE)
	{
        symbolNode* sdeclS = sdecl(subdeclsTailIn, 1);
        if(match(SEMICOLON_TYPE, "subdeclsTail", "begin")!=NULL)
        {
            return subdeclsTail(sdeclS);
        }
        else
            return sdeclS;
	}
	else if (currToken->tokenType == BEGIN_TYPE)
    {
        return subdeclsTailIn;
    }
    else
    {
        synerr("function or begin", "subdeclsTail", "begin");
        return subdeclsTailIn;
	}
};

//sdecl				function			sdecl -> subhead sdeclFact
symbolNode* sdecl(symbolNode* sdeclIn, int attachMode)
{
	if (currToken->tokenType == FUNCTION_TYPE)
	{
        symbolNode* subheadS = subhead(sdeclIn, attachMode);
        symbolNode* sdeclFactS = sdeclFact(subheadS);
        symbolTableEye = gnStack->thisNode;
        popGreenStack(); //pushed in subhead
        return sdeclFactS;
	}
    else
    {
        synerr("function", "sdecl", ";");
        return sdeclIn;
    }
};

/*sdeclFact			var				sdeclFact -> decls sdeclFact2
sdeclFact			function			sdeclFact -> subdecls cpdstmt
sdeclFact			begin			sdeclFact -> cpdstmt*/
symbolNode* sdeclFact(symbolNode* sdeclFactIn)
{
	if (currToken->tokenType == VAR_TYPE)
    {
        symbolNode* declsS = decls(sdeclFactIn);
        return sdeclFact2(declsS);
    }
    else if (currToken->tokenType == FUNCTION_TYPE)
    {
        symbolNode* subdeclsS = subdecls(sdeclFactIn);
        cpdstmt();
        return subdeclsS;
    }
    else if (currToken->tokenType == BEGIN_TYPE)
    {
        cpdstmt();
        return sdeclFactIn;
    }
    else
	{
        synerr("var, function, or begin", "sdeclFact", ";");
        return sdeclFactIn;
	}
};

/*sdeclFact2			function			sdeclFact2 -> subdecls cpdstmt
sdeclFact2			begin			sdeclFact2 -> cpdstmt*/
symbolNode* sdeclFact2(symbolNode* sdeclFact2In)
{
	if (currToken->tokenType == FUNCTION_TYPE)
	{
        symbolNode* subdeclsS = subdecls(sdeclFact2In);
        cpdstmt();
        return subdeclsS;
	}
	else if (currToken->tokenType == BEGIN_TYPE)
    {
        cpdstmt();
        return sdeclFact2In;
    }
	else
    {
		synerr("function or begin", "sdeclFact2", ";");
		return sdeclFact2In;
    }
};

//subhead			function			subhead -> function id subheadFact
symbolNode* subhead(symbolNode* subheadIn, int attachMode)
{
	if (currToken->tokenType == FUNCTION_TYPE)
	{
        if(match(FUNCTION_TYPE, "subhead","var, function, or begin")!=NULL)
        {
            tokenNode* addedId = match(ID_TYPE, "subhead","var, function, or begin");
            if(addedId!=NULL)
            {
                symbolNode* addedNode = checkAddGreenNode(addedId->lexeme, T_FNAME, subheadIn, attachMode);
                if(addedNode != NULL)
                {
                    pushGreenStack(addedNode); //popped in sdecl
                    symbolTableEye = addedNode;
                    symbolNode* subheadFactS = subheadFact(addedNode);
                    return subheadFactS;
                }
                else
                {
                    fprintf(outputLFile,"SEMERR: in subhead, function name %s is already declared\n", addedId->lexeme);
                    subheadFact(NULL);
                    return subheadIn;
                }
            }
        }
	}
	else
        synerr("function","subhead","var, function, or begin");
    return subheadIn;
};

/*subheadFact		(				subheadFact -> arguments : stdType ;
subheadFact		:				subheadFact -> : stdType ;*/
symbolNode* subheadFact(symbolNode* subheadFactIn)
{
	if (currToken->tokenType == LPARENS_TYPE)
	{
        symbolNode* argumentsS = arguments(subheadFactIn);
        if(match(COLON_TYPE,"subheadFact","var, function, or begin")!=NULL)
        {
            typeAtt* typeInfo = stdType();
            if(subheadFactIn != NULL)
                gnStack->thisNode->returnType = typeInfo->type;
            free(typeInfo);
            match(SEMICOLON_TYPE,"subheadFact","var, function, or begin");
        }
        return argumentsS;
	}
	else if (currToken->tokenType == COLON_TYPE)
    {
        if(match(COLON_TYPE,"subheadFact","var, function, or begin")!=NULL)
        {
            typeAtt* typeInfo = stdType();
            match(SEMICOLON_TYPE,"subheadFact","var, function, or begin");
            if(subheadFactIn != NULL)
                gnStack->thisNode->returnType = typeInfo->type;
            free(typeInfo);
        }
        if(subheadFactIn != NULL)
            gnStack->thisNode->numParams = 0;
        return subheadFactIn;
    }
    else
    {
        synerr("( or :", "subheadFact","var, function, or begin");
        return subheadFactIn;
    }
};

//arguments			(				arguments -> ( paraList )
symbolNode* arguments(symbolNode* argumentsIn)
{
	if (currToken->tokenType == LPARENS_TYPE)
	{
        if(match(LPARENS_TYPE,"arguments",":")!=NULL)
        {
            symbolNode* paraListS = paraList(argumentsIn);
            match(RPARENS_TYPE,"arguments",":");
            return paraListS;
        }
	}
	else
    {
        synerr("(","arguments",":");
    }
    return argumentsIn;
};

//paraList			id				paraList -> id : type paraListTail
symbolNode* paraList(symbolNode* paraListIn)
{
    gnStack->thisNode->numParams = 0;
	if (currToken->tokenType == ID_TYPE)
	{
	    tokenNode* idAdded = match(ID_TYPE,"paraList",")");
        if(idAdded!=NULL)
        {
            if(match(COLON_TYPE,"paraList",")")!=NULL)
            {
                typeAtt* typeInfo = type();
                symbolNode* addedNode = NULL;
                if(paraListIn != NULL)
                {
                    if(typeInfo->type == T_INT)
                        addedNode = checkAddBlueNode(idAdded->lexeme, T_FPINT, paraListIn, -1);
                    else if (typeInfo->type == T_REAL)
                        addedNode = checkAddBlueNode(idAdded->lexeme, T_FPREAL, paraListIn, -1);
                    else if (typeInfo->type == T_AINT)
                        addedNode = checkAddBlueNode(idAdded->lexeme, T_FPAINT, paraListIn, -1);
                    else if (typeInfo->type == T_AREAL)
                        addedNode = checkAddBlueNode(idAdded->lexeme, T_FPAREAL, paraListIn, -1);
                    else //ERR
                        addedNode = checkAddBlueNode(idAdded->lexeme, T_ERR, paraListIn, -1);
                    if(addedNode == NULL)
                    {
                        fprintf(outputLFile,"SEMERR: in paraList, repeated parameter names\n");
                        addedNode = paraListIn;
                    }
                    else
                    {
                        gnStack->thisNode->numParams++;
                        symbolTableEye = addedNode;
                    }
                    free(typeInfo);
                    return paraListTail(addedNode);
                }
                else
                {
                    free(typeInfo);
                    return paraListTail(paraListIn);
                }
            }
        }
	}
	else
    {
        synerr("(","paraList",")");
	}
    return paraListIn;
};

/*paraListTail		;				paraListTail -> ; id : type paraListTail
paraListTail		)				paraListTail -> E*/
symbolNode* paraListTail(symbolNode* paraListTailIn)
{
	if (currToken->tokenType == SEMICOLON_TYPE)
	{
        if(match(SEMICOLON_TYPE,"paraListTail",")")!=NULL)
        {
            tokenNode* idAdded = match(ID_TYPE,"paraListTail",")");
            if(idAdded!=NULL)
            {
                if(match(COLON_TYPE,"paraListTail",")")!=NULL)
                {
                    typeAtt* typeInfo = type();
                    symbolNode* addedNode = NULL;
                    if(paraListTailIn != NULL)
                    {
                        if(typeInfo->type == T_INT)
                            addedNode = checkAddBlueNode(idAdded->lexeme, T_FPINT, paraListTailIn, -1);
                        else if (typeInfo->type == T_REAL)
                            addedNode = checkAddBlueNode(idAdded->lexeme, T_FPREAL, paraListTailIn, -1);
                        else if (typeInfo->type == T_AINT)
                            addedNode = checkAddBlueNode(idAdded->lexeme, T_FPAINT, paraListTailIn, -1);
                        else if (typeInfo->type == T_AREAL)
                            addedNode = checkAddBlueNode(idAdded->lexeme, T_FPAREAL, paraListTailIn, -1);
                        else //ERR
                            addedNode = checkAddBlueNode(idAdded->lexeme, T_ERR, paraListTailIn, -1);
                        if(addedNode == NULL)
                        {
                            fprintf(outputLFile,"SEMERR: in paraList, repeated parameter names\n");
                            addedNode = paraListTailIn;
                        }
                        else
                        {
                            gnStack->thisNode->numParams++;
                            symbolTableEye = addedNode;
                        }
                        free(typeInfo);
                        return paraListTail(addedNode);
                    }
                    else
                    {
                        free(typeInfo);
                        return paraListTail(paraListTailIn);
                    }
        }   }   }
	}
	else if (currToken->tokenType == RPARENS_TYPE)
    {
        return paraListTailIn;
    }
    else
    {
        synerr("; or )","paraListTail",")");
	}
	return paraListTailIn;
};

//cpdstmt			begin			cpdstmt -> begin cpdstmtFact
void cpdstmt()
{
	if (currToken->tokenType == BEGIN_TYPE)
	{
        if(match(BEGIN_TYPE,"cpdstmt","., else, end, or ;")!=NULL)
            cpdstmtFact();
	}
	else
    {
        synerr("begin","cpdstmt","., else, end, or ;");
    }
};

/*cpdstmtFact		id				cpdstmtFact -> stmtList end
cpdstmtFact		begin			cpdstmtFact -> stmtList end
cpdstmtFact		if				cpdstmtFact -> stmtList end
cpdstmtFact		while			cpdstmtFact -> stmtList end
cpdstmtFact		end				cpdstmtFact -> end*/
void cpdstmtFact()
{
	if (currToken->tokenType == ID_TYPE
     || currToken->tokenType == BEGIN_TYPE
     || currToken->tokenType == IF_TYPE
     || currToken->tokenType == WHILE_TYPE)
	{
        stmtList();
        match(END_TYPE,"cpdstmtFact","., else, end, or ;");
	}
    else if (currToken->tokenType == END_TYPE)
        match(END_TYPE,"cpdstmtFact","., else, end, or ;");
    else
        synerr("id, begin, if, while, or end", "cpdstmtFact","., else, end, or ;");
};

/*stmtList			id				stmtList -> stmt stmtListTail
stmtList			begin			stmtList -> stmt stmtListTail
stmtList			if				stmtList -> stmt stmtListTail
stmtList			while			stmtList -> stmt stmtListTail*/
void stmtList()
{
	if (currToken->tokenType == ID_TYPE
     || currToken->tokenType == BEGIN_TYPE
     || currToken->tokenType == IF_TYPE
     || currToken->tokenType == WHILE_TYPE)
     {
        stmt();
        stmtListTail();
     }
     else
        synerr("id, begin, if, or while", "stmtList","end");
};

/*stmtListTail		;				stmtListTail -> ; stmt stmtListTail
stmtListTail		end				stmtListTail -> E*/
void stmtListTail()
{
	if (currToken->tokenType == SEMICOLON_TYPE)
    {
        if(match(SEMICOLON_TYPE, "stmtListTail","end")!=NULL)
        {
            stmt();
            stmtListTail();
        }
    }
    else if (currToken->tokenType == END_TYPE)
        return;
    else
        synerr("; or end", "stmtListTail","end");
};

/*stmt				id				stmt -> variable assignop expr
stmt				begin			stmt -> cpdStmt
stmt				if				stmt -> if expr then stmt stmtTail
stmt				while			while stmt do stmt*/
void stmt()
{
    if (currToken->tokenType == ID_TYPE)
    {
        int varType = variable();
        if(match(ASSIGNOP_TYPE, "stmt", "else, end, or ;")!=NULL)
        {
            int exprType = expr();
            if(varType != T_ERR && exprType != T_ERR && varType != exprType)
            {
                fprintf(outputLFile,"SEMERR: in stmt, assignment operand type mismatch\n");
            }
        }
    }
    else if (currToken->tokenType == BEGIN_TYPE)
    {
        cpdstmt();
    }
    else if (currToken->tokenType == IF_TYPE)
    {
        if(match(IF_TYPE, "stmt", "else, end, or ;")!=NULL)
        {
            int exprType = expr();
            if(match(THEN_TYPE, "stmt", "else, end, or ;")!=NULL)
            {
                stmt();
                stmtTail();
                if(exprType == T_ERR)
                    fprintf(outputLFile,"SEMERR: in stmt, conditional expression type could not be checked\n");
                else if(exprType != T_BOOL)
                    fprintf(outputLFile,"SEMERR: in stmt, conditional expression is not boolean type\n");
            }
        }
    }
    else if (currToken->tokenType == WHILE_TYPE)
    {
        if(match(WHILE_TYPE, "stmt", "else, end, or ;")!=NULL)
        {
            int exprType = expr();
            if(match(DO_TYPE, "stmt", "else, end, or ;")!=NULL)
            {
                stmt();
                if(exprType == T_ERR)
                    fprintf(outputLFile,"SEMERR: in stmt, conditional expression type could not be checked\n");
                else if(exprType != T_BOOL)
                    fprintf(outputLFile,"SEMERR: in stmt, conditional expression is not boolean type\n");
            }
        }
    }
    else
        synerr("id, begin, if, or while","stmt","else, end, or ;");
};

/*stmtTail			else				stmtTail -> else stmt
stmtTail			end				stmtTail -> E
stmtTail			;				stmtTail -> E*/
void stmtTail()
{
	if (currToken->tokenType == ELSE_TYPE)
    {
        if(match(ELSE_TYPE,"stmtTail","else, end, or ;")!=NULL)
            stmt();
    }
    else if (currToken->tokenType == END_TYPE
             || currToken->tokenType == SEMICOLON_TYPE)
    {
        return;
    }
    else
        synerr("else, end, or ;","stmtTail","else, end, or ;");
};

//variable			id				variable -> id varTail
int variable()
{
	if (currToken->tokenType == ID_TYPE)
	{
	    tokenNode* idAdded = match(ID_TYPE, "variable", "assignop");
        if(idAdded!=NULL)
        {
            return varTail(idAdded);
        }
	}
	else
    {
    	synerr("id", "variable", "assignop");
	}
	return T_ERR;
};

/*varTail			[				varTail -> [ expr ]
varTail			assignop			varTail -> E*/
int varTail(tokenNode* varTailIn)
{
	if(currToken->tokenType == LBRACKET_TYPE)
	{
        if(match(LBRACKET_TYPE, "varTail", "assignop")!=NULL)
        {
            int exprType = expr();
            match(RBRACKET_TYPE, "varTail", "assignop");
            //does this need an if-clause based on the match?
            if(exprType == T_INT)
            {
                symbolNode* temp = findGreenNode(varTailIn->lexeme,symbolTableEye);
                if(temp != NULL)
                {
                    int idType = temp->type;
                    if(idType == T_AINT || idType == T_FPAINT)
                        return T_INT;
                    else if(idType == T_AREAL || idType == T_FPAREAL)
                        return T_REAL;
                    else if(idType == T_ERR)
                    {
                        fprintf(outputLFile,"SEMERR: in varTail, array variable type unknown %s\n", varTailIn->lexeme);
                        return T_ERR;
                    }
                    else
                    {
                        fprintf(outputLFile,"SEMERR: in varTail, cannot take array index of non-array variable %s\n", varTailIn->lexeme);
                        return T_ERR;
                    }
                }
                else
                {
                    fprintf(outputLFile,"SEMERR: in varTail, cannot take array index of non-array variable %s\n", varTailIn->lexeme);
                    return T_ERR;
                }
            }
            else
            {
                fprintf(outputLFile,"SEMERR: in varTail, variable %s not recognized\n", varTailIn->lexeme);
                return T_ERR;
            }
        }
	}
	else if (currToken->tokenType == ASSIGNOP_TYPE)
    {
        symbolNode* temp = findGreenNode(varTailIn->lexeme,symbolTableEye);
        if(temp != NULL)
        {
            int idType = temp->type;
            if(idType == T_INT || idType == T_REAL)
                return idType;
            else if (idType == T_FPINT)
                return T_INT;
            else if (idType == T_FPREAL)
                return T_REAL;
            else if(idType == T_FNAME && strcmp(varTailIn->lexeme, gnStack->thisNode->name)==0)
                return gnStack->thisNode->returnType;
            else if(idType == T_ERR)
            {
                fprintf(outputLFile,"SEMERR: in varTail, variable type unknown %s\n", varTailIn->lexeme);
                return T_ERR;
            }
            else
            {
                fprintf(outputLFile,"SEMERR: in varTail, assignment to non-assignable variable %s\n", varTailIn->lexeme);
                return T_ERR;
            }
        }
        else
        {
            fprintf(outputLFile,"SEMERR: in varTail, variable %s was not recognized\n", varTailIn->lexeme);
            return T_ERR;
        }
    }
    else
        synerr("[ or :=", "varTail", "assignop");
    return T_ERR;
};

/*exprList			id				exprList -> expr exprListTail
exprList			num				exprList -> expr exprListTail
exprList			not				exprList -> expr exprListTail
exprList			+				exprList -> expr exprListTail
exprList			-				exprList -> expr exprListTail*/
int exprList(symbolNode* exprListIn)
{
    if(exprListIn->numParams == -1) //syntax error while processing
        return 0;
	else if(currToken->tokenType == ID_TYPE
        || currToken->tokenType == INT_TYPE
        || currToken->tokenType == REAL_TYPE
        || currToken->tokenType == LONGREAL_TYPE
        || currToken->tokenType == LPARENS_TYPE
        || currToken->tokenType == NOT_TYPE
        || (currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_PLUS)
        || (currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_MINUS))
	{
		int exprType = expr();
		pushGreenStack(exprListIn);
        if(gnStack->thisNode->numParams == 0)
        {
            fprintf(outputLFile,"SEMERR: in exprList, function %s should have no parameters\n", exprListIn->name);
            popGreenStack();
            return 0;
        }
        int returnVal = exprListTail(1);
        if(exprType != getParamType(1, gnStack->thisNode))
        {
            returnVal = 0; //false
            fprintf(outputLFile,"SEMERR: in exprList, function %s parameter 1 type mismatch\n", exprListIn->name);
        }
        popGreenStack();
        return returnVal;
    }
    else
        synerr("id, num, nor, +, or -", "exprList", ")");
    return 0; //error
};

/*exprListTail		,				exprListTail -> , expr exprListTail
exprListTail		)				exprListTail -> E*/
int exprListTail(int exprListTailIn)
{
	if(currToken->tokenType == COMMA_TYPE)
	{
		if(match(COMMA_TYPE, "exprListTail",")")!=NULL)
        {
            int exprType = expr();
            if(gnStack->thisNode->numParams == exprListTailIn)
            {
                fprintf(outputLFile,"SEMERR: in exprListTail, function %s should have no more than %d parameters\n", gnStack->thisNode->name, exprListTailIn);
                return 0;
            }
            int returnVal = exprListTail(exprListTailIn+1);
            if(exprType != getParamType(exprListTailIn+1, gnStack->thisNode))
            {
                returnVal = 0;
                fprintf(outputLFile,"SEMERR: in exprListTail, function %s parameter %d type mismatch\n", gnStack->thisNode->name, exprListTailIn+1);
            }
            return returnVal;
        }
	}
	else if(currToken->tokenType == RPARENS_TYPE)
    {
        if(exprListTailIn != gnStack->thisNode->numParams)
        {
            fprintf(outputLFile,"SEMERR: in exprListTail, function %s was not supplied enough parameters; needs %d but received %d\n",
                                                    gnStack->thisNode->name, gnStack->thisNode->numParams, exprListTailIn);
            return 0;
        }
        else
            return 1;
    }
    else
        synerr(", or )","exprListTail",")");
    return 0;
};

/*expr				id				expr -> smpExpr relop smpExpr
expr				num				expr -> smpExpr relop smpExpr
expr				not				expr -> smpExpr relop smpExpr
expr				+				expr -> smpExpr relop smpExpr
expr				-				expr -> smpExpr relop smpExpr*/
int expr()
{
	if(currToken->tokenType == ID_TYPE
        || currToken->tokenType == INT_TYPE
        || currToken->tokenType == REAL_TYPE
        || currToken->tokenType == LONGREAL_TYPE
        || currToken->tokenType == LPARENS_TYPE
        || currToken->tokenType == NOT_TYPE
        || (currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_PLUS)
        || (currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_MINUS))
	{
        int smpExprType = smpExpr();
        return exprTail(smpExprType);
	}
	else
    {
        synerr("id, num, (, not, +, or -", "expr", "then, do, ], ), else, end, or ;");
        return T_ERR;
    }
};

/*exprTail			-				exprTail -> relop smpExpr
exprTail			then				exprTail -> E
exprTail			]				exprTail -> E
exprTail			)				exprTail -> E
exprTail			,				exprTail -> E
exprTail			end				exprTail -> E
exprTail			;				exprTail -> E*/
int exprTail(int exprTailIn)
{
	if(currToken->tokenType == RELOP_TYPE)
    {
        if(match(RELOP_TYPE, "expr", "then, do, ], ), else, end, or ;")!=NULL)
        {
            int smpExprType = smpExpr();
            if(exprTailIn == T_INT || exprTailIn == T_REAL || exprTailIn == T_FPINT || exprTailIn == T_FPREAL)
            {
                if(exprTailIn == smpExprType)
                    return T_BOOL;
                else
                {
                    if(smpExprType==T_ERR)
                        fprintf(outputLFile,"SEMERR: in exprTail, operand types cannot be checked\n");
                    else
                    {
                        if(smpExprType == T_INT || smpExprType == T_REAL || smpExprType == T_FPINT || smpExprType == T_FPREAL)
                            fprintf(outputLFile,"SEMERR: in exprTail, operand types do not match on relop\n");
                        else
                            fprintf(outputLFile,"SEMERR: in exprTail, second operand type not appropriate for relop\n");
                    }
                    return T_ERR;
                }
            }
            else if(exprTailIn == T_ERR)
            {
                fprintf(outputLFile,"SEMERR: in exprTail, first operand type cannot be checked on relop\n");
                return T_ERR;
            }
            else
            {
                fprintf(outputLFile,"SEMERR: in exprTail, first operand type not appropriate for relop\n");
                return T_ERR;
            }
        }
    }
    else if(currToken->tokenType == THEN_TYPE
        || currToken->tokenType == DO_TYPE
        || currToken->tokenType == RBRACKET_TYPE
        || currToken->tokenType == RPARENS_TYPE
        || currToken->tokenType == COMMA_TYPE
        || currToken->tokenType == ELSE_TYPE
        || currToken->tokenType == END_TYPE
        || currToken->tokenType == SEMICOLON_TYPE)
	{
        return exprTailIn;
	}
	else
        synerr("relop, then, ], ), ,, end, or ;", "exprTail", "then, do, ], ), else, end, or ;");
    return T_ERR;
};

/*smpExpr			id				smpExpr -> term smpExprTail
smpExpr			num				smpExpr -> term smpExprTail
smpExpr			(				smpExpr -> term smpExprTail
smpExpr			not				smpExpr -> term smpExprTail
smpExpr			+				smpExpr -> sign term smpExprTail
smpExpr			-				smpExpr -> sign term smpExprTail*/
int smpExpr()
{
	if(currToken->tokenType == ID_TYPE
        || currToken->tokenType == INT_TYPE
        || currToken->tokenType == REAL_TYPE
        || currToken->tokenType == LONGREAL_TYPE
        || currToken->tokenType == LPARENS_TYPE
        || currToken->tokenType == NOT_TYPE)
	{
        int termType = term();
        return smpExprTail(termType);
	}
	else if((currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_PLUS)
        || (currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_MINUS))
    {
        sign();
        int termType = term();
        int smpExprTailIn;
        if(termType == T_INT || termType == T_REAL)
            smpExprTailIn = termType;
        else
        {
            smpExprTailIn = T_ERR;
            if(termType == T_ERR)
                fprintf(outputLFile,"SEMERR: in smpExpr, signed type cannot be checked\n");
            else
                fprintf(outputLFile,"SEMERR: in smpExpr, sign is not appropriate for this term type\n");
        }
        return smpExprTail(smpExprTailIn);
    }
    else
    {
        synerr("id, num, (, not, +, or -", "smpExpr", "relop, then, do, ], ), else, end, or ;");
        return T_ERR;
    }
};

/*smpExprTail		addop			smpExprTail -> addop term smpExprTail
smpExprTail		relop			smpExprTail -> E
smpExprTail		then				smpExprTail -> E
smpExprTail		]				smpExprTail -> E
smpExprTail		)				smpExprTail -> E
smpExprTail		,				smpExprTail -> E
smpExprTail		end				smpExprTail -> E
smpExprTail		;				smpExprTail -> E*/
int smpExprTail(int smpExprTailIn)
{
	if(currToken->tokenType == ADDOP_TYPE)
    {
        tokenNode* addopIn = match(ADDOP_TYPE,"smpExprTail", "relop, then, do, ], ), else, end, or ;");
        if(addopIn!=NULL)
        {
            int smpExprTailTailIn;
            int termType = term();
            if(smpExprTailIn == T_ERR)
            {
                fprintf(outputLFile,"SEMERR: in smpExprTail, first operand type cannot be checked\n");
                smpExprTailTailIn = T_ERR;
            }
            else if((addopIn->tokenType == ADDOP_TYPE && addopIn->attribute == ADDOP_OR && smpExprTailIn != T_BOOL)
                    || (addopIn->tokenType == ADDOP_TYPE && addopIn->attribute != ADDOP_OR && (smpExprTailIn != T_INT && smpExprTailIn != T_REAL)))
            {
                fprintf(outputLFile,"SEMERR: in smpExprTail, first operand inappropriate for addop\n");
                smpExprTailTailIn = T_ERR;
            }
            else
            {
                if(smpExprTailIn == termType)
                    smpExprTailTailIn = smpExprTailIn;
                else
                {
                    smpExprTailTailIn = T_ERR;
                    if(termType == T_INT || termType == T_REAL || termType == T_BOOL)
                        fprintf(outputLFile,"SEMERR: in smpExprTail; operand types do not match on addop\n");
                    else if(termType == T_ERR)
                        fprintf(outputLFile,"SEMERR: in smpExprTail; second operand type cannot be checked on addop\n");
                    else
                        fprintf(outputLFile,"SEMERR: in smpExprTail; second operand type is inappropriate for this addop\n");
                }
            }
            return smpExprTail(smpExprTailTailIn);
        }
    }
    else if(currToken->tokenType == RELOP_TYPE
        || currToken->tokenType == THEN_TYPE
        || currToken->tokenType == DO_TYPE
        || currToken->tokenType == RBRACKET_TYPE
        || currToken->tokenType == RPARENS_TYPE
        || currToken->tokenType == COMMA_TYPE
        || currToken->tokenType == ELSE_TYPE
        || currToken->tokenType == END_TYPE
        || currToken->tokenType == SEMICOLON_TYPE)
	{
		return smpExprTailIn;
	}
    else
        synerr("addop, relop, then, ], ), ,, end, or ;", "smpExprTail", "relop, then, do, ], ), else, end, or ;");
    return T_ERR;
};

/*term				id				term -> factor termTail
term				num				term -> factor termTail
term				(				term -> factor termTail
term				not				term -> factor termTail*/
int term()
{
	if(currToken->tokenType == ID_TYPE
        || currToken->tokenType == INT_TYPE
        || currToken->tokenType == REAL_TYPE
        || currToken->tokenType == LONGREAL_TYPE
        || currToken->tokenType == LPARENS_TYPE
        || currToken->tokenType == NOT_TYPE)
	{
        int factorType = factor();
        return termTail(factorType);
	}
	else
    {
        synerr("id, num, (, not", "term", "addop, relop, then, do, ], ), else, end, or ;");
        return T_ERR;
    }
};
/*
termTail			mulop			termTail -> mulop factor termTail
termTail			addop			termTail -> E
termTail			relop			termTail -> E
termTail			then				termTail -> E
termTail			]				termTail -> E
termTail			)				termTail -> E
termTail			,				termTail -> E
termTail			end				termTail -> E
termTail			;				termTail -> E*/
int termTail(int termTailIn)
{
	if(currToken->tokenType == MULOP_TYPE)
    {
        tokenNode* mulopToken = match(MULOP_TYPE, "termTail", "addop, relop, then, do, ], ), else, end, or ;");
        if(mulopToken!=NULL)
		{
		    int factorType = factor();
		    int termTailTailIn;
            if(((mulopToken->attribute == MULOP_MULT || mulopToken->attribute == MULOP_DIVWORD || mulopToken->attribute == MULOP_DIV)&& (termTailIn == T_INT || termTailIn == T_REAL))
            ||(mulopToken->attribute == MULOP_MOD && termTailIn == T_INT)
            ||(mulopToken->attribute == MULOP_AND && termTailIn == T_BOOL))
            {
                if(termTailIn == factorType)
                    termTailTailIn = factorType;
                else
                {
                    termTailTailIn = T_ERR;
                    if(factorType == T_INT||factorType == T_REAL||factorType == T_BOOL)
                        fprintf(outputLFile,"SEMERR: in termTail, operand types do not match\n");
                    else if(factorType == T_ERR)
                        fprintf(outputLFile,"SEMERR: in termTail; second operand type cannot be checked on mulop\n");
                    else
                        fprintf(outputLFile,"SEMERR: in termTail; second operand type inappropriate for mulop\n");
                }
            }
            else
            {
                termTailTailIn = T_ERR;
                if(termTailIn == T_ERR)
                    fprintf(outputLFile,"SEMERR: in termTail, cannot check first operand type for mulop\n");
                else
                    fprintf(outputLFile,"SEMERR: in termTail, operands mismatched with mulop\n");
            }
            return termTail(termTailTailIn);
		}
    }
    else if(currToken->tokenType == ADDOP_TYPE
        || currToken->tokenType == RELOP_TYPE
        || currToken->tokenType == THEN_TYPE
        || currToken->tokenType == DO_TYPE
        || currToken->tokenType == RBRACKET_TYPE
        || currToken->tokenType == RPARENS_TYPE
        || currToken->tokenType == COMMA_TYPE
        || currToken->tokenType == ELSE_TYPE
        || currToken->tokenType == END_TYPE
        || currToken->tokenType == SEMICOLON_TYPE)
	{
		return termTailIn;
	}
    else
        synerr("mulop, addop, relop, then, ], ), ,, end, or ;","termTail", "addop, relop, then, do, ], ), else, end, or ;");
    return T_ERR;
};

/*
factor			id				factor -> id factorFact
factor			num				factor -> num
factor			(			    factor -> ( expr )
factor			not				factor -> not factor
*/
int factor()
{
	if(currToken->tokenType == ID_TYPE)
    {
        tokenNode* addedId = match(ID_TYPE,"factor", "mulop, addop, relop, then, do, ], ), else, end, or ;");
        if(addedId!=NULL)
        {
            symbolNode* factorFactIn = findGreenNode(addedId->lexeme,symbolTableEye);
            if(factorFactIn != NULL)
                return factorFact(factorFactIn);
            else
            {
                fprintf(outputLFile,"SEMERR: in factor; %s undeclared in this scope\n", addedId->lexeme);
                return T_ERR;
            }
        }
        else
            return T_ERR;
    }
    else if(currToken->tokenType == INT_TYPE || currToken->tokenType == REAL_TYPE || currToken->tokenType == LONGREAL_TYPE)
    {
        tokenNode* numToken = match(NUM_TYPE,"factor", "mulop, addop, relop, then, do, ], ), else, end, or ;");
        if(numToken->tokenType == INT_TYPE)
            return T_INT;
        else //real or longreal
            return T_REAL;
    }
    else if(currToken->tokenType == LPARENS_TYPE)
    {
        if(match(LPARENS_TYPE,"factor", "mulop, addop, relop, then, do, ], ), else, end, or ;")!=NULL)
        {
            int exprType = expr();
            match(RPARENS_TYPE,"factor", "mulop, addop, relop, then, do, ], ), else, end, or ;");
            return exprType;
        }
        else
            return T_ERR;
    }
    else if(currToken->tokenType == NOT_TYPE)
    {
        if(match(NOT_TYPE,"factor", "mulop, addop, relop, then, do, ], ), else, end, or ;")!=NULL)
        {
            int factorType = factor();
            if(factorType == T_BOOL)
                return T_BOOL;
            else if(factorType == T_ERR)
            {
                fprintf(outputLFile,"SEMERR: in factor; operand type cannot be checked\n");
                return T_ERR;
            }
            else
            {
                fprintf(outputLFile,"SEMERR: in factor; not used with non-boolean factor\n");
                return T_ERR;
            }
        }
        else
            return T_ERR;
    }
    else
    {
		synerr("id, num, or not","factor", "mulop, addop, relop, then, do, ], ), else, end, or ;");
        return T_ERR;
    }
};

/*
factorFact			(				factorFact -> ( exprList )
factorFact			[				factorFact -> [ expr ]
factorFact			mulop			factorFact -> E
factorFact			addop			factorFact -> E
factorFact			relop			factorFact -> E
factorFact			then			factorFact -> E
factorFact			]				factorFact -> E
factorFact			)				factorFact -> E
factorFact			,				factorFact -> E
factorFact			end				factorFact -> E
factorFact			;				factorFact -> E
*/
int factorFact(symbolNode* factorFactIn)
{
	if(currToken->tokenType == LPARENS_TYPE)
    {
        if(match(LPARENS_TYPE,"factorFact", "mulop, addop, relop, then, do, ], ), else, end, or ;")!=NULL)
        {
            int exprListS = exprList(factorFactIn);
            match(RPARENS_TYPE, "factorFact", "mulop, addop, relop, then, do, ], ), else, end, or ;");
            if(factorFactIn->type == T_FNAME && factorFactIn->numParams > 0)
            {
                if(exprListS == 1) //do the parameters match
                    return factorFactIn->returnType;
                else
                    return T_ERR;
            }
            else if (factorFactIn->type == T_ERR)
            {
                fprintf(outputLFile,"SEMERR: in factorFact; %s's type could not be checked to be a function\n", factorFactIn->name);
                return T_ERR;
            }
            else
            {
                fprintf(outputLFile,"SEMERR: in factorFact; %s is not a function with parameters\n", factorFactIn->name);
                return T_ERR;
            }
        }
        return T_ERR;
    }
    else if(currToken->tokenType == LBRACKET_TYPE)
    {
        if(match(LBRACKET_TYPE,"factorFact", "mulop, addop, relop, then, do, ], ), else, end, or ;")!=NULL)
        {
            int exprType = expr();
            match(RBRACKET_TYPE,"factorFact", "mulop, addop, relop, then, do, ], ), else, end, or ;");
            if(exprType == T_INT)
            {
                if(factorFactIn->type == T_AINT || factorFactIn->type == T_FPAINT)
                    return T_INT;
                else if (factorFactIn->type == T_AREAL || factorFactIn->type == T_FPAREAL)
                    return T_REAL;
                else
                {
                    fprintf(outputLFile,"SEMERR: in factorFact; array index applied to %s, not of array type\n", factorFactIn->name);
                    return T_ERR;
                }
            }
            else
            {
                fprintf(outputLFile,"SEMERR: in factorFact; array index must be an integer\n");
                return T_ERR;
            }
        }
    }
    else if(currToken->tokenType == MULOP_TYPE
        || currToken->tokenType == ADDOP_TYPE
        || currToken->tokenType == RELOP_TYPE
        || currToken->tokenType == THEN_TYPE
        || currToken->tokenType == DO_TYPE
        || currToken->tokenType == RBRACKET_TYPE
        || currToken->tokenType == RPARENS_TYPE
        || currToken->tokenType == COMMA_TYPE
        || currToken->tokenType == ELSE_TYPE
        || currToken->tokenType == END_TYPE
        || currToken->tokenType == SEMICOLON_TYPE)
	{
	    if(factorFactIn->type == T_FPINT)
            return T_INT;
        else if(factorFactIn->type == T_FPREAL)
            return T_REAL;
        else if(factorFactIn->type == T_FPAINT)
            return T_AINT;
        else if(factorFactIn->type == T_FPAREAL)
            return T_AREAL;
        else if(factorFactIn->type == T_FNAME)
        {
            if(factorFactIn->numParams > 0)
            {
                fprintf(outputLFile,"SEMERR: in factorFact; %s is not called with the required parameters\n", factorFactIn->name);
                return T_ERR;
            }
            else if (factorFactIn->numParams == -1 || factorFactIn->returnType == -1)
            {
                fprintf(outputLFile,"SEMERR: in factorFact; cannot check parameter list\n");
                return T_ERR;
            }
            else
                return factorFactIn->returnType;
        }
        else
            return factorFactIn->type;
	}
	else
        synerr("(, [, mulop, addop, relop, then, ], ), ,, end, or ;","factorFact", "mulop, addop, relop, then, do, ], ), else, end, or ;");
	return T_ERR;
};

//sign				+				sign -> +
//sign				-				sign -> -
void sign()
{
	if(currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_PLUS)
    {
        match(ADDOP_TYPE,"sign","id, num, (, or not");
    }
    else if(currToken->tokenType == ADDOP_TYPE && currToken->attribute == ADDOP_MINUS)
	{
		match(ADDOP_TYPE,"sign","id, num, (, or not");
	}
	else
    {
		synerr("+ or -","sign", "id, num, (, or not");
	}
};

char* typeToString(int typeIn)
{
    switch(typeIn)
    {
        case LEXERR_TYPE:
            return "lexerr";
        case ADDOP_TYPE:
            return "addop";
        case MULOP_TYPE:
            return "mulop";
        case RELOP_TYPE:
            return "relop";
        case ASSIGNOP_TYPE:
            return "assignop";
        case WS_TYPE:
            return "whitespace";
        case ID_TYPE:
            return "id";
        case COLON_TYPE:
            return ":";
        case SEMICOLON_TYPE:
            return ";";
        case DOT_TYPE:
            return ".";
        case DOTDOT_TYPE:
            return "..";
        case LPARENS_TYPE:
            return "(";
        case RPARENS_TYPE:
            return ")";
        case LBRACKET_TYPE:
            return "[";
        case RBRACKET_TYPE:
            return "]";
        case COMMA_TYPE:
            return ",";
        case INT_TYPE:
            return "int";
        case REAL_TYPE:
            return "real";
        case LONGREAL_TYPE:
            return "longreal";
        case EOF_TYPE:
            return "$";
        case NUM_TYPE:
            return "num";
        case ARRAY_TYPE:
            return "array";
        case BEGIN_TYPE:
            return "begin";
        case DO_TYPE:
            return "do";
        case ELSE_TYPE:
            return "else";
        case END_TYPE:
            return "end";
        case FUNCTION_TYPE:
            return "function";
        case IF_TYPE:
            return "if";
        case INTWORD_TYPE:
            return "integer";
        case NOT_TYPE:
            return "not";
        case OF_TYPE:
            return "of";
        case PROCEDURE_TYPE:
            return "procedure";
        case PROGRAM_TYPE:
            return "program";
        case REALWORD_TYPE:
            return "real";
        case THEN_TYPE:
            return "then";
        case VAR_TYPE:
            return "var";
        case WHILE_TYPE:
            return "while";
        default:
            return "unknown";
    }
}

symbolNode* findBlueNode(char* id, symbolNode* currNode)
{
    while(currNode != NULL)
    {
        if(strcmp(id,currNode->name)==0)
            return currNode;
        else
        {
            if(currNode->returnType == -1)
                currNode = currNode->prevNode;
            else //we have reached a green node
                return NULL;
        }
    }
    return currNode;
};
symbolNode* findGreenNode(char* id, symbolNode* currNode)
{
    while(currNode != NULL && strcmp(id,currNode->name)!=0)
    {
        currNode = currNode->prevNode;
    }
    return currNode;
};
symbolNode* checkAddBlueNode(char* nameIn, int typeIn, symbolNode* pNode, int memLoc)
{
    symbolNode* blueNode = findBlueNode(nameIn, pNode);
    if(blueNode == NULL)
    {
        symbolNode* newNode = malloc(sizeof(symbolNode));

        int len = 0;
        while(nameIn[len] != '\0')
            len++;
        len++; //count the null character as well
        newNode->name = malloc(len*sizeof(char));
        int i;
        for(i = 0; i < len; i++)
            newNode->name[i] = nameIn[i];

        newNode->type = typeIn;
        newNode->prevNode = pNode;
        pNode->nextLowerNode = newNode;
        newNode->nextSameNode = NULL;
        newNode->nextLowerNode = NULL;
        newNode->memLocation = memLoc;

        newNode->returnType = -1;
        newNode->numParams = -1;

        //print new thing to file
        for(i = 0; i < numTabs; i++)
            fprintf(symMemTableFile, "\t");
        fprintf(symMemTableFile, "%s\t%X\n", nameIn, memLoc);

        return newNode;
    }
    else
        return NULL;
};
//find/add green (function) node to scope graph; check all the way to the top; no repeats.
symbolNode* checkAddGreenNode(char* nameIn, int typeIn, symbolNode* pNode, int attachMode)
{
    symbolNode* greenNode = findGreenNode(nameIn, pNode);
    if(greenNode == NULL) //if not present
    {
        symbolNode* newNode = malloc(sizeof(symbolNode));

        int len = 0;
        while(nameIn[len] != '\0')
            len++;
        len++; //count the null character as well
        newNode->name = malloc(len*sizeof(char));
        int i;
        for(i = 0; i < len; i++)
            newNode->name[i] = nameIn[i];

        newNode->type = typeIn;
        newNode->prevNode = pNode;
        if(attachMode == 0)
            pNode->nextLowerNode = newNode;
        else if (attachMode == 1)
            pNode->nextSameNode = newNode;
        newNode->nextSameNode = NULL;
        newNode->nextLowerNode = NULL;

        newNode->returnType = T_ERR;
        newNode->numParams = -1;

        //print new thing to file
        for(i = 0; i < numTabs; i++)
            fprintf(symMemTableFile, "\t");
        fprintf(symMemTableFile, "%s\t%X\n", nameIn, -1);

        return newNode;
    }
    else
    {
        return NULL;
    }
};
//at the end of a program unit (function), remove the top node from the green node stack
symbolNode* popGreenStack()
{
    if(gnStack != NULL)
    {
        memAddress += gnStack->prevOffset;
        gsNode* topNode = gnStack;
        gnStack = gnStack->prevNode;
        symbolNode* temp = topNode->thisNode;
        free(topNode);
        return temp;
    }
    else
        return NULL;
};
void pushGreenStack(symbolNode* pushMe)
{
    gsNode* newNode = malloc(sizeof(gsNode));
    newNode->name = pushMe->name;
    newNode->prevOffset = memAddress;
    memAddress = 0;
    newNode->thisNode = pushMe;
    newNode->prevNode = gnStack;
    gnStack = newNode;
};
symbolNode* traceTree(symbolNode* root)
{
    printf("Tracing tree below %s.\n",root->name);
    return traceTreeBelow(root);
};
symbolNode* traceTreeBelow(symbolNode* sTN)
{
    printf("%s is a %d; ",sTN->name, sTN->type);
    symbolNode* temp;
    if(sTN->nextSameNode != NULL)
    {
        printf("checking next node in same scope.\n");
        temp = traceTreeBelow(sTN->nextSameNode);
        if(temp != NULL) return temp;
    }
    if(sTN->nextLowerNode != NULL)
    {
        printf("checking first node in sub-scope.\n");
        temp = traceTreeBelow(sTN->nextLowerNode);
        if(temp != NULL) return temp;
    }
    printf("going back up.\n");
    return NULL;
};
int getType(char* name)
{
    symbolNode* temp = findGreenNode(name,symbolTableEye);
    if(temp != NULL)
        return temp->type;
    else
        return -1;
};
int getNumParams(symbolNode* a)
{
    if(a->type == T_FNAME)
        return a->numParams;
    else
        return -1;
};
int getParamType(int i, symbolNode* a)
{
    if(a->type != T_FNAME) return -1;
    while(i > 0)
    {
        if(a==NULL) return -1;
        a = a->nextLowerNode;
        i--;
    }
    if(a != NULL)
    {
        if(a->type == T_FPINT)
            return T_INT;
        else if(a->type == T_FPREAL)
            return T_REAL;
        else if(a->type == T_FPAINT)
            return T_AINT;
        else if(a->type == T_FPAREAL)
            return T_AREAL;
    }
    return -1;
};

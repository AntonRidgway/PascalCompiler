% Anton Ridgway, Fall 2013, Pascal Compiler %

This project is the front-end of a Pascal compiler, designed for the simplified version of the language used in the Compilers course at the University of Tulsa (CS4013). It consists of two main components, a lexical analyzer (lexical.c) that breaks up a source file into tokens, and a parser (parser.c) that leverages a decorated parse tree to understand the tokens and assign identifiers memory addresses. These are designed to be compiled as separate executables, and run in sequence. Together, they are able to identify any lexical, syntax, or semantic errors present in the source code. Example source code is included in the /src folder, and a build of the executables with the output for example.pas is included in the /build folder.

Note that reserved.txt is required for the lexical analyzer to run correctly.

The root of the repository includes a makefile for easy compilation, as well as an example source file to compile (example.pas), and the list of reserved words (the hard-coded input, reserved.txt). Once built, the compiler can be run using compile.bat, which simply executes lexical.exe and parser.exe in sequence. In Windows, this is done on the command line with either:

compile example.pas

or

lexical example.pas
parser

Once the compiler is run, the console will contain a trace of the compiler's progress through the code. Output files are:

Lexical Analyzer:
listing.txt - The primary output. Numbers each line in the source and adds notes for any lexical errors.
symbolTable.txt - A listing of the symbols identified in the source and the memory addresses where each is stored.
tokenFile.txt - A listing of the tokens detected in the source, with 1) the line number, 2) the lexeme, 3) the number of the token type, and 4) the attribute (here, assigned memory addresses, for identifiers only).
tokenListing.txt - A formatted version of tokenFile.txt.

Parser:
listing2.txt - The primary output. Adds notes of any syntactical or semantic errors in the source to listing.txt.
symMemTable.txt - A listing of the symbols identified in the source, and the memory addresses assigned to each (according to the size of the identifier's type in bytes).

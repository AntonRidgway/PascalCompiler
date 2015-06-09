CC = gcc

.PHONY: all lexical parser

all: lexical parser

lexical: lexical.o
	$(CC) -o lexical lexical.o

parser: parser.o
	$(CC) -o parser parser.o
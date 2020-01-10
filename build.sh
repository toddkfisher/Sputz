#!/bin/sh
echo "Generating: TAGS file"
etags *.c *.h
echo "Generating: prototypes.h"
cproto util.c stackalloc.c strtab.c gen-read.c lexical-unit.c parse-tree.c > prototypes.h
echo "Compiling: gen-read.c"
gcc -DDEBUG -c gen-read.c
echo "Compiling: util.c"
gcc -DDEBUG -c util.c
echo "Compiling: stackalloc.c"
gcc -DDEBUG -c stackalloc.c
echo "Compiling: strtab.c"
gcc -DDEBUG -c strtab.c
echo "Compiling: lexical-unit.c"
gcc -DDEBUG -DNO_CPROTO -c lexical-unit.c
echo "Compiling: parse-tree.c"
gcc -DTEST_PARSE -DDEBUG -DNO_CPROTO -c parse-tree.c
echo "Linking: spz"
gcc -ggdb -o spz  util.o stackalloc.o strtab.o gen-read.o lexical-unit.o parse-tree.o -lm
echo "Build complete"

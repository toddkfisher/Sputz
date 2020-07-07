#!/bin/sh
CMD_LINE_DEFINES="-DDEBUG -DTEST_PARSE"
echo "Common #defines: " ${CMD_LINE_DEFINES}
echo "Generating: prototypes.h"
cproto util.c status-codes.c stackalloc.c strtab.c gen-read.c lexical-unit.c parse-tree.c > prototypes.h
echo "Compiling: gen-read.c"
gcc -ggdb $CMD_LINE_DEFINES -c gen-read.c
echo "Compiling: util.c"
gcc -ggdb $CMD_LINE_DEFINES -c util.c
echo "Compiling: stackalloc.c"
gcc -ggdb $CMD_LINE_DEFINES -c stackalloc.c
echo "Compiling: strtab.c"
gcc -ggdb $CMD_LINE_DEFINES -c strtab.c
echo "Compiling: lexical-unit.c"
gcc -ggdb $CMD_LINE_DEFINES -DNO_CPROTO -c lexical-unit.c
echo "Compiling: parse-tree.c"
gcc -ggdb $CMD_LINE_DEFINES -DNO_CPROTO -c parse-tree.c
echo "Compiling: status-codes.c"
gcc -ggdb $CMD_LINE_DEFINES -DNO_CPROTO -c status-codes.c
echo "Linking: spz"
gcc -ggdb -o spz  status-codes.o util.o stackalloc.o strtab.o gen-read.o lexical-unit.o parse-tree.o -lm
echo "Generating: TAGS file"
etags *.c *.h
echo "Build complete"

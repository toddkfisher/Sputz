#!/bin/sh
gcc `pkg-config --cflags guile-2.2` -c test.c
gcc -o gt test.o `pkg-config --libs guile-2.2`

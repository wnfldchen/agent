# Copyright 2019, Winfield Chen and Lloyd T. Elliott.

SRCDIRS := ../src ../lib/src
SOURCES := $(wildcard $(foreach i,$(SRCDIRS),$i/*.c)) 
SOURCES := $(filter-out ../src/main.c,$(SOURCES))
OBJECTS := $(SOURCES:%.c=%.o)

test: test.c
	gcc-9 -c util.c -std=c17 -I../include -I../lib/include -I/usr/local/include 
	gcc-9 test.c -o test util.o $(OBJECTS) -std=c17 -I../include -I../lib/include -I/usr/local/include -L/usr/local/lib -lcmocka -lm -lgsl

clean:
	rm test
	rm util.o

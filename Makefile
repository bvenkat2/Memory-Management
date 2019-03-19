CC=gcc
CFLAGS=-I. -c -g -Wall
LINKARGS=-lm
LIBS=-lm

SOURCE_FILES= asgn3.c \
				asgn3_support.c
OBJECT_FILES=asgn3.o \
				asgn3_support.o

all : asgn3

asgn3 : $(OBJECT_FILES)
	$(CC) $(LINKARGS) $(OBJECT_FILES) -o $@ $(LIBS)

clean : 
	rm -f asgn3 $(OBJECT_FILES)

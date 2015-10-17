# http://mrbook.org/blog/tutorials/make/

# all:
# 	gcc test_buffered.c my_stdio.c -o my_stdio_test

CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=test_buffered.c my_stdio.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=my_stdio_test

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
# http://mrbook.org/blog/tutorials/make/

CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
# SOURCES=test_buffered.c test_format.c my_stdio.c
# OBJECTS=$(SOURCES:.c=.o)
# EXECUTABLE=my_stdio_test

# all: $(SOURCES) $(EXECUTABLE)
    
# $(EXECUTABLE): $(OBJECTS) 
	# $(CC) $(LDFLAGS) $(OBJECTS) -o $@

test_rw: test_buffered.o my_stdio.o
	$(CC) $(LDFLAGS) test_buffered.c my_stdio.c -o $@

test_ps: test_format.o my_stdio.o
	$(CC) $(LDFLAGS) test_format.c my_stdio.c -o $@
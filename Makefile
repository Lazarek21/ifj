C=gcc
CFLAGS=-std=c99 -Wall -Werror -pedantic 
SOURCES=*.c


ifj20:$(SOURCES)
	$(C) $(CFLAGS) $(SOURCES) -o ifj20



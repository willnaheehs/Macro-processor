CC = gcc
CFLAGS = -std=c17 -Wall -Werror -pedantic -g

Macro: proj1.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) Macro
	$(RM) *.o

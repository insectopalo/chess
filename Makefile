CC=gcc
CFLAGS=-I.
LIBS=-lm

cmatrix: cmatrix.o
	$(CC) -o cmatrix cmatrix.o $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm *.o

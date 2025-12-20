CC = gcc
CFLAGS = -Wall -Wextra -O2
OBJ = main.o fonction.o
EXEC = projet

OUTDIR = output
TMPDIR = tmp

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

main.o: main.c fichier.h
	$(CC) $(CFLAGS) -c main.c

fonction.o: fonction.c fichier.h
	$(CC) $(CFLAGS) -c fonction.c

clean:
	rm -f $(OBJ) $(EXEC)
	rm -rf $(OUTDIR) $(TMPDIR)

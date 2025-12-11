
PROG = projet

# Sources
SRC = main.c fonction.c

OBJ = $(SRC:.c=.o)

#Compilateur
CC = gcc

#Compilation
CFLAGS = -Wall -g

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) -o $(PROG) $(OBJ) -lm


%.o: %.c fichier.h
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f *.o $(PROG)

# Règle lancement programme
run: $(PROG)
	./$(PROG)


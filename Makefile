PROG = projet

#sources
SRC = main.c fonction.c

#règle par défaut
all: $(PROG)
	./$(PROG)

#compilation du programme
$(PROG): $(SRC) fichier.h
	gcc -Wall -o $(PROG) $(SRC)

#Nettoyage
clean:
	rm -f $(PROG)

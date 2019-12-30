
INC = include


SOURCES = main.c

OBJ = bin/audioinfo

all:$(SOURCES)
	gcc -g $(SOURCES) -I$(INC) -o $(OBJ)

debug:
	gcc -g $(SOURCES) -I$(INC) -o $(OBJ) -DEBUG


clean:
	rm -rf $(OBJ)
	rm -rf obj/*


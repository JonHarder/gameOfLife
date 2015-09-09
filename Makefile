all: game

game: main.c
	gcc main.c -o game -lncurses

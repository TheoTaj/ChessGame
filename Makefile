
OBJS = main.o chess.o display.o IA.o Move.o fen.o Zobrist.o GMmove.o

TARGET = chess
CC = gcc -pg
CFLAGS = -Wall -Wextra -Wmissing-prototypes --pedantic -std=c99 -g -O0 -pg $(shell pkg-config --cflags glib-2.0)
LDFLAGS = $(shell sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer $(shell pkg-config --libs glib-2.0)

# Définir la position FEN initiale

FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" # Position basique

# Pos 2
# FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"


#pos 3

# FEN ="8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"

#pos 4

# FEN ="r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"

#pos 5

# FEN ="rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ" 

#pos 6
# FEN ="r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w"



#test

# 2 rooks vs 1 king
# FEN = "3kr3/4r3/8/8/8/8/2K5/8 w - - 0 1" 

# 1 queen vs 1 king

# FEN = "3kq3/8/8/8/8/8/2K5/8 w - - 0 1"

# 1 rook vs 1 king

# FEN = "3kr3/8/8/8/8/8/2K5/8 w - - 0 1"

# knioght and bishop vs 1 king

# FEN = "1nkb4/8/8/8/8/8/8/7K w - - 0 1"

# hard checkmate

FEN = "8/3K4/4P3/8/8/8/6k1/7q w - - 0 1" #il trouve mais c'est pas le plus rapide

# temp

# FEN = "8/4PPPP/8/k7/8/K7/4pppp/8 w - - 0 1"

# FEN = "k7/2p4p/8/8/8/8/2p4p/K7 b - - 0 1"


# RUN_ARGS = --noai
# RUN_ARGS = --aiW
RUN_ARGS = --aiB

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: all clean run memcheck perfcheck

clean:
	rm -f $(OBJS) $(TARGET) gmon.out

run: $(TARGET)
	./$(TARGET) $(RUN_ARGS) $(FEN)

memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --suppressions=supp.txt --gen-suppressions=all ./$(TARGET) $(RUN_ARGS) $(FEN) 2> valgrind_report.txt

perfcheck: $(TARGET)
	./$(TARGET) $(RUN_ARGS) $(FEN)
	gprof $(TARGET) gmon.out > gprof_report.txt

# Compilation des fichiers objets avec profilage
main.o: main.c chess.h display.h Move.h IA.h fen.h Zobrist.h GMmove.h
	$(CC) $(CFLAGS) -pg -c main.c

chess.o: chess.c chess.h Move.h
	$(CC) $(CFLAGS) -pg -c chess.c

display.o: display.c display.h chess.h
	$(CC) $(CFLAGS) -pg -c display.c

IA.o: IA.c IA.h chess.h Move.h Zobrist.h
	$(CC) $(CFLAGS) -pg -c IA.c

Move.o: Move.c Move.h chess.h
	$(CC) $(CFLAGS) -pg -c Move.c

fen.o: fen.c fen.h chess.h
	$(CC) $(CFLAGS) -pg -c fen.c

Zobrist.o: Zobrist.c Zobrist.h
	$(CC) $(CFLAGS) -pg -c Zobrist.c

GMmove.o: GMmove.c GMmove.h Zobrist.h chess.h
	$(CC) $(CFLAGS) -pg -c GMmove.c
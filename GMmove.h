#ifndef GMMOVE_H
#define GMMOVE_H

#include <stdio.h>
#include <stdlib.h>

#include "chess.h"
#include "fen.h"

typedef struct GMmove_t
{
    unsigned long long key; 
    int nbMove;             
    Move **moveTab;         
} GMmove;

GMmove *createGMmove(unsigned long long key, int nbMove, char *moves, int *frequencies);

void freeGMmove(GMmove *move);

Move *convertCharInMove(char *move, Player player);

void replaceFenWithZobristKey(const char *inputFile, const char *outputFile);

unsigned long long createZobristKeyFromFen(char *fen);

Move *getRandGMmove(GMmove* gmMove);
void initGmTable(void);
void initGmMove(const char *filename);
void freeGmTable(void);
unsigned long long getKeyFromGMmove(GMmove *move);
int getNbMoveFromGMmove(GMmove *move);
void insertGmTable(GMmove *move);







#endif
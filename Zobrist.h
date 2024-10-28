
#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "chess.h"
#include "IA.h"


#define NUM_PIECES 12   // Nombre total de pièces différentes
#define NUM_ROWS 8      // Nombre de lignes sur un échiquier
#define NUM_COLS 8      // Nombre de colonnes sur un échiquier


typedef struct ZobristMove_t
{
    unsigned long long key;
    double eval;
    Move *bestMove;
    int depth;
    
} ZobristMove;



extern unsigned long long piecesArray[8][8][12];
extern unsigned long long castlingRights[4];
extern unsigned long long enPassantCol[8];
extern unsigned long long sideToMove[2];

void ZobristInit(void);

void printZobristValues(void);

unsigned long long RandomUnsigned64BitNumber(void);

unsigned long long getZobristKey(ChessGame *game);


ZobristMove *createZobristMove(unsigned long long key, double eval, int depth, Move *bestMove);
unsigned long long getUpdZobristKey(ChessGame *game, Move *move);
void initTranspositionTable(void);
void insertTranspositionTable(ZobristMove *move);
unsigned long long getKeyFromZobristMove(ZobristMove *move);
double getEvalFromZobristMove(ZobristMove *move);
int getDepthFromZobristMove(ZobristMove *move);
Move *getBestMoveFromZobristMove(ZobristMove *move);
void freeTranspositionTable(void);

void FreeZobristMove(ZobristMove *move);
GList *copyGlistOfZobrist(GList *zobristList);
void freeGlistOfZobrist(GList *zobristList) ;
void printZobristList(ChessGame *game);




#endif // ZOBRIST_H

#ifndef MOVE_H
#define MOVE_H



#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include <time.h>
#include <glib.h>


typedef enum MoveType_t
{
    Classique = 0,
    Promotion = 1000,
    Roque = 10,
    Prise = 500,
    PriseEnPassant = 100,
} MoveType;

#include "chess.h"

typedef struct Move_t
{
    Player player;
    int start_row;
    int start_col;
    int end_row;
    int end_col;
    int response;
    double eval;
    MoveType moveType;
    bool prise;
    int frequency;
} Move;



Move *CreateMove( Player player, int start_row, int start_col, int end_row, int end_col); //ok
void FreeMove(Move *move);//ok
int getStartColFromMove(Move *move);
int getStartRowFromMove(Move *move);
int getEndColFromMove(Move *move);
int getEndRowFromMove(Move *move);
Player getPlayerFromMove(Move *move);
Move* copy_move(const Move* original);
void newStartRow(Move *move, int newStartRow);
void newStartCol(Move *move, int newStartCol);
void newEndRow(Move *move, int newEndRow);
void newEndCol(Move *move, int newEndCol);
void newPlayer(Move *move, Player newPlayer);
int getResponseFromMove(Move *move);
int getFrequencyFromMove(Move *move);


void newResponse(Move *move, int response);
void PrintMove(Move *move);
void PrintAllMoves(GList *moveList);
double getEvalFromMove(Move *move);
void newEval(Move *move, double eval);
MoveType getMoveTypeFromMove(Move *move);
void newMoveType(Move *move, MoveType newMoveType);
int compareMoves(const void *a, const void *b);
void FindMoveType(ChessGame *game, Move *move);
void newEndPiece(Move *move, Piece endPiece);
Piece getEndPieceFromMove(Move *move);
Move *convertMouseClicksToMove(int *tab, ChessGame *game, Player user);
Piece getPromotedPiece(Player player, int response);
void newFrequency(Move *move, int frequency);

#endif // MOVE_H
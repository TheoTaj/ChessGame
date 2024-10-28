#ifndef CHESS_H
#define CHESS_H

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include<glib.h>




extern int nbPositions;
extern int depth;
extern int nbPromotion;
extern int nbRoque;
extern int nbPriseEnPassant;
extern int nbClassique;
extern int nbPrise;
extern int totalLength;
extern int nbZobrist;
extern int nbZobrist2;




typedef struct ChessGame_t ChessGame;
typedef enum Piece_t
{
    EMPTY_CASE = 0,

    WHITE_KING = 1,
    WHITE_QUEEN = 2,
    WHITE_BISHOP = 3,
    WHITE_KNIGHT = 4,
    WHITE_ROOK = 5,
    WHITE_PAWN = 6,

    BLACK_KING = -1,
    BLACK_QUEEN = -2,
    BLACK_BISHOP = -3,
    BLACK_KNIGHT = -4,
    BLACK_ROOK = -5,
    BLACK_PAWN = -6,
} Piece;

typedef enum Player_t
{
    PLAYER_WHITE = 10,
    PLAYER_BLACK = -10,
    PLAYER_EMPTY = 1000,
} Player;




#include "move.h"

ChessGame *ChessCreateGame(void); //ok
ChessGame *createChessGameFromFEN(char *fen); //ok


void updateWK(ChessGame *game, bool WK);
void updateWQ(ChessGame *game, bool WQ);
void updateBK(ChessGame *game, bool BK);
void updateBQ(ChessGame *game, bool BQ);
void updateEpWhiteCol(ChessGame *game, int col);
void updateEpBlackCol(ChessGame *game, int col);



void updateGListOfFen(ChessGame *game);
void updateGlistOfZobrist(ChessGame *game, unsigned long long key);
void updateCurrentZobritstKey(ChessGame *game, unsigned long long key);
void updateEndGame(ChessGame *game, bool endGame);
void isEndGame(ChessGame *game);





bool getWK(ChessGame *game);
bool getWQ(ChessGame *game);
bool getBK(ChessGame *game);
bool getBQ(ChessGame *game);
int getEpWhiteCol(ChessGame *game);
int getEpBlackCol(ChessGame *game);


bool getEndGame(ChessGame *game);
GList *getZobristList(ChessGame *game);
unsigned long long getCurrentZobristKey(ChessGame *game);






void ChessFree( ChessGame *game);//ok
void UndoMove(ChessGame *game);
void UndoMove2(ChessGame *game);
Player GetNextPlayer(ChessGame *game);
Player GetCaseOwner(ChessGame *game, int row, int col);

bool GameIsPawnMoveValid(ChessGame *game, Move *move);
bool GameIsROOKMoveValid(ChessGame *game, Move *move);
bool GameIsBishopMoveValid(ChessGame *game, Move *move);

void PlayMove(ChessGame *game, Move *move, bool check);//ok
void PlayMove2(ChessGame *game, Move *move, bool check);
void PlayMoveAI(ChessGame *game, Move *move);



bool GameIsValidMove(ChessGame *game, Move *move); //ok
bool GameIsKnightMoveValid(ChessGame *game, Move *move);

Piece GetPieceOnCase(ChessGame *game, int row, int col);
Player GetCurrentPlayer(ChessGame *game);
bool GameIsKingMoveValid(ChessGame *game, Move *move);
void enumprintf(Piece piece);
bool GameIsQueenValidMove(ChessGame *game, Move *move);
bool GameIsCheck(ChessGame *game, Player player);
ChessGame *copyChessGame(const ChessGame *game) ;
void movePieceWithoutVerif(ChessGame *game, int srcRow, int srcCol, int destRow, int destCol, bool updGListOfFen);

bool RoqueIsValidMove(ChessGame *game, Move *move);//ok

void PlayRoqueMove(ChessGame *game, Move *move);



bool PriseEnPassantValidMove(ChessGame *game, Move *move);//ok

void PlayPriseEnPassantMove(ChessGame *game, Move *move);

void printPlayer(Player player);

void PlayPromotion(ChessGame *game, Move *move, int response);
void PlayClassiqueMove(ChessGame *game, Move *move);

bool ChessGameIsMate(ChessGame *game, bool printWhy);

bool ChessGameIsOver(ChessGame *game, bool printWhy);//ok


bool AutreCoupPossible(ChessGame *game, Player player);

bool ChessGameIsPat(ChessGame *game, bool printWhy);

bool ChessGameNoMatMin(ChessGame *game, bool printWhy);

bool ChessGameRepetition(ChessGame *game, bool printWhy);
int getNbCoupInutile(ChessGame *game);
bool CompareGame(ChessGame *g1, ChessGame *g2);

GList *GetALLMove(ChessGame *game, bool onlyCaptures);

GList *GetALLMoveFromCase(ChessGame *game, int start_row, int start_col, bool onlyCaptures);
GList* GetALLPawnMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures);

GList* GetALLKingMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures);
GList* GetALLQueenMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures);
GList* GetALLRookMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures);
GList* GetALLBishopMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures);
GList* GetALLKnightMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures);

bool IsCheckMove(ChessGame *game, Move *move);






#endif
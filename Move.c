
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include <glib.h>

// #include "Move.h"
#include "chess.h"

Move *CreateMove(Player player, int start_row, int start_col, int end_row, int end_col)
{
    Move* newMove = (Move*)malloc(sizeof(Move));
    if (newMove == NULL) {
        fprintf(stderr, "Erreur d'allocation de mémoire pour Move\n");
        return NULL;
    }
    newMove->player = player;
    newMove->start_row = start_row;
    newMove->start_col = start_col;
    newMove->end_row = end_row;
    newMove->end_col = end_col;
    newMove->eval = 0;
    newMove->response = -1;
    newMove->moveType = 0;
    newMove->prise = false;
    newMove->frequency = -1;
    
    
    return newMove;
}

void FreeMove(Move *move)
{
    if(move != NULL)
        free(move);
    return ;
}

int compareMoves(const void *a, const void *b)
{
    const Move *moveA = (const Move *)a;
    const Move *moveB = (const Move *)b;

    if (moveA->moveType > moveB->moveType)
        return -1;
    else if (moveA->moveType < moveB->moveType)
        return 1;
    else
        return 0;
}

int getStartColFromMove(Move *move)
{
    return move->start_col;
}
int getStartRowFromMove(Move *move)
{   
    // fprintf(stderr, "10a\n");
    int start_row = move->start_row;
    // fprintf(stderr, "10b\n");
    return start_row;
}
int getEndColFromMove(Move *move)
{
    return move->end_col;
}
int getEndRowFromMove(Move *move)
{
    return move->end_row;
}


Player getPlayerFromMove(Move *move)
{
    return move->player;
}

int getResponseFromMove(Move *move)
{
    return move->response;
}
double getEvalFromMove(Move *move)
{
    return move->eval;
}

MoveType getMoveTypeFromMove(Move *move)
{
    return move->moveType;
}

int getFrequencyFromMove(Move *move)
{
    return move->frequency;
}


void newStartRow(Move *move, int newStartRow) 
{
    move->start_row = newStartRow;
}

void newStartCol(Move *move, int newStartCol) 
{
    move->start_col = newStartCol;
}

void newEndRow(Move *move, int newEndRow) 
{
    move->end_row = newEndRow;
}

void newEndCol(Move *move, int newEndCol) 
{
    move->end_col = newEndCol;
}

void newPlayer(Move *move, Player newPlayer) 
{
    move->player = newPlayer;
}
void newEval(Move *move, double eval)
{
    move->eval = eval;
}

void newResponse(Move *move, int response)
{
    if(response < 1 || response > 4)
    {
        fprintf(stderr, "response must be included between 1 and 4\n");
        return ;
    }
    move->response = response;
}

void newMoveType(Move *move, MoveType newMoveType)
{
    move->moveType = newMoveType;
}

void newFrequency(Move *move, int frequency)
{
    move->frequency = frequency;
    return ;
}



Move* copy_move(const Move* original) 
{
    Move* new_move = malloc(sizeof(Move));
    if (new_move == NULL) {
        fprintf(stderr, "Erreur d'allocation de mémoire dans copy_move\n");
        return NULL;
    }
    *new_move = *original;  // Copie les valeurs du Move d'origine
    return new_move;
}

void PrintMove(Move *move) 
{
    fprintf(stderr, "Player: %d, Start: (%d, %d), End: (%d, %d), Response: %d, Eval = %lf, moveType = %d\n",
           move->player,
           move->start_row, move->start_col,
           move->end_row, move->end_col,
           move->response, move->eval, move->moveType);
}

// Fonction pour imprimer tous les mouvements de la liste
void PrintAllMoves(GList *moveList) 
{
    GList *iterator = NULL;

    for (iterator = moveList; iterator; iterator = iterator->next) {
        Move *move = (Move *)iterator->data;
        PrintMove(move);
    }
}

void FindMoveType(ChessGame *game, Move *move)
{
    int start_row = move->start_row;
    int end_row = move->end_row;
    int start_col = move->start_col;
    int end_col = move->end_col;

    Piece startPiece = GetPieceOnCase(game, start_row, start_col);
    Piece endPiece = GetPieceOnCase(game, end_row, end_col);

    if((startPiece == WHITE_KING && start_row == 7 && start_col == 4 && (end_col == 6 || end_col == 2)) || (startPiece == BLACK_KING && start_row == 0 && start_col == 4 && (end_col == 6 || end_col == 2)))
        move->moveType = Roque;
    else if((startPiece == WHITE_PAWN && start_row == 3 && start_col != end_col && endPiece == EMPTY_CASE) || (startPiece == BLACK_PAWN && start_row == 4 && start_col != end_col && endPiece == EMPTY_CASE))
        move->moveType = PriseEnPassant;
    else if((startPiece == WHITE_PAWN && end_row == 0) || (startPiece == BLACK_PAWN && end_row == 7))
        move->moveType = Promotion;
    else if(endPiece != EMPTY_CASE)
        move->moveType = Prise;
    else
        move->moveType = Classique;

    return ;
}

Piece getPromotedPiece(Player player, int response)
{
    if (response < 1 || response > 4) 
    {
        printf("error: invalid promotion choice\n");
        return EMPTY_CASE;
    }

    if (player == PLAYER_WHITE)
    {
        switch (response)
        {
            case 1:
                return WHITE_QUEEN;
            case 2:
                return WHITE_ROOK;
            case 3:
                return WHITE_BISHOP;
            case 4:
                return WHITE_KNIGHT;
            default:
                return EMPTY_CASE; // Shouldn't reach here
        }
    }
    else if (player == PLAYER_BLACK)
    {
        switch (response)
        {
            case 1:
                return BLACK_QUEEN;
            case 2:
                return BLACK_ROOK;
            case 3:
                return BLACK_BISHOP;
            case 4:
                return BLACK_KNIGHT;
            default:
                return EMPTY_CASE; // Shouldn't reach here
        }
    }
    else
    {
        printf("error: invalid player\n");
        return EMPTY_CASE;
    }
}

Move *convertMouseClicksToMove(int *tab, ChessGame *game, Player user) 
{
    int start_row, start_col, end_row, end_col;
    start_row = tab[0];
    start_col = tab[1];
    end_row = tab[2];
    end_col = tab[3];
    if(user == PLAYER_BLACK)
    {
        start_row = 7 - start_row;
        start_col = 7 - start_col;
        end_row = 7 - end_row;
        end_col = 7 - end_col;
    }
    Move *move = CreateMove(GetCaseOwner(game, start_row, start_col), start_row, start_col, end_row, end_col); // Allouer dynamiquement la mémoire pour la structure Move
    FindMoveType(game, move);

    
    if (move == NULL) {
        // Gestion de l'échec d'allocation mémoire
        printf("Erreur lors de l'allocation mémoire pour Move.\n");
        return NULL;
    }

    
    return move;
}
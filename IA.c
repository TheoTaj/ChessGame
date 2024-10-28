
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include <time.h>
#include <glib.h>

#include "IA.h"
#include "chess.h" 
#include "Zobrist.h"
#include "GMmove.h"


#define BOARD_SIZE 8

#define DEPTH 4
#define QUIESCENTDEPTH 3

extern int nbPositions;
extern int depth;
extern int totalLength;
extern int nbZobrist;
extern int nbZobrist2;


GHashTable *transpositionTable;
GHashTable *gmTable;



double getEval(ChessGame *game)
{
    double eval = 0;
    for(int row = 0; row<BOARD_SIZE; row++)
    {
        for(int col = 0; col<BOARD_SIZE; col++)
        {
            Piece current_piece = GetPieceOnCase(game, row, col);
            switch(current_piece)
            {
                case EMPTY_CASE:
                {
                    break;
                }
                case WHITE_KING:
                {
                    double value = pieceValues[VALUE_WHITE_KING];
                    if(getEndGame(game))
                        value += WhiteKingEnd[row][col];
                    else
                        value += WhiteKingStart[row][col];

                    eval += value;
                    break;
                }
                case WHITE_QUEEN:
                {
                    double value = pieceValues[VALUE_WHITE_QUEEN] + WhiteQueens[row][col];
                    eval += value;
                    break;
                }
                case WHITE_BISHOP:
                {
                    double value = pieceValues[VALUE_WHITE_BISHOP] + WhiteBishops[row][col];
                    eval += value;
                    break;
                }
                case WHITE_KNIGHT:
                {
                    double value = pieceValues[VALUE_WHITE_KNIGHT] + WhiteKnights[row][col];
                    eval += value;
                    break;
                }
                case WHITE_ROOK:
                {
                    double value = pieceValues[VALUE_WHITE_ROOK] + WhiteRook[row][col];
                    eval += value;
                    break;
                }
                case WHITE_PAWN:
                {
                    double value = pieceValues[VALUE_WHITE_PAWN];
                    if(getEndGame(game))
                        value += WhitePawnEnd[row][col];
                    else
                        value += WhitePawnStart[row][col];

                    eval += value;
                    break;
                }
                case BLACK_KING:
                {
                    double value = pieceValues[VALUE_BLACK_KING];
                    if(getEndGame(game))
                        value += BlackKingEnd[row][col];
                    else
                        value += BlackKingStart[row][col];

                    eval += value;
                    break;
                }
                case BLACK_QUEEN:
                {
                    double value = pieceValues[VALUE_BLACK_QUEEN] + BlackQueens[row][col];
                    eval += value;
                    break;
                }
                case BLACK_BISHOP:
                {
                    double value = pieceValues[VALUE_BLACK_BISHOP] + BlackBishops[row][col];
                    eval += value;
                    break;
                }
                case BLACK_KNIGHT:
                {
                    double value = pieceValues[VALUE_BLACK_KNIGHT] + BlackKnights[row][col];
                    eval += value;
                    break;
                }
                case BLACK_ROOK:
                {
                    double value = pieceValues[VALUE_BLACK_ROOK] + BlackRook[row][col];
                    eval += value;
                    break;
                }
                case BLACK_PAWN:
                {
                    double value = pieceValues[VALUE_BLACK_PAWN];
                    if(getEndGame(game))
                        value += BlackPawnEnd[row][col];
                    else
                        value += BlackPawnStart[row][col];

                    eval += value;
                    break;
                }

            }
        }
    }

    if(getEndGame(game) && eval > 0)
    {
        eval += forceKingToTheEdge(game, PLAYER_WHITE);
    }
    else if(getEndGame(game) && eval < 0)
    {
        eval -= forceKingToTheEdge(game, PLAYER_BLACK);
    }

    if(ChessGameIsMate(game, false)) //Les blans ont mis les noirs en échec et math
    {
        // fprintf(stderr, "in the if!\n");
        if(GetCurrentPlayer(game) == PLAYER_BLACK)
            eval = 5000;
        else
            eval = -5000;
    }
    else if(ChessGameIsPat(game, false) || ChessGameNoMatMin(game, false) || ChessGameRepetition(game, false) || getNbCoupInutile(game) >= 100)
    {
        eval = 0;
    }
   

    return eval;
}
Move *IAGetMove(ChessGame *game)
{
   
    depth = DEPTH;
    Move *bestMove = NULL;
    ChessGame *copy = copyChessGame(game);
    Player player = GetCurrentPlayer(game);

    unsigned long long currentZobristKey = getCurrentZobristKey(game);
    GMmove *gmMove = (GMmove *)g_hash_table_lookup(gmTable, &currentZobristKey);

    if(gmMove != NULL)
    {
        Move *bestMove = getRandGMmove(gmMove);
        
        if(bestMove == NULL)
        {
            fprintf(stderr, "Error in IAGetMove\n");
        }
        newPlayer(bestMove, player);
        FindMoveType(game, bestMove);
        fprintf(stderr, "Move from GM\n");
        ChessFree(copy);
        return copy_move(bestMove);
    }
    
    if(player == PLAYER_WHITE)
    {
        bestMove = IAGetMoveRec(copy, depth, true, -10001, 10001);
    }
    else 
    {
        bestMove = IAGetMoveRec(copy, depth, false, -10001, 10001);   
    }
    ChessFree(copy);
    

    return bestMove;



}

//alpha = la max de que le maximiseur est sur de pouvoir obtenir
//beta = le min que le minimiseur est sur de pouvoir obtenir

Move *IAGetMoveRec(ChessGame *game, int depth, bool maximizingPlayer , double alpha, double beta)
{

    if(depth < 1)
    {
        fprintf(stderr, "Error in IAGetMoveRec\n");
        return NULL;
    }
    
    
    Move *bestMove = NULL;
    double bestEval;

    unsigned long long currentZobristKey = getCurrentZobristKey(game);
    
    // Vérifier la table de transposition
    ZobristMove *transpositionMove = (ZobristMove *)g_hash_table_lookup(transpositionTable, &currentZobristKey);

    if (transpositionMove != NULL && getDepthFromZobristMove(transpositionMove) >= depth)
    {
        ChessGame *copy = copyChessGame(game);
        PlayMoveAI(copy, getBestMoveFromZobristMove(transpositionMove));
        

        if( !ChessGameRepetition(copy, false) && !(getNbCoupInutile(copy) >= 100) )
        {
            // fprintf(stderr, "here\n");
            nbZobrist++;
            bestEval = getEvalFromZobristMove(transpositionMove);
            bestMove = copy_move(getBestMoveFromZobristMove(transpositionMove));
            bestMove->eval = bestEval;
            ChessFree(copy);
            return bestMove;
        }

        ChessFree(copy);
    }

    // fprintf(stderr, "AVANT__________________________________________\n\n");
    GList *moveList = GetALLMove(game, false); // Liste de tous les coups valides
    // fprintf(stderr, "APRES__________________________________________\n\n");

    if (maximizingPlayer) 
    {
        
        bestEval = -10001;
        
        for (GList *l = moveList; l != NULL; l = l->next) 
        {
            
            double currentEval;
            Move *responseMove = NULL;
            Move* move = (Move*) l->data;
            ChessGame *copy = copyChessGame(game);
            PlayMoveAI(copy, move);

            if(ChessGameIsOver(copy, false))
            {
                currentEval = getEval(copy);
                if(currentEval == 5000)
                    currentEval += depth;

                newEval(move, currentEval);
                nbPositions++;
            }
            else if(depth == 1)
            {
                currentEval = evaluateCaptures(copy, QUIESCENTDEPTH ,false, alpha, beta);
                newEval(move, currentEval);
            }
            else
            {
                responseMove = IAGetMoveRec(copy, depth - 1, false, alpha, beta);
                
                currentEval = getEvalFromMove(responseMove);
                newEval(move, currentEval);
            }

            if (currentEval > bestEval) 
            {
                bestEval = currentEval;
                if (bestMove != NULL) 
                {
                    FreeMove(bestMove);
                }
                bestMove = copy_move(move);
                bestMove->eval = bestEval;
            }

            ChessFree(copy);

            if(responseMove != NULL)
                FreeMove(responseMove);
    
            if(alpha < bestEval)
            {
                alpha = bestEval;
            }
            if(beta <= alpha)
            {
                break;
            }
            
        }

    } 
    else 
    {
        bestEval = 10001;
        for (GList *l = moveList; l != NULL; l = l->next) 
        {
            
            Move* move = (Move*) l->data;
            Move *responseMove = NULL;
            double currentEval;
            ChessGame *copy = copyChessGame(game);
            PlayMoveAI(copy, move);

            if(ChessGameIsOver(copy, false))
            {
                currentEval = getEval(copy);
                if(currentEval == -5000)
                    currentEval -= depth;
                
                newEval(move, currentEval);
                nbPositions++;
            }
            else if(depth == 1)
            {
                currentEval = evaluateCaptures(copy, QUIESCENTDEPTH, true, alpha, beta);
                newEval(move, currentEval);
            }
            else
            {
                responseMove = IAGetMoveRec(copy, depth - 1, true, alpha, beta);
                currentEval = getEvalFromMove(responseMove);
                newEval(move, currentEval);
                
            }

            if (currentEval < bestEval) 
            {
                bestEval = currentEval;
                if (bestMove != NULL) 
                {
                    FreeMove(bestMove);
                }
                bestMove = copy_move(move);
                bestMove->eval = bestEval;
            }

            ChessFree(copy);
            
            if(responseMove != NULL)
                FreeMove(responseMove);


            if(beta > bestEval)
            {
                beta = bestEval;
            }
            if (beta <= alpha) 
            {
                break; // Alpha cut-off
            }


           
        }
    }

    ZobristMove *zobristMove = createZobristMove(currentZobristKey, bestEval, depth, copy_move(bestMove));
    if (zobristMove != NULL)
    {
        insertTranspositionTable(zobristMove);
    }

    g_list_free_full(moveList, free);
    
    return bestMove;
}

double evaluateCaptures(ChessGame *game, int quiescentDepth, bool maximizingPlayer, double alpha, double beta) 
{
    double firstEval = getEval(game);
    if(quiescentDepth < 1)
    {
        return firstEval;
    }
    double eval;
    unsigned long long currentZobristKey = getCurrentZobristKey(game);
    ZobristMove *transpositionMove = (ZobristMove *)g_hash_table_lookup(transpositionTable, &currentZobristKey);

    int depth = getNegDepthFromQuiescentDepth(quiescentDepth);

    

    if (transpositionMove != NULL && getDepthFromZobristMove(transpositionMove) >= depth)
    {
        nbZobrist2++;
        
        eval = getEvalFromZobristMove(transpositionMove);

        return eval;
    }

    
    if(quiescentDepth == 1)
    {
        nbPositions++;
        return firstEval;
    }

        
    // fprintf(stderr, "AVANT__________________________________________\n\n");

    GList *captureMoves = GetALLMove(game, true);  // Récupère tous les coups de capture possibles    
    // fprintf(stderr, "APRES__________________________________________\n\n");

    int sizeOfCaptureMoves = g_list_length(captureMoves);
    if(sizeOfCaptureMoves == 0 || captureMoves == NULL) 
    {
        nbPositions++;
        return firstEval;  // Si aucune capture, retourne l'évaluation actuelle de la position
    }

    
    if (maximizingPlayer) 
    {
        eval = firstEval;
        for (GList *l = captureMoves; l != NULL; l = l->next) 
        {
            double currentEval;
            Move *move = (Move*) l->data;
            if(move == NULL)
                fprintf(stderr, "MOVE == NULL");
            ChessGame *copy = copyChessGame(game);
         
            PlayMoveAI(copy, move);  // Joue la capture
            
            
            // Continue l'évaluation récursive sur les captures suivantes
            if(ChessGameIsOver(copy, false))
            {
                currentEval = getEval(copy);
                nbPositions++;

            }
            else
                currentEval = evaluateCaptures(copy, quiescentDepth -1, false, alpha, beta);
            
            if (currentEval > eval) 
            {
                eval = currentEval;
            }

            ChessFree(copy);
            if(alpha < eval)
            {
                alpha = eval;
            }
            if(beta <= alpha)
            {
                break;
            }
        }
    } 
    else 
    {
        eval = firstEval;
        for (GList *l = captureMoves; l != NULL; l = l->next) 
        {
            double currentEval;
            Move *move = (Move*) l->data;
            ChessGame *copy = copyChessGame(game);
            PlayMoveAI(copy, move);  // Joue la capture
            
            // Continue l'évaluation récursive sur les captures suivantes
            if(ChessGameIsOver(copy, false))
            {
                nbPositions++;
                currentEval = getEval(copy);
            }
            else
                currentEval = evaluateCaptures(copy, quiescentDepth -1, true, alpha, beta);
            
            if (currentEval < eval) 
            {
                eval = currentEval;
            }


            ChessFree(copy);
            
            if(beta > eval)
            {
                beta = eval;
            }
            if (beta <= alpha) 
            {
                break; // Alpha cut-off
            }
        }
    }

    ZobristMove *zobristMove = createZobristMove(currentZobristKey, eval, depth, NULL);
    if (zobristMove != NULL)
    {
        insertTranspositionTable(zobristMove);
    }

    g_list_free_full(captureMoves, free);
    return eval;
}




int countPosition(ChessGame *game, int depth) 
{
    if (depth == 0) {
        return 1;
    }

    GList *moveList = GetALLMove(game, false); // Liste de tous les coups valides
    int positionCount = 0;

    for (GList *l = moveList; l != NULL; l = l->next) {
        Move* move = (Move*) l->data;
        ChessGame *copy = copyChessGame(game);
        PlayMoveAI(copy, move);
        positionCount += countPosition(copy, depth - 1);
        ChessFree(copy);
        FreeMove(move);
    }

    g_list_free(moveList); // Libération de la mémoire de moveList

    return positionCount;
}

int getNegDepthFromQuiescentDepth(int quiescentDepth)
{
    if(quiescentDepth == 3)
        return -1;
    else if(quiescentDepth == 2)
        return -2;
    else if(quiescentDepth == 1)
        return -3;
    else
        fprintf(stderr, "Error in getNegDepthFromQuiescentDepth\n");
    return 0;
}

Value pieceToValue(Piece piece) 
{
    switch (piece) {
        case WHITE_KING:
            return VALUE_WHITE_KING;
        case WHITE_QUEEN:
            return VALUE_WHITE_QUEEN;
        case WHITE_BISHOP:
            return VALUE_WHITE_BISHOP;
        case WHITE_KNIGHT:
            return VALUE_WHITE_KNIGHT;
        case WHITE_ROOK:
            return VALUE_WHITE_ROOK;
        case WHITE_PAWN:
            return VALUE_WHITE_PAWN;
        case BLACK_KING:
            return VALUE_BLACK_KING;
        case BLACK_QUEEN:
            return VALUE_BLACK_QUEEN;
        case BLACK_BISHOP:
            return VALUE_BLACK_BISHOP;
        case BLACK_KNIGHT:
            return VALUE_BLACK_KNIGHT;
        case BLACK_ROOK:
            return VALUE_BLACK_ROOK;
        case BLACK_PAWN:
            return VALUE_BLACK_PAWN;
        case EMPTY_CASE:
            return VALUE_EMPTY_CASE;
        default:
            fprintf(stderr, "Error in pieceToValue\n");
            return VALUE_COUNT; // Valeur indiquant une erreur ou une valeur invalide
    }
}

//player correspond au joueur qui est en train de gagner
double forceKingToTheEdge(ChessGame *game, Player winingPlayer)
{
    double winingKingRow, winingKingCol;
    double loosingKingRow, loosingKingCol;

    winingKingRow = -1;
    winingKingCol = -1;
    loosingKingRow = -1;
    loosingKingCol = -1;


    for(int row = 0; row < BOARD_SIZE; row++)
    {
        for(int col = 0; col < BOARD_SIZE; col++)
        {
            Piece piece = GetPieceOnCase(game, row, col);

            if(winingPlayer == PLAYER_WHITE) 
            {
                if(piece == WHITE_KING)
                {
                    winingKingRow = (double)row;
                    winingKingCol = (double)col;
                }
                else if(piece == BLACK_KING)
                {
                    loosingKingRow = (double)row;
                    loosingKingCol = (double)col;
                }
            }
            else
            {
                if(piece == WHITE_KING)
                {
                    loosingKingRow = (double)row;
                    loosingKingCol = (double)col;
                }
                else if(piece == BLACK_KING)
                {
                    winingKingRow = (double)row;
                    winingKingCol = (double)col;
                }
            }
        }
    } 


    if(winingKingRow == -1 || winingKingCol == -1 || loosingKingRow == -1 || loosingKingCol == -1)
    {
        fprintf(stderr, "Error in forceKingToTheEdge\n");
        return 0;
    }

    double bonus = 0;

    double centerRow, centerCol;

    centerRow = 3.5;
    centerCol = 3.5;

    double deltaX = fabs(centerRow - loosingKingRow);
    double deltaY = fabs(centerCol - loosingKingCol);

    double distanceFromCenter = deltaX + deltaY;

    deltaX = fabs(winingKingRow - loosingKingRow);
    deltaY = fabs(winingKingCol - loosingKingCol);

    double distanceBeetwennKings = deltaX + deltaY;


    //on met un bonus si ke roi adverse est bien éloigné du centre
    bonus = distanceFromCenter;

    //et un met un malus si le roi gagnant est éloigné du roi adverse
    // bonus -= distanceBeetwennKings;

    bonus += (14 - (distanceBeetwennKings) ) * 1.5;


    bonus = bonus * 0.1;

    // fprintf(stderr, "Bonus = %lf\n", bonus);

   
    return bonus;

}
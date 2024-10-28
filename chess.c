
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include <glib.h>
#include<string.h>
#include<ctype.h>


#include "chess.h"
#include "Move.h"
#include "fen.h"
#include "Zobrist.h"


#define BOARD_SIZE 8
#define MAX_FEN_LENGTH 100


int nbPositions;
int nbClassique;
int nbPromotion;
int nbRoque;
int nbPriseEnPassant;
int nbPrise;
int depth;
int totalLength;
int nbZobrist;
int nbZobrist2;

struct ChessGame_t
{
    Piece **board;
    Player current_player;
    bool WK; // 1 if white can still king side castle
    bool WQ;
    bool BK;
    bool BQ;
    int epWhiteCol; // -1 si les blancs ne peuvent pas prendre en passant
    int epBlackCol;
    int nbCoupInutile;
    GList *fenList;
    GList *zobristList;
    unsigned long long currentZobristKey;
    bool endGame;
};



ChessGame *ChessCreateGame(void)
{
    ChessGame *newgame;
    newgame = malloc(sizeof(ChessGame));
    if(!newgame)
        return NULL;
    newgame->current_player = PLAYER_WHITE;

    newgame->board = (Piece**)malloc(BOARD_SIZE * sizeof(Piece*));
    if(!(newgame->board)){
        free(newgame);
        return NULL;
    }

    int i, j;
    for( i = 0; i<BOARD_SIZE; i++){
        newgame->board[i] = (Piece *)malloc(BOARD_SIZE * sizeof(Piece));
        if(newgame->board[i] == NULL)
        {
            for(j =0; j<i; j++){
                free(newgame->board[j]);
            }
            free(newgame->board);
            free(newgame);
            return NULL;
        }
    }

    newgame->WK = false;
    newgame->WQ = false;
    newgame->BK = false;
    newgame->BQ = false;
    newgame->epWhiteCol = -1;
    newgame->epBlackCol = -1;
    newgame->nbCoupInutile = 0;
    newgame->fenList = NULL;
    newgame->zobristList = NULL;
    return newgame; 

}
ChessGame *createChessGameFromFEN(char *fen)
{
   
    ChessGame *game = ChessCreateGame();

    
    game->fenList = g_list_append(game->fenList, copyString(fen));


   
    
    int length = strlen(fen);
    int row, col, i;
    for(row = 0; row<BOARD_SIZE; row++)
    {
        for(col = 0; col<BOARD_SIZE; col++)
        {
            game->board[row][col] = EMPTY_CASE;
        }
    }
    row = 0;
    col = 0;

    int nbSpace = 0;
    

    

    for(i = 0; i < length; i++)
    {
        
        char c = fen[i];
        

        if(c == ' ')
        {
            nbSpace++;
            continue;
        }
        if(nbSpace == 0)
        {
            
            
            if(c == '/' || col > 7)
            {
                row++;
                col = 0;
                continue;
            }
            else if(isdigit(c))
            {
                int num = c - '0';
                
                col += num;
                continue;
            }
            else if(isPiece(c))
            {
                Piece piece = pieceFromChar(c);
            
                game->board[row][col] = piece;
                
            }
            col++;

            

        }
        
        else if(nbSpace == 1)
        {
           

            if(c == 'w')
                game->current_player = PLAYER_WHITE;
            else if(c == 'b')
                game->current_player = PLAYER_BLACK;
            
           
        }
        
        else if(nbSpace == 2)
        {

            if(c == '-')
                continue;
            else if(c == 'K')
            {
    
                game->WK = true;
            }
            else if(c == 'Q')
            {
                game->WQ = true;
            }
            else if(c == 'k')
            {
                game->BK = true;
            }
            else if(c == 'q')
            {
                game->BQ = true;
            }
        }
        else if(nbSpace == 3)
        {   
            if(c == '-')
            {
                continue;
            }
            else if(!isdigit(c))
            {
                continue;
            }
            else
            {
                char prev = fen[i-1];

                if(c == '6')
                {
                    game->epWhiteCol = getColumnIndex(prev);
                }
                else if(c == '3')
                {
                    game->epBlackCol = getColumnIndex(prev);
                }
            }

        }
        else if(nbSpace == 4)
        {
            int size = 0;
            for(int j = i; ;j++)
            {
                char l = fen[j];
                if( !isdigit(l) || l == ' ' ||l == '\0')
                {
                    break;
                }
                else
                    size++;


            }
            char *temp = malloc(sizeof(char) * (size + 1));
            int m = i;
            int k;
            for(k = 0; k < size; k++)
            {
                temp[k] = fen[m];
                m++;
            }
            temp[size] = '\0';
            game->nbCoupInutile = atoi(temp);

            free(temp);

            break;
        }
        

    }

  
    game->currentZobristKey = getZobristKey(game);
    updateGlistOfZobrist(game, game->currentZobristKey);

    isEndGame(game);

    return game;

}

void ChessFree(ChessGame *game)
{
    for(int i =0; i<BOARD_SIZE; i++){
        free(game->board[i]);
    }
    free(game->board);
    if(game->fenList != NULL)
    {
        g_list_free_full(game->fenList, free);
    }
    freeGlistOfZobrist(game->zobristList);

    free(game);
}

ChessGame *copyChessGame(const ChessGame *game) 
{
    if (game == NULL) {
        return NULL;
    }

    ChessGame *copy = malloc(sizeof(ChessGame));
    if (copy == NULL) {
        // Gérer l'échec d'allocation de mémoire
        return NULL;
    }

    copy->current_player = game->current_player;
   

    copy->WK = game->WK;
    copy->WQ = game->WQ;
    copy->BK = game->BK;
    copy->BQ = game->BQ;
   
    
    copy->epWhiteCol = game->epWhiteCol;
    copy->epBlackCol = game->epBlackCol;

    copy->nbCoupInutile = game->nbCoupInutile;
    // copy->fenList = copyGListOfFen(game->fenList);
    copy->fenList = NULL;
    copy->zobristList = copyGlistOfZobrist(game->zobristList);
    copy->currentZobristKey = game->currentZobristKey;
    copy->endGame = game->endGame;

    // Allouer de la mémoire pour la copie du tableau
    copy->board = malloc(BOARD_SIZE * sizeof(Piece *));
    if (copy->board == NULL) {
        // Gérer l'échec d'allocation de mémoire
        free(copy);
        return NULL;
    }

    // Copier chaque ligne du tableau
    for (int row = 0; row < BOARD_SIZE; ++row) 
    {
        copy->board[row] = malloc(BOARD_SIZE * sizeof(Piece));
        if (copy->board[row] == NULL) 
        {
            // Gérer l'échec d'allocation de mémoire
            free(copy->board);
            free(copy);
            return NULL;
        }

        // Copier chaque colonne du tableau
        for (int col = 0; col < BOARD_SIZE; ++col) 
        {
            copy->board[row][col] = game->board[row][col];
        }
    }

    return copy;
}




int getNbCoupInutile(ChessGame *game)
{
    return game->nbCoupInutile;
}



void updateWK(ChessGame *game, bool WK)
{
    game->WK = WK;
}

void updateWQ(ChessGame *game, bool WQ)
{
    game->WQ = WQ;
}

void updateBK(ChessGame *game, bool BK)
{
    game->BK = BK;
}

void updateBQ(ChessGame *game, bool BQ)
{
    game->BQ = BQ;
}

void updateEpWhiteCol(ChessGame *game, int col)
{
    game->epWhiteCol = col;
}

void updateEpBlackCol(ChessGame *game, int col)
{
    game->epBlackCol = col;
}

void updateCurrentZobritstKey(ChessGame *game, unsigned long long key)
{
    game->currentZobristKey = key;
}


void updateGListOfFen(ChessGame *game)
{
    char *fen = createFEN(game);
    game->fenList = g_list_append(game->fenList, fen);
}



// void updateGlistOfZobrist(ChessGame *game, unsigned long long key)
// {
//     game->zobristList = g_list_append(game->zobristList, GINT_TO_POINTER(key));
// }

void updateGlistOfZobrist(ChessGame *game, unsigned long long key)
{   
    unsigned long long *keyPoint = (unsigned long long *)malloc(sizeof(unsigned long long));

    *keyPoint = key; 
    game->zobristList = g_list_append(game->zobristList, keyPoint);

    return ;
}
void updateEndGame(ChessGame *game, bool endGame)
{
    game->endGame = endGame;
}

void isEndGame(ChessGame *game)
{
    double total = 0;

    for(int row = 0; row < BOARD_SIZE; row++)
    {
        for(int col = 0; col < BOARD_SIZE; col++)
        {
            Piece piece = GetPieceOnCase(game, row, col);
            switch(piece)
            {
                case EMPTY_CASE:
                {
                    break;
                }
                case WHITE_KING:
                case BLACK_KING:
                {
                    total += pieceValues[VALUE_WHITE_KING];                    
                    break;
                }
                case WHITE_QUEEN:
                case BLACK_QUEEN:
                {
                    total += pieceValues[VALUE_WHITE_QUEEN];                    
                    break;
                }
                case WHITE_BISHOP:
                case BLACK_BISHOP:
                {
                    total += pieceValues[VALUE_WHITE_BISHOP];                    
                    break;
                }
                case WHITE_KNIGHT:
                case BLACK_KNIGHT:
                {
                    total += pieceValues[VALUE_WHITE_KNIGHT];                    
                    break;
                }
                case WHITE_ROOK:
                case BLACK_ROOK:
                {
                    total += pieceValues[VALUE_WHITE_ROOK];                    
                    break;
                }
                case WHITE_PAWN:
                case BLACK_PAWN:
                {
                    total += pieceValues[VALUE_WHITE_PAWN];                    
                    break;
                }

            }
        }
    }

    if(total <= 40)
    {
        updateEndGame(game, true);
    }
    else 
        updateEndGame(game, false);

    return ;

}



bool getWK(ChessGame *game)
{
    return game->WK;
}

bool getWQ(ChessGame *game)
{
    return game->WQ;
}

bool getBK(ChessGame *game)
{
    return game->BK;
}

bool getBQ(ChessGame *game)
{
    return game->BQ;
}

int getEpWhiteCol(ChessGame *game)
{
    return game->epWhiteCol;
}
int getEpBlackCol(ChessGame *game)
{
    return game->epBlackCol;
}

unsigned long long getCurrentZobristKey(ChessGame *game)
{
    return game->currentZobristKey;
}

bool getEndGame(ChessGame *game)
{
    return game->endGame;
}

GList *getZobristList(ChessGame *game)
{
    return game->zobristList;
}



Player GetNextPlayer(ChessGame *game)
{
    if(game->current_player == PLAYER_WHITE)
        return PLAYER_BLACK;
    return PLAYER_WHITE;
}
Player GetCurrentPlayer(ChessGame *game)
{
    return game->current_player;
}

Player GetCaseOwner(ChessGame *game, int row, int col)
{
    int res = game->board[row][col];
    if(res == 0)
        return PLAYER_EMPTY;
    
    if(res > 0)
        return PLAYER_WHITE;
    
    return PLAYER_BLACK;
}
Piece GetPieceOnCase(ChessGame *game, int row, int col)
{
    return game->board[row][col];
}
void enumprintf(Piece piece)
{
    switch (piece) {
    case EMPTY_CASE:
        fprintf(stderr, "Empty Case");
        break;
    case WHITE_KING:
        fprintf(stderr, "White King");
        break;
    case WHITE_QUEEN:
        fprintf(stderr, "White Queen");
        break;
    case WHITE_BISHOP:
        fprintf(stderr, "White Bishop");
        break;
    case WHITE_KNIGHT:
        fprintf(stderr, "White Knight");
        break;
    case WHITE_ROOK:
        fprintf(stderr, "White Rook");
        break;
    case WHITE_PAWN:
        fprintf(stderr, "White Pawn");
        break;
    case BLACK_KING:
        fprintf(stderr, "Black King");
        break;
    case BLACK_QUEEN:
        fprintf(stderr, "Black Queen");
        break;
    case BLACK_BISHOP:
        fprintf(stderr, "Black Bishop");
        break;
    case BLACK_KNIGHT:
        fprintf(stderr, "Black Knight");
        break;
    case BLACK_ROOK:
        fprintf(stderr, "Black Rook");
        break;
    case BLACK_PAWN:
        fprintf(stderr, "Black Pawn");
        break;
    default:
        fprintf(stderr, "Unknown Piece");
        break;
    }
}

void printPlayer(Player player) 
{
    if (player == PLAYER_WHITE) {
        printf("blanc");
    } else if (player == PLAYER_BLACK) {
        printf("noir");
    } else {
        printf("Joueur vide\n");
    }
}

void PlayMove(ChessGame *game, Move *move, bool check)
{
    unsigned long long newZobristKey = getUpdZobristKey(game, move);
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);   
    end_col = getEndColFromMove(move);
    Player player = GetCurrentPlayer(game);
    Piece arrivePiece = GetPieceOnCase(game, end_row, end_col);
    

    bool wasValidMove = false;
   
    // fprintf(stderr, "startrow = %d, startcol = %d\n", start_row, start_col);
    Piece startPiece = game->board[start_row][start_col];
    // fprintf(stderr, "1c\n");

    if(check == true)
    {

        if( (GameIsValidMove(game, move)) &&  ( (startPiece == WHITE_PAWN && end_row == 0) || (startPiece == BLACK_PAWN && end_row == 7 )) )
        {
            PlayPromotion(game, move, getResponseFromMove(move));
            wasValidMove = true;
        }
        else if(GameIsValidMove(game, move))
        {
            PlayClassiqueMove(game, move);
            wasValidMove = true;
        }
        else if(RoqueIsValidMove(game, move))
        {
            wasValidMove = true;
            PlayRoqueMove(game, move);
        }
        else if(PriseEnPassantValidMove(game, move))
        {
            wasValidMove = true;
            PlayPriseEnPassantMove(game, move);
        }
    }
    else
    {
        
        wasValidMove = true;
        MoveType moveType = getMoveTypeFromMove(move);

        if(moveType == Classique || moveType == Prise)
            PlayClassiqueMove(game, move);
        else if(moveType == Promotion)
            PlayPromotion(game, move, getResponseFromMove(move));
        else if(moveType == Roque)
            PlayRoqueMove(game, move);
        else if(moveType == PriseEnPassant)
            PlayPriseEnPassantMove(game, move);
    }

    // fprintf(stderr, "1d\n");

    

    if(wasValidMove)
    {
        if(player == PLAYER_WHITE && startPiece == WHITE_PAWN && start_row == 6 && end_row == 4)
        {
            updateEpBlackCol(game, start_col);
        }
        else if(player == PLAYER_BLACK && startPiece == BLACK_PAWN && start_row == 1 && end_row == 3)
        {
            updateEpWhiteCol(game, start_col);
        }

        if(startPiece == BLACK_KING)
        {
            game->BK = false;
            game->BQ = false;
            
        }        
        else if(startPiece == WHITE_KING)
        {
            game->WK = false;
            game->WQ = false;
           
        }
        else if(startPiece == BLACK_ROOK && start_col ==0)
        {
            game->BQ = false;
        }

        else if(startPiece == BLACK_ROOK && start_col == 7)
        {
            game->BK = false;
        }        
        else if(startPiece == WHITE_ROOK && start_col ==0)
        {
            game->WQ = false;
        }
        else if(startPiece == WHITE_ROOK && start_col ==7)
        {
            game->WK = false;
        }

        if(player == PLAYER_BLACK)
        {
            updateEpBlackCol(game, -1);
        }

        if(player == PLAYER_WHITE)
        {
            updateEpWhiteCol(game, -1);
        }

        if(arrivePiece == EMPTY_CASE && startPiece != WHITE_PAWN && startPiece != BLACK_PAWN)
        {
            game->nbCoupInutile++;
        }
        else
        {
            game->nbCoupInutile = 0;
        }
        
        updateGListOfFen(game);
        
        // fprintf(stderr, "start\n");
        updateCurrentZobritstKey(game, newZobristKey);
        updateGlistOfZobrist(game, newZobristKey);

        if(getEndGame(game) == false)
        {
            isEndGame(game);
        }
        // fprintf(stderr, "end\n");
    }

    return ;
}

//ici on update pas le GLISTOFFEN ni LE GLIST OF ZOBRIST
void PlayMove2(ChessGame *game, Move *move, bool check)
{
    
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);   
    end_col = getEndColFromMove(move);
    Player player = GetCurrentPlayer(game);
    Piece arrivePiece = GetPieceOnCase(game, end_row, end_col);
    

    bool wasValidMove = false;
   
    // fprintf(stderr, "startrow = %d, startcol = %d\n", start_row, start_col);
    Piece startPiece = game->board[start_row][start_col];
    // fprintf(stderr, "1c\n");

    if(check == true)
    {

        if( (GameIsValidMove(game, move)) &&  ( (startPiece == WHITE_PAWN && end_row == 0) || (startPiece == BLACK_PAWN && end_row == 7 )) )
        {
            PlayPromotion(game, move, getResponseFromMove(move));
            wasValidMove = true;
        }
        else if(GameIsValidMove(game, move))
        {
            PlayClassiqueMove(game, move);
            wasValidMove = true;
        }
        else if(RoqueIsValidMove(game, move))
        {
            wasValidMove = true;
            PlayRoqueMove(game, move);
        }
        else if(PriseEnPassantValidMove(game, move))
        {
            wasValidMove = true;
            PlayPriseEnPassantMove(game, move);
        }
    }
    else
    {
        
        wasValidMove = true;
        MoveType moveType = getMoveTypeFromMove(move);

        if(moveType == Classique || moveType == Prise)
            PlayClassiqueMove(game, move);
        else if(moveType == Promotion)
            PlayPromotion(game, move, getResponseFromMove(move));
        else if(moveType == Roque)
            PlayRoqueMove(game, move);
        else if(moveType == PriseEnPassant)
            PlayPriseEnPassantMove(game, move);
    }


    

    if(wasValidMove)
    {
        if(player == PLAYER_WHITE && startPiece == WHITE_PAWN && start_row == 6 && end_row == 4)
        {
            updateEpBlackCol(game, start_col);
        }
        else if(player == PLAYER_BLACK && startPiece == BLACK_PAWN && start_row == 1 && end_row == 3)
        {
            updateEpWhiteCol(game, start_col);
        }
        if(startPiece == BLACK_KING)
        {
            game->BK = false;
            game->BQ = false;
           
        }        
        else if(startPiece == WHITE_KING)
        {
            game->WK = false;
            game->WQ = false;
            
        }
        else if(startPiece == BLACK_ROOK && start_col ==0)
        {
            game->BQ = false;
        }

        else if(startPiece == BLACK_ROOK && start_col == 7)
        {
            game->BK = false;
        }        
        else if(startPiece == WHITE_ROOK && start_col ==0)
        {
            game->WQ = false;
        }
        else if(startPiece == WHITE_ROOK && start_col ==7)
        {
            game->WK = false;
        }

        if(player == PLAYER_BLACK)
        {
            updateEpBlackCol(game, -1);
        }

        if(player == PLAYER_WHITE)
        {
            updateEpWhiteCol(game, -1);
        }

        if(arrivePiece == EMPTY_CASE && startPiece != WHITE_PAWN && startPiece != BLACK_PAWN)
        {
            game->nbCoupInutile++;
        }
        else
            game->nbCoupInutile = 0;
        

    }

    return ;
}

//ici on joue le coup mais on update pas la glistof fen, on actualise bien le nbr de Zobrist 
//On ne vérifie pas non plus si on passe en endgame
//on ne vérifie pas non plus si le coup est valide 
void PlayMoveAI(ChessGame *game, Move *move)
{
    unsigned long long newZobristKey = getUpdZobristKey(game, move);
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);   
    end_col = getEndColFromMove(move);
    Player player = GetCurrentPlayer(game);
    Piece arrivePiece = GetPieceOnCase(game, end_row, end_col);
       
    Piece startPiece = game->board[start_row][start_col];
   
    
        
    MoveType moveType = getMoveTypeFromMove(move);

    if(moveType == Classique || moveType == Prise)
        PlayClassiqueMove(game, move);
    else if(moveType == Promotion)
        PlayPromotion(game, move, getResponseFromMove(move));
    else if(moveType == Roque)
        PlayRoqueMove(game, move);
    else if(moveType == PriseEnPassant)
        PlayPriseEnPassantMove(game, move);    


    if(player == PLAYER_WHITE && startPiece == WHITE_PAWN && start_row == 6 && end_row == 4)
    {
        updateEpBlackCol(game, start_col);
    }
    else if(player == PLAYER_BLACK && startPiece == BLACK_PAWN && start_row == 1 && end_row == 3)
    {
        updateEpWhiteCol(game, start_col);
    }
    if(startPiece == BLACK_KING)
    {
        game->BK = false;
        game->BQ = false;
     
    }        
    else if(startPiece == WHITE_KING)
    {
        game->WK = false;
        game->WQ = false;
        
    }
    else if(startPiece == BLACK_ROOK && start_col ==0)
    {
        game->BQ = false;
    }

    else if(startPiece == BLACK_ROOK && start_col == 7)
    {
        game->BK = false;
    }        
    else if(startPiece == WHITE_ROOK && start_col ==0)
    {
        game->WQ = false;
    }
    else if(startPiece == WHITE_ROOK && start_col ==7)
    {
        game->WK = false;
    }

    if(player == PLAYER_BLACK)
    {
        updateEpBlackCol(game, -1);
    }

    if(player == PLAYER_WHITE)
    {
        updateEpWhiteCol(game, -1);
    }

    if(arrivePiece == EMPTY_CASE && startPiece != WHITE_PAWN && startPiece != BLACK_PAWN)
    {
        game->nbCoupInutile++;
    }
    else
        game->nbCoupInutile = 0;
    

    
    updateCurrentZobritstKey(game, newZobristKey);
    updateGlistOfZobrist(game, newZobristKey);   
    
}




void UndoMove(ChessGame *game)
{

  
    

    GList *temp = g_list_last(game->fenList);
    char *fen;

    if (temp != NULL && temp->prev != NULL) {
        fen = (char *)temp->prev->data;
    } else 
    {   
        fprintf(stderr, "Error in UndoMove\n");
        return ;
    }

    game->epWhiteCol = -1;
    game->epBlackCol = -1;
    game->WK = false;
    game->WQ = false;
    game->BK = false;
    game->BQ = false;

    game->nbCoupInutile = 0;
    
    int length = strlen(fen);
    int row, col, i;
    for(row = 0; row<BOARD_SIZE; row++)
    {
        for(col = 0; col<BOARD_SIZE; col++)
        {
            game->board[row][col] = EMPTY_CASE;
        }
    }
    row = 0;
    col = 0;

    int nbSpace = 0;
    

    

    for(i = 0; i < length; i++)
    {
        
        char c = fen[i];
        

        if(c == ' ')
        {
            nbSpace++;
            continue;
        }
        if(nbSpace == 0)
        {
            
            
            if(c == '/' || col > 7)
            {
                row++;
                col = 0;
                continue;
            }
            else if(isdigit(c))
            {
                int num = c - '0';
                
                col += num;
                continue;
            }
            else if(isPiece(c))
            {
                Piece piece = pieceFromChar(c);
            
                game->board[row][col] = piece;
                
            }
            col++;

            

        }
        
        else if(nbSpace == 1)
        {
           

            if(c == 'w')
                game->current_player = PLAYER_WHITE;
            else if(c == 'b')
                game->current_player = PLAYER_BLACK;
            
           
        }
        else if(nbSpace == 2)
        {

            if(c == '-')
                continue;
            else if(c == 'K')
            {
    
                game->WK = true;
            }
            else if(c == 'Q')
            {
                game->WQ = true;
            }
            else if(c == 'k')
            {
                game->BK = true;
            }
            else if(c == 'q')
            {
                game->BQ = true;
            }
        }
        else if(nbSpace == 3)
        {   
            if(c == '-')
            {
                continue;
            }
            else if(!isdigit(c))
            {
                continue;
            }
            else
            {
                char prev = fen[i-1];

                if(c == '6')
                {
                    game->epWhiteCol = getColumnIndex(prev);
                }
                else if(c == '3')
                {
                    game->epBlackCol = getColumnIndex(prev);
                }
            }

        }
        else if(nbSpace == 4)
        {
            int size = 0;
            for(int j = i; ;j++)
            {
                char l = fen[j];
                if( !isdigit(l) || l == ' ' ||l == '\0')
                {
                    break;
                }
                else
                    size++;


            }
            char *temp = malloc(sizeof(char) * (size + 1));
            int m = i;
            for(int k = 0; k < size; k++)
            {
                temp[k] = fen[m];
                m++;
            }
            temp[size] = '\0';
            game->nbCoupInutile = atoi(temp);

            free(temp);

            break;
        }


        

        

    }

    
    GList *lastElement = g_list_last(game->fenList);
    free(lastElement->data);
    
    game->fenList = g_list_remove_link(game->fenList, lastElement);

    

    g_list_free(lastElement);

    GList *lastZobrist = g_list_last(game->zobristList);
    game->currentZobristKey = *(unsigned long long*)lastZobrist->prev->data;
    g_list_remove_link(game->zobristList, lastZobrist);

}


void UndoMove2(ChessGame *game)
{
    GList *temp = g_list_last(game->fenList);
    GList *temp2;
    char *fen;

    if (temp != NULL && temp->prev != NULL) {
        temp2 = temp->prev;
    } else 
    {   
        fprintf(stderr, "Error in UndoMove\n");
        return ;
    }

    if(temp2 != NULL && temp2->prev != NULL)
    {
        fen = (char*)temp2->prev->data;
    }
    else
    {
        fprintf(stderr, "Error 2 in UndoMove\n");
        return ;

    }

     game->epWhiteCol = -1;
    game->epBlackCol = -1;
    game->WK = false;
    game->WQ = false;
    game->BK = false;
    game->BQ = false;

    game->nbCoupInutile = 0;
    
    int length = strlen(fen);
    int row, col, i;
    for(row = 0; row<BOARD_SIZE; row++)
    {
        for(col = 0; col<BOARD_SIZE; col++)
        {
            game->board[row][col] = EMPTY_CASE;
        }
    }
    row = 0;
    col = 0;

    int nbSpace = 0;
    

    

    for(i = 0; i < length; i++)
    {
        
        char c = fen[i];
        

        if(c == ' ')
        {
            nbSpace++;
            continue;
        }
        if(nbSpace == 0)
        {
            
            
            if(c == '/' || col > 7)
            {
                row++;
                col = 0;
                continue;
            }
            else if(isdigit(c))
            {
                int num = c - '0';
                
                col += num;
                continue;
            }
            else if(isPiece(c))
            {
                Piece piece = pieceFromChar(c);
            
                game->board[row][col] = piece;
                
            }
            col++;

            

        }
        
        else if(nbSpace == 1)
        {
           

            if(c == 'w')
                game->current_player = PLAYER_WHITE;
            else if(c == 'b')
                game->current_player = PLAYER_BLACK;
            
           
        }
        else if(nbSpace == 2)
        {

            if(c == '-')
                continue;
            else if(c == 'K')
            {
    
                game->WK = true;
            }
            else if(c == 'Q')
            {
                game->WQ = true;
            }
            else if(c == 'k')
            {
                game->BK = true;
            }
            else if(c == 'q')
            {
                game->BQ = true;
            }
        }
        else if(nbSpace == 3)
        {   
            if(c == '-')
            {
                continue;
            }
            else if(!isdigit(c))
            {
                continue;
            }
            else
            {
                char prev = fen[i-1];

                if(c == '6')
                {
                    game->epWhiteCol = getColumnIndex(prev);
                }
                else if(c == '3')
                {
                    game->epBlackCol = getColumnIndex(prev);
                }
            }

        }
        else if(nbSpace == 4)
        {
            int size = 0;
            for(int j = i; ;j++)
            {
                char l = fen[j];
                if( !isdigit(l) || l == ' ' ||l == '\0')
                {
                    break;
                }
                else
                    size++;


            }
            char *temp = malloc(sizeof(char) * (size+1));
            int m = i;
            for(int k = 0; k < size; k++)
            {
                temp[k] = fen[m];
                m++;
            }
            temp[size] = '\0';
            game->nbCoupInutile = atoi(temp);

            free(temp);

            break;
        }


        

        

    }

    
    GList *lastElement = g_list_last(game->fenList);
    free(lastElement->data);
    
    game->fenList = g_list_remove_link(game->fenList, lastElement);

    GList *lastElement2 = g_list_last(game->fenList);
    free(lastElement2->data);
    game->fenList = g_list_remove_link(game->fenList, lastElement2);

    g_list_free(lastElement);
    g_list_free(lastElement2);

    GList *lastZobrist = g_list_last(game->zobristList);
    GList *lastZobrist2 = lastZobrist->prev;
    game->currentZobristKey = *(unsigned long long *)lastZobrist2->prev->data;

    game->zobristList = g_list_remove_link(game->zobristList, lastZobrist);
    game->zobristList = g_list_remove_link(game->zobristList, lastZobrist2);

}

void PlayClassiqueMove(ChessGame *game, Move *move)
{
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);

    Piece start_piece = game->board[start_row][start_col];
    
    
    
    
    
    
    game->board[end_row][end_col] = start_piece;
    game->board[start_row][start_col] = EMPTY_CASE;

    game->current_player = GetNextPlayer(game);

    return ;

}

bool GameIsValidMove(ChessGame *game, Move *move) 
{
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);

    Player currentPlayer = game->current_player;

    // fprintf(stderr, "end row = %d ; end col = %d \n", end_row, end_col);

    // fprintf(stderr, "10\n");
    Piece start_piece = game->board[start_row][start_col];
    // fprintf(stderr, "11\n");



    if(start_piece == EMPTY_CASE)
        return false;
    if(start_row < 0 || start_row >= BOARD_SIZE || start_col < 0 || start_col >= BOARD_SIZE || end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col>= BOARD_SIZE)
        return false;
    if(start_col == end_col && start_row == end_row)
        return false;    

    if(start_piece > 0 && currentPlayer == PLAYER_BLACK){
        printf("Ce n'est pas votre tour, c'est aux noirs 1\n");
        return false;
    }
    
    if(start_piece < 0 && currentPlayer == PLAYER_WHITE){
        printf("Ce n'est pas votre tour, c'est aux blancs 1\n");
        return false;
    }
    if(currentPlayer != player)
        return false;
    
    bool res = false;

    

    if(start_piece == WHITE_PAWN || start_piece == BLACK_PAWN)
    {
        // fprintf(stderr, "100\n");
        res = GameIsPawnMoveValid(game, move);
        // fprintf(stderr, "101\n");
        
    }
    else if(start_piece == WHITE_ROOK || start_piece == BLACK_ROOK)
    {
        res = GameIsROOKMoveValid(game, move);
        
    }     
    else if(start_piece == WHITE_BISHOP || start_piece == BLACK_BISHOP)
    {
        res = GameIsBishopMoveValid(game, move);
    }
    else if(start_piece == WHITE_KNIGHT || start_piece == BLACK_KNIGHT)
    {
        res = GameIsKnightMoveValid(game, move);
    }
    else if(start_piece == WHITE_QUEEN || start_piece == BLACK_QUEEN)
    {
        
        res = GameIsQueenValidMove(game, move);
    }
    else if(start_piece == WHITE_KING || start_piece == BLACK_KING)
    {
        res = GameIsKingMoveValid(game, move);
    }

    if(res == false)
        return false;

    // PrintMove(move);
    
    if(IsCheckMove(game, move))
        return false;
    
    return true;
}
bool GameIsQueenValidMove(ChessGame *game,  Move *move)
{
   
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    

    Piece start_piece = game->board[start_row][start_col];
    if(end_row > 7 || end_row < 0 || end_col > 7 || end_col < 0)
        return false;
    
    if(start_piece != WHITE_QUEEN && start_piece != BLACK_QUEEN)
        return false;

    int delta_x = abs(start_row - end_row);
    int delta_y = abs(start_col -end_col);

    if(delta_x == delta_y)
    {
        bool res = GameIsBishopMoveValid(game, move);
        return res;
    }
    bool res = GameIsROOKMoveValid(game, move);
        return res;

}

bool GameIsBishopMoveValid(ChessGame *game, Move *move)
{
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);

    if(end_row > 7 || end_row < 0 || end_col > 7 || end_col < 0)
        return false;
    Piece start_piece = game->board[start_row][start_col];
    int i, j;

    if(start_piece != WHITE_BISHOP && start_piece != BLACK_BISHOP && start_piece != WHITE_QUEEN && start_piece != BLACK_QUEEN)
        return false;
    int delta_x = abs(start_row - end_row);
    int delta_y = abs(start_col - end_col);

    if(delta_x != delta_y)
        return false;

    if(start_row < end_row && start_col < end_col){              // x augmente et y augmente
        for( i = start_row +1, j=start_col+1; i<end_row; i++, j++){
            if(game->board[i][j] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
    }

    if(start_row < end_row && start_col > end_col){           //x augmente et y diminue
        for( i = start_row +1, j=start_col-1; i<end_row; i++, j--){
            if(game->board[i][j] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
    }

    if(start_row > end_row && start_col < end_col){       // x diminue et y augmente 
        for( i = start_row -1, j=start_col+1; i>end_row; i--, j++){
            if(game->board[i][j] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(move->player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(move->player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
    }

    if(start_row > end_row && start_col > end_col){   //x et y diminuent 
        for( i = start_row -1, j=start_col-1; i>end_row; i--, j--){
            if(game->board[i][j] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(move->player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(move->player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
    }
    return false;
   
}

bool GameIsPawnMoveValid(ChessGame *game, Move *move)
{
    // fprintf(stderr, " A GameIsPawnMoveValid \n");
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    

    Piece start_piece = game->board[start_row][start_col];
    if(end_row > 7 || end_row < 0 || end_col > 7 || end_col < 0)
        return false;
    if(start_piece != WHITE_PAWN && start_piece != BLACK_PAWN)
        return false;



    if(start_piece == WHITE_PAWN)           //White pawn
    {
        int end_row_theorique = start_row - 1;
        // fprintf(stderr, " A.5 GameIsPawnMoveValid \n");
        // Piece arrive = game->board[end_row][end_col]; //seg fault ici depuis GameIsPat
        // fprintf(stderr, "end_row = %d; end_col = %d\n", end_row, end_col);
        Piece arrive = GetPieceOnCase(game, end_row, end_col);
        // fprintf(stderr, " B GameIsPawnMoveValid \n");
        if(end_row_theorique == end_row && start_col == end_col && arrive == EMPTY_CASE)
            return true;
        // fprintf(stderr, "C GameIsPawnMoveValid \n");

        if(start_row == 6)
        {
            end_row_theorique = 4;
            arrive = game->board[end_row][end_col];

            if(end_row_theorique == end_row && start_col == end_col && arrive==EMPTY_CASE)
            {
                if(GetPieceOnCase(game, 5, start_col) == EMPTY_CASE)
                    return true;
            }
        }
        // fprintf(stderr, " D GameIsPawnMoveValid \n");
        

        int end_col_theorique = start_col + 1;
        end_row_theorique = start_row-1;
        arrive = game->board[end_row][end_col];
        
        if(end_col_theorique == end_col && end_row_theorique == end_row && arrive < 0)
            return true;
        
        end_col_theorique = start_col - 1;        
        if(end_col_theorique == end_col && end_row_theorique == end_row && arrive < 0)
            return true;

        return false;
        
    }

    if(start_piece == BLACK_PAWN) //Black pawn
    {
        int end_row_theorique = start_row + 1;
        Piece arrive = game->board[end_row][end_col];
        if(end_row_theorique == end_row && start_col == end_col && arrive == EMPTY_CASE)
            return true;
        
        if(start_row == 1)
        {
            end_row_theorique = 3;
            arrive = game->board[end_row][end_col];
            if(end_row_theorique == end_row && start_col == end_col && arrive == EMPTY_CASE)
            {
                if(GetPieceOnCase(game, 2, start_col) == EMPTY_CASE)
                    return true;
            }
        }

        int end_col_theorique = start_col +1;
        end_row_theorique = start_row +1;
        arrive = game->board[end_row][end_col];

        if(end_col_theorique == end_col && end_row_theorique == end_row && arrive > 0)
            return true;

        end_col_theorique = start_col-1;
        if(end_col_theorique == end_col && end_row_theorique == end_row && arrive > 0)
            return true;

        return false;

    }
    return false;
}

bool GameIsROOKMoveValid(ChessGame *game, Move *move)
{
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);

    if(end_row > 7 || end_row < 0 || end_col > 7 || end_col < 0)
        return false;
    Piece start_piece = game->board[start_row][start_col];
    if(start_piece != WHITE_ROOK && start_piece != BLACK_ROOK && start_piece != WHITE_QUEEN && start_piece != BLACK_QUEEN)
        return false;

    
    if(start_col != end_col && start_row != end_row)
        return false;
    if(start_col < end_col)
    {
        int i;
        for(i = start_col +1; i< end_col; i++){
            if(game->board[start_row][i] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
        
    }
    
    if(start_col > end_col)
    {
        int i;
        for(i = start_col -1; i> end_col; i--){
            if(game->board[start_row][i] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
    }

    if(start_row < end_row)
    {
        int i;
        for(i = start_row +1; i<end_row; i++)
        {
            if(game->board[i][start_col] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
    }
    
    if(start_row > end_row)
    {
        int i;
        for(i = start_row -1; i>end_row; i--)
        {
            if(game->board[i][start_col] != EMPTY_CASE)
                return false;
        }
        Piece end_case = game->board[end_row][end_col];
        if(player == PLAYER_WHITE){
            if(end_case > 0)
                return false;

        }
        if(player == PLAYER_BLACK){
            if(end_case < 0)
                return false;
        }
        return true;
    }
    return false;
   
}

bool GameIsKnightMoveValid(ChessGame *game, Move *move)
{
    
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);

    if(end_row > 7 || end_row < 0 || end_col > 7 || end_col < 0)
        return false;

    Piece start_piece = game->board[start_row][start_col];
    if(start_piece != WHITE_KNIGHT && start_piece != BLACK_KNIGHT)
        return false;

    int delta_x, delta_y;
    delta_x = start_row - end_row;
    delta_y = start_col - end_col;

    
    int directions[8][2] = {{1, -2}, {1, 2}, {-1, -2}, {-1, 2}, {2, -1}, {2, 1}, {-2, -1}, {-2, 1}};


    for( int k = 0; k<8; k++){
        
        if(delta_x == directions[k][0] && delta_y == directions[k][1]){
            Piece end_case = game->board[end_row][end_col];
            if(player == PLAYER_WHITE){
                if(end_case > 0)
                    return false;

            }
            if(player == PLAYER_BLACK){
                if(end_case < 0)
                    return false;
            }
            return true;
        }
    }
    return false;


}

bool GameIsKingMoveValid(ChessGame *game, Move *move)
{
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);

    if(end_row > 7 || end_row < 0 || end_col > 7 || end_col < 0)
        return false;

    Piece start_piece = game->board[start_row][start_col];
    if(start_piece != WHITE_KING && start_piece != BLACK_KING)
        return false;

    if(start_row < 0 || start_row >= BOARD_SIZE || start_col < 0 || start_col >= BOARD_SIZE || end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col>= BOARD_SIZE)
        return false;


    int directions[8][2] = {{1, 0}, {-1, 0}, {0, +1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    int delta_x, delta_y;
    delta_x = start_row - end_row;
    delta_y = start_col - end_col;

    for(int k=0; k<8; k++){
        if(delta_x == directions[k][0] && delta_y == directions[k][1]){
            Piece end_case = game->board[end_row][end_col];
            if(player == PLAYER_WHITE){
                if(end_case > 0)
                    return false;

            }
            if(player == PLAYER_BLACK){
                if(end_case < 0)
                    return false;
            }
            return true;
        }
    }

  
    return false;

}

//aller du roi vers la tour, le fou, le cavalier, la dame et le pion
bool GameIsCheck(ChessGame *game, Player player) 
{
    int i, j;
    int row = -1, col = -1; // Initialisation des coordonnées du roi

    Player attacker;
    if (player == PLAYER_WHITE)
    {
        attacker = PLAYER_BLACK;
        // row = game->wkRow;
        // col = game->wkCol;
    }
    else if (player == PLAYER_BLACK)
    {
        attacker = PLAYER_WHITE;
        // row = game->bkRow;
        // col = game->bkCol;
    }
    else 
    {
        printf("Impossible de vérifier si un joueur vide est en échec.\n");
        return false;
    }

    // if(player == PLAYER_WHITE && game->WK == true && game->WQ == true)
    // {
    //     row = 7;
    //     col = 4;

    //     Piece temp = GetPieceOnCase(game, row, col);
    //     if(temp != WHITE_KING)
    //     {
    //         fprintf(stderr, "EROOOOOOOOOOOOOR°______________________________________\n");
    //         enumprintf(temp);
    //         fprintf(stderr, "\n");
    //         printFEN(createFEN(game));
    //     }
    // }
    // else if(player == PLAYER_BLACK && game->BK == true && game->BQ == true)
    // {
    //     row = 0;
    //     col = 4;
    // }
    // else 
    // {
    //     // Recherche des coordonnées du roi du joueur courant
        
    //     for (i = 0; i < 8; i++) 
    //     {
    //         for (j = 0; j < 8; j++) 
    //         {
    //             Piece temp = GetPieceOnCase(game, i, j);
    //             if (temp == WHITE_KING && player == PLAYER_WHITE) 
    //             {
    //                 row = i;
    //                 col = j;
    //                 break;
    //             }
    //             else if(temp == BLACK_KING && player == PLAYER_BLACK)
    //             {
    //                 row = i;
    //                 col = j;
    //                 break;
    //             }
    //         }

    //         if(row != -1 && col != -1)
    //             break;
    //     }

    // }
    
    for (i = 0; i < 8; i++) 
    {
        for (j = 0; j < 8; j++) 
        {
            Piece temp = GetPieceOnCase(game, i, j);
            if (temp == WHITE_KING && player == PLAYER_WHITE) 
            {
                row = i;
                col = j;
                break;
            }
            else if(temp == BLACK_KING && player == PLAYER_BLACK)
            {
                row = i;
                col = j;
                break;
            }
        }

        if(row != -1 && col != -1)
            break;
    }
    

    

    if(row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
    {
        fprintf(stderr, "ERROR !, didnt find the king positions in function GameIsCheck\n");
        fprintf(stderr, "row = %d, col = %d\n", row, col);
        char *fen = createFEN(game);
        fprintf(stderr, " FEN = %s !\n", fen);
        free(fen);
        return false; // ! ici on retourne false comme si tout allait bien mais on a pas vérifié s'il y avait échec.
    }

    // Vérification des mouvements possibles des pièces adverses

    for (i = 0; i < 8; i++) 
    {
        for (j = 0; j < 8; j++) 
        {
            if (GetCaseOwner(game, i, j) == attacker) 
            {
                Move *move = CreateMove(attacker, i, j, row, col);
                if (move == NULL) {
                    printf("Erreur lors de l'allocation mémoire pour Move.\n");
                    return false;
                }
                

                Piece piece_attack = GetPieceOnCase(game, i, j);
                switch (piece_attack) {
                    // Gérer les différents types de pièces et vérifier si elles peuvent attaquer le roi
                    case EMPTY_CASE:
                        break;
                    case WHITE_KING:
                    case BLACK_KING:
                        
                        if (GameIsKingMoveValid(game, move)) {
                            free(move);
                            return true;
                        }
                        break;
                    case WHITE_QUEEN:
                    case BLACK_QUEEN:
                        if (GameIsQueenValidMove(game,  move)) {
                            free(move);
                            return true;
                        }
                        break;
                    case WHITE_BISHOP:
                    case BLACK_BISHOP:
                        if (GameIsBishopMoveValid(game,  move)) {
                            free(move);
                            return true;
                        }
                        break;
                    case WHITE_KNIGHT:
                    case BLACK_KNIGHT:
                        if (GameIsKnightMoveValid(game,  move)) {
                            free(move);
                            return true;
                        }
                        break;
                    case WHITE_ROOK:
                    case BLACK_ROOK:
                        if (GameIsROOKMoveValid(game,  move)) {
                            free(move);
                            return true;
                        }
                        break;
                    case WHITE_PAWN:
                    case BLACK_PAWN:
                        if (GameIsPawnMoveValid(game, move)) {
                            free(move);
                            return true;
                        }
                        break;
                    default:
                        fprintf(stderr, "Pièce inconnue.\n");
                        fprintf(stderr, "Piece inconnue = %d\n", piece_attack);

                        break;
                }
                free(move);
            }
        }
    }

    return false;
}






bool RoqueIsValidMove(ChessGame *game, Move *move)
{
    
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);
    Piece start_piece = GetPieceOnCase(game, start_row, start_col);
   
    Player currentPlayer = game->current_player;

    if(GameIsCheck(game, player))
    {
        
        return false;
    }
    if(start_piece == EMPTY_CASE)
        return false;
    if(start_row < 0 || start_row >= BOARD_SIZE || start_col < 0 || start_col >= BOARD_SIZE || end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col>= BOARD_SIZE)
        return false;
    if(start_col == end_col && start_row == end_row)
        return false;    

    if(start_piece > 0 && currentPlayer == PLAYER_BLACK){
        printf("Ce n'est pas votre tour, c'est aux noirs 2\n");
        return false;
    }
    
    if(start_piece < 0 && currentPlayer == PLAYER_WHITE){
        printf("Ce n'est pas votre tour, c'est aux blancs 2\n");
        return false;
    }
    if(currentPlayer != player)
        return false;




    if(start_piece == WHITE_KING) //roque blanc 
    {
        
        if(end_row != 7)
        {
            return false;
        }
        if(end_col != 2 && end_col != 6)
        {
            return false;
        }


        if(end_col == 6) //petit roque blanc
        {
            if(game->WK == false)
            {
                return false;
            }

            
            if(GetPieceOnCase(game, 7, 5) != EMPTY_CASE || GetPieceOnCase(game, 7, 6)!= EMPTY_CASE || GetPieceOnCase(game, 7, 7) != WHITE_ROOK)
                return false;
                

            movePieceWithoutVerif(game, start_row, start_col, 7, 5, false);
            if(GameIsCheck(game, PLAYER_WHITE))
            {
                movePieceWithoutVerif(game, 7, 5, start_row, start_col, false);
                return false;
            }
            movePieceWithoutVerif(game, 7, 5, 7, 6, false);
            if(GameIsCheck(game, PLAYER_WHITE))
            {
                movePieceWithoutVerif(game, 7, 6, 7, 5, false);
                movePieceWithoutVerif(game, 7, 5, start_row, start_col, false);
                return false;
            }

            movePieceWithoutVerif(game, 7, 6, 7, 5, false);
            movePieceWithoutVerif(game, 7, 5, start_row, start_col, false);
            return true;

        }

        if(end_col == 2) //grand roque blanc
        {
            if(game->WQ == false)
            {
                return false;
            }

            if(GetPieceOnCase(game, 7, 3) != EMPTY_CASE || GetPieceOnCase(game, 7, 2)!= EMPTY_CASE || GetPieceOnCase(game, 7, 1) != EMPTY_CASE || GetPieceOnCase(game, 7, 0) != WHITE_ROOK)
                return false;

            movePieceWithoutVerif(game, start_row, start_col, 7, 3, false);
            if(GameIsCheck(game, PLAYER_WHITE))
            {
                movePieceWithoutVerif(game, 7, 3, start_row, start_col, false);
                return false;
            }
            movePieceWithoutVerif(game, 7, 3, 7, 2, false);
            if(GameIsCheck(game, PLAYER_WHITE))
            {
                movePieceWithoutVerif(game, 7, 2, 7, 3, false);
                movePieceWithoutVerif(game, 7, 3, start_row, start_col, false);
                return false;
            }

            movePieceWithoutVerif(game, 7, 2, 7, 3, false);
            movePieceWithoutVerif(game, 7, 3, start_row, start_col, false);

            return true;                
        
        }



    }

    if(start_piece == BLACK_KING) //roque noir
    {
        
        if(end_row != 0)
        {
            
            return false;
        }
        if(end_col != 2 && end_col != 6)
        {
            
            return false;
        }


        if(end_col == 6) //petit roque noir
        {
            if(game->BK == false)
            {                
                return false;
            }

            
            if(GetPieceOnCase(game, 0, 5) != EMPTY_CASE || GetPieceOnCase(game, 0, 6)!= EMPTY_CASE || GetPieceOnCase(game, 0, 7) != BLACK_ROOK)
                return false;
                

            movePieceWithoutVerif(game, start_row, start_col, 0, 5, false);
            if(GameIsCheck(game, PLAYER_BLACK))
            {
                movePieceWithoutVerif(game, 0, 5, start_row, start_col, false);

                return false;
            }
            movePieceWithoutVerif(game, 0, 5, 0, 6, false);
            if(GameIsCheck(game, PLAYER_BLACK))
            {
                movePieceWithoutVerif(game, 0, 6, 0, 5, false);
                movePieceWithoutVerif(game, 0, 5, start_row, start_col, false);

                return false;
            }

            movePieceWithoutVerif(game, 0, 6, 0, 5, false);
            movePieceWithoutVerif(game, 0, 5, start_row, start_col, false);



            return true;

        }

        if(end_col == 2) //grand roque noir
        {
            if(game->BQ == false)
            {
                return false;
            }

            if(GetPieceOnCase(game, 0, 3) != EMPTY_CASE || GetPieceOnCase(game, 0, 2)!= EMPTY_CASE || GetPieceOnCase(game, 0, 1) != EMPTY_CASE || GetPieceOnCase(game, 0, 0) != BLACK_ROOK)
                return false;

            movePieceWithoutVerif(game, start_row, start_col, 0, 3, false);
            if(GameIsCheck(game, PLAYER_BLACK))
            {
                movePieceWithoutVerif(game, 0, 3, start_row, start_col, false);

                return false;
            }
            movePieceWithoutVerif(game, 0, 3, 0, 2, false);
            if(GameIsCheck(game, PLAYER_BLACK))
            {
                movePieceWithoutVerif(game, 0, 2, 0, 3, false);
                movePieceWithoutVerif(game, 0, 3, start_row, start_col, false);

                return false;
            }

            movePieceWithoutVerif(game, 0, 2, 0, 3, false);
            movePieceWithoutVerif(game, 0, 3, start_row, start_col, false);

            return true;                
        
        }



    

    }
    return false;

}



void movePieceWithoutVerif(ChessGame *game, int srcRow, int srcCol, int destRow, int destCol, bool updGListOfFen) 
{
    // Vérifier si les coordonnées source et destination sont valides
    if (srcRow < 0 || srcRow >= BOARD_SIZE || srcCol < 0 || srcCol >= BOARD_SIZE ||
        destRow < 0 || destRow >= BOARD_SIZE || destCol < 0 || destCol >= BOARD_SIZE) {
        printf("Coordonnées invalides.\n");
        return;
    }

    // Récupérer la pièce à déplacer
    Piece pieceToMove = game->board[srcRow][srcCol];

    // Déplacer la pièce vers la destination
    game->board[destRow][destCol] = pieceToMove;
    game->board[srcRow][srcCol] = EMPTY_CASE;

    if(updGListOfFen)
        updateGListOfFen(game);


}

void PlayRoqueMove(ChessGame *game, Move *move)
{
    
    int start_row, start_col;
    int end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    
    end_col = getEndColFromMove(move);
   

    Piece start_piece = GetPieceOnCase(game, start_row, start_col);

    if(start_piece == WHITE_KING && end_col == 6) //petit roque blanc
    {
        game->board[7][6] = WHITE_KING;
        game->board[7][5] = WHITE_ROOK;
        game->board[7][4] = EMPTY_CASE;
        game->board[7][7] = EMPTY_CASE;
       
    }

    if(start_piece == WHITE_KING && end_col == 2) //grand roque blanc
    {
        game->board[7][2] = WHITE_KING;
        game->board[7][3] = WHITE_ROOK;
        game->board[7][0] = EMPTY_CASE;
        game->board[7][4] = EMPTY_CASE;


    }

    if(start_piece == BLACK_KING && end_col == 6) //petit roque noir
    {
        game->board[0][6] = BLACK_KING;
        game->board[0][5] = BLACK_ROOK;
        game->board[0][4] = EMPTY_CASE;
        game->board[0][7] = EMPTY_CASE;

     
    }

    if(start_piece == BLACK_KING && end_col == 2) //grand roque noir
    {
        game->board[0][2] = BLACK_KING;
        game->board[0][3] = BLACK_ROOK;
        game->board[0][0] = EMPTY_CASE;
        game->board[0][4] = EMPTY_CASE;
    }
    
    game->current_player = GetNextPlayer(game);

    return ;
}

bool PriseEnPassantValidMove(ChessGame *game, Move *move)
{
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);
    Player current_player = game->current_player;
    Piece start_piece = game->board[start_row][start_col];

    if(start_piece != WHITE_PAWN && start_piece != BLACK_PAWN)
    {
        return false;
    }
    if(start_piece == EMPTY_CASE)
        return false;
    if(start_row < 0 || start_row >= BOARD_SIZE || start_col < 0 || start_col >= BOARD_SIZE || end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col>= BOARD_SIZE)
        return false;
    if(start_col == end_col && start_row == end_row)
        return false;    

    if(start_piece > 0 && current_player == PLAYER_BLACK){
        return false;
    }
    
    if(start_piece < 0 && current_player == PLAYER_WHITE){
        return false;
    }
    if(current_player != player)
        return false;


    if(current_player == PLAYER_WHITE)
    {
        if(end_col != game->epWhiteCol)
            return false;
    }

    if(current_player == PLAYER_BLACK)
    {
        if(end_col != game->epBlackCol)
            return false;
    }
   


    if(current_player == PLAYER_WHITE)
    {
        if(start_row != 3)
        {
            return false;
        }

        Piece piecevoisine = GetPieceOnCase(game, start_row, end_col);

        if(piecevoisine != BLACK_PAWN)
        {
            return false;
        }

    }
    else if(current_player == PLAYER_BLACK)
    {
        if(start_row != 4)
        {
            return false;
        }
        Piece piecevoisine = GetPieceOnCase(game, start_row, end_col);

        if(piecevoisine != WHITE_PAWN)
        {
            return false;
        } 

    }

    


    ChessGame *copy = copyChessGame(game);
    PlayPriseEnPassantMove(copy, move);

    if(GameIsCheck(copy, current_player))
    {
        ChessFree(copy);
        return false;
    }
    else
    {
        ChessFree(copy);
        return true;
    }
    ChessFree(copy);

    return false;
}

void PlayPriseEnPassantMove(ChessGame *game, Move *move)
{
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
   

    Piece start_piece = game->board[start_row][start_col];
    

    game->board[start_row][start_col] = EMPTY_CASE;
    game->board[start_row][end_col]= EMPTY_CASE;
    game->board[end_row][end_col]=start_piece;

    game->current_player = GetNextPlayer(game);

    return ;
}

void PlayPromotion(ChessGame *game, Move *move, int response)
{
    
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);
    Player currentPlayer = game->current_player;

    if(player != currentPlayer)
    {
        printf("Ce n'est pas votre tour(fonction PlayPromotion)\n");
        return ;
    }

    if(response != 1 && response !=2 && response != 3 && response != 4)
    {
        printf("Response n'est pas un chiffre valid\n");
        return;
    }

    if(player == PLAYER_WHITE)
    {
        if(end_row != 0)
        {
            printf("error\n");
            return;
        }
        switch(response)
        {
            case 1:
                game->board[end_row][end_col]= WHITE_QUEEN;
                break;
            case 2:
                game->board[end_row][end_col]= WHITE_ROOK;
                break;
            case 3:
                game->board[end_row][end_col]= WHITE_BISHOP;
                break;
            case 4:
                game->board[end_row][end_col]= WHITE_KNIGHT;
                break;
        }
    }

    if(player == PLAYER_BLACK)
    {
        if(end_row != 7)
        {
            printf("error\n");
            return;
        }
        switch(response)
        {
            case 1:
                game->board[end_row][end_col]= BLACK_QUEEN;
                break;
            case 2:
                game->board[end_row][end_col]= BLACK_ROOK;
                break;
            case 3:
                game->board[end_row][end_col]= BLACK_BISHOP;
                break;
            case 4:
                game->board[end_row][end_col]= BLACK_KNIGHT;
                break;
        }
    }

    game->board[start_row][start_col] = EMPTY_CASE;
    
    game->current_player = GetNextPlayer(game);

    return;
    


}




bool ChessGameIsMate(ChessGame *game, bool printWhy)
{
    // fprintf(stderr, "i\n");
    Player current_player = GetCurrentPlayer(game);
    // fprintf(stderr, "f\n");
    if(GameIsCheck(game, current_player)) // Il y a échec, il faut vérifier si il n'y a pas échec et math
    {
        // fprintf(stderr, "j1\n");
        

        
        int kingMoves[8][2] = 
        {
            {-1, -1}, // Haut gauche
            {-1, 0},  // Haut
            {-1, 1},  // Haut droite
            {0, -1},  // Gauche
            {0, 1},   // Droite
            {1, -1},  // Bas gauche
            {1, 0},   // Bas
            {1, 1}    // Bas droite
        };

        int i, j, start_row, start_col;
        start_row = -1;
        start_col = -1;

        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++) 
            {
                if (game->board[i][j] == WHITE_KING || game->board[i][j] == BLACK_KING) 
                {
                    if (current_player == GetCaseOwner(game, i, j)) 
                    {
                        start_row = i;
                        start_col = j;
                        break;
                    }
                }
            }
            
        }
        if (start_row < 0 || start_col < 0 || start_row >= BOARD_SIZE || start_col >= BOARD_SIZE)
        {
            fprintf(stderr, "Impossible de trouver les coordonées du roi\n");
            fprintf(stderr, "row = %d, col = %d\n", start_row, start_col);
            
            return false;
        }

        bool casedispo = false;

        // fprintf(stderr, "j2\n");
     

        for (i = 0; i < 8; i++) 
        {
            ChessGame *gameCopy = copyChessGame(game);
            int dest_row = start_row + kingMoves[i][0];
            int dest_col = start_col + kingMoves[i][1];
            // fprintf(stderr, "destROw = %d, destCol = %d\n", dest_row, dest_col);

            // fprintf(stderr, "k1\n");
            if(dest_row < 0 || dest_row >= BOARD_SIZE || dest_col < 0 || dest_col >= BOARD_SIZE)
            {
                ChessFree(gameCopy);
                continue;
            }
            Move *move = CreateMove(current_player, start_row, start_col, dest_row, dest_col);
            // fprintf(stderr, "k2\n");

            if (move == NULL) {
                
                fprintf(stderr, "Erreur lors de l'allocation mémoire pour le mouvement dans la fonction chessgameisover.\n");
                ChessFree(gameCopy);
                continue; 
            }

            
    


            if (GameIsKingMoveValid(gameCopy, move)) 
            {
                PlayMoveAI(gameCopy, move);
                if(!GameIsCheck(gameCopy, current_player))
                {
                    casedispo = true;

                    free(move);
                    ChessFree(gameCopy);
                    

                    break ;
                }
            }


            free(move);
            ChessFree(gameCopy);
        }

        // fprintf(stderr, "j3\n");

        

        if(! casedispo &&  !AutreCoupPossible(game, current_player))
        {
            
            if(printWhy)
            {
                printf("Les ");
                printPlayer(current_player);
                printf(" sont en échec et math\n");
            }
            return true;
        }
        // fprintf(stderr, "j4\n");
    }
    // fprintf(stderr, "j2\n");

    return false;
}

bool ChessGameIsOver(ChessGame *game, bool printWhy)
{
    if(ChessGameIsMate(game, printWhy) || ChessGameIsPat(game, printWhy) || ChessGameNoMatMin(game, printWhy) || ChessGameRepetition(game, printWhy))
    {
        return true;
    }
    if(game->nbCoupInutile >= 100)
    {
        if(printWhy)
            fprintf(stderr, "Match Nulle par la règle des 50 coups\n");
        
        return true;
    }

    return false;

}




//teste tous les coups possible, pour chaque coup regarder si ca ne bloque pas l'échec (en mettant une pièce sur la trajectoire)
bool AutreCoupPossible(ChessGame *game, Player player)
{
    
    if(player == PLAYER_WHITE) //quand les blancs sont en échecs, y'a t'il des coups pour les sauver ?
    {
        int i, j;

        for(i = 0; i<BOARD_SIZE; i++)
        {
            for(j=0; j<BOARD_SIZE; j++)
            {
                Piece current_piece = GetPieceOnCase(game, i, j);
                if(current_piece > 0) //il y a une pièce sur cette case, vériifier tous les coups possibles de cette pièce et tester si les blancs sont encore en échec 
                {
                    switch(current_piece)
                    {
                        case EMPTY_CASE:
                        case BLACK_QUEEN:
                        case BLACK_KING:
                        case BLACK_BISHOP:
                        case BLACK_KNIGHT:
                        case BLACK_ROOK:
                        case BLACK_PAWN:    
                        case WHITE_KING:                    
                        {
                            break;
                        }
                        case WHITE_QUEEN:
                        {
                            int k, l;
                            for(k=0; k<BOARD_SIZE; k++)
                            {
                                for(l=0; l<BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if(!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move(fontion : AutreCoupPossible\n");
                                        return NULL;
                                    }

                                    
                                    if(GameIsQueenValidMove(game, move)) // la dame peut jouer un coup, joué le coup dans une copie et verifier si les blancs sont encore en échecs 
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if(! GameIsCheck(gameCopy, PLAYER_WHITE)) // un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }

                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }
                            break;
                        }

                        case WHITE_ROOK: // Implémente ici
                        {
                            int k, l;
                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                        return NULL;
                                    }

                                    

                                    if (GameIsROOKMoveValid(game,move)) // La tour peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }
                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }

                            break ;
                        }

                        case WHITE_BISHOP: // Implémente ici
                        {
                            int k, l;
                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                        return NULL;
                                    }

                                   

                                    if (GameIsBishopMoveValid(game, move)) // Le fou peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }
                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }
                        
                        case WHITE_KNIGHT: // Implémente ici
                        {
                            int k, l;
                            int knightMoves[8][2] = { {-2, -1}, {-1, -2}, {1, -2}, {2, -1}, {-2, 1}, {-1, 2}, {1, 2}, {2, 1} };

                            for (int m = 0; m < 8; m++)
                            {
                                k = i + knightMoves[m][0];
                                l = j + knightMoves[m][1];
                                
                                if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                        return NULL;
                                    }

                                    

                                    if (GameIsKnightMoveValid(game,move)) // Le cavalier peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }
                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }

                        case WHITE_PAWN: // Implémente ici
                        {
                            int k, l;
                            
                            if(game->epWhiteCol != -1) //les blancs peuvent prendre en passant => vérifier si ca annule l'échec
                            {
                                int pawnMoves[2][2] = { {-1, -1}, {-1, 1} }; // Mouvements diagonaux pour la prise en passant

                                // Vérification des coups diagonaux pour la prise en passant
                                for (int m = 0; m < 2; m++)
                                {
                                    k = i + pawnMoves[m][0];
                                    l = j + pawnMoves[m][1];

                                    if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                    {
                                        Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                        if (!move)
                                        {
                                            printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                            return NULL;
                                        }

                                        

                                        if (PriseEnPassantValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                                        {
                                            ChessGame *gameCopy = copyChessGame(game);
                                            PlayPriseEnPassantMove(gameCopy, move);

                                            if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                            {
                                                ChessFree(gameCopy);
                                                free(move);
                                                return true;
                                            }

                                            ChessFree(gameCopy);
                                        }
                                        free(move);
                                    }
                                }
                            }
                            // Vérification du coup avancé
                            k = i - 1;
                            l = j;
                           
                            Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                            if (!move)
                            {
                                printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                return NULL;
                            }

                            

                            if (GameIsPawnMoveValid(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                {
                                    free(move);
                                    ChessFree(gameCopy);
                                    return true;
                                }

                                ChessFree(gameCopy);
                            }
                            
                            k = i-2;

                            newEndRow(move, k);

                            if (GameIsPawnMoveValid(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                {
                                    free(move);
                                    ChessFree(gameCopy);
                                    return true;
                                }
                                ChessFree(gameCopy);
                            }

                            k=i-1;
                            l=j-1;

                            newEndRow(move, k);
                            newEndCol(move, l);

                            if (GameIsPawnMoveValid(game,  move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                {
                                    
                                    free(move);
                                    ChessFree(gameCopy);
                                    return true;
                                }
                                ChessFree(gameCopy);
                            }

                            l=j+1;

                            newEndCol(move, l);

                            if (GameIsPawnMoveValid(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_WHITE)) // Un autre coup est possible si on entre dans le if
                                {
                                    free(move);
                                    ChessFree(gameCopy);
                                    return true;
                                }
                                ChessFree(gameCopy);
                            }

                            free(move);

                            

                            break;
                        }
                                                
                    }
                }
            }
        }

        return false;
    }

    if(player == PLAYER_BLACK)
    {
        int i, j;

        for(i = 0; i<BOARD_SIZE; i++)
        {
            for(j=0; j<BOARD_SIZE; j++)
            {
                Piece current_piece = GetPieceOnCase(game, i, j);
                if(current_piece < 0) //il y a une pièce sur cette case, vérifier tous les coups possibles de cette pièce et tester si les noirs sont encore en échec 
                {
                    switch(current_piece)
                    {
                        case EMPTY_CASE:
                        case WHITE_QUEEN:
                        case WHITE_KING:
                        case WHITE_BISHOP:
                        case WHITE_KNIGHT:
                        case WHITE_ROOK:
                        case WHITE_PAWN: 
                        case BLACK_KING:                       
                        {
                            break;
                        }
                        case BLACK_QUEEN:
                        {
                            int k, l;
                            for(k=0; k<BOARD_SIZE; k++)
                            {
                                for(l=0; l<BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if(!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                        return false;
                                    }

                                    

                                    if(GameIsQueenValidMove(game, move)) // la dame peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs 
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if(!GameIsCheck(gameCopy, PLAYER_BLACK)) // un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }
                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }
                            break;
                        }

                        case BLACK_ROOK: // Implémente ici
                        {
                            int k, l;
                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                        return false;
                                    }

                                    

                                    if (GameIsROOKMoveValid(game,  move)) // La tour peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }
                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }
                            break;
                        }

                        case BLACK_BISHOP: // Implémente ici
                        {
                            int k, l;
                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                        return false;
                                    }

                                    
                                    if (GameIsBishopMoveValid(game, move)) // Le fou peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }

                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }
                            break;
                        }

                        case BLACK_KNIGHT: // Implémente ici
                        {
                            int k, l;
                            int knightMoves[8][2] = { {-2, -1}, {-1, -2}, {1, -2}, {2, -1}, {-2, 1}, {-1, 2}, {1, 2}, {2, 1} };

                            for (int m = 0; m < 8; m++)
                            {
                                k = i + knightMoves[m][0];
                                l = j + knightMoves[m][1];
                                
                                if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                        return false;
                                    }

                                    

                                    if (GameIsKnightMoveValid(game, move)) // Le cavalier peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                        if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                        {
                                            ChessFree(gameCopy);
                                            free(move);
                                            return true;
                                        }

                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }

                        case BLACK_PAWN: // Implémente ici
                        {
                            int k, l;

                            if (game->epBlackCol != -1) // les noirs peuvent prendre en passant => vérifier si ça annule l'échec
                            {
                                int pawnMoves[2][2] = { {1, -1}, {1, 1} }; // Mouvements diagonaux pour la prise en passant

                                // Vérification des coups diagonaux pour la prise en passant
                                for (int m = 0; m < 2; m++)
                                {
                                    k = i + pawnMoves[m][0];
                                    l = j + pawnMoves[m][1];

                                    if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                    {
                                        Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                        if (!move)
                                        {
                                            printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                            return NULL;
                                        }

                                        

                                        if (PriseEnPassantValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                                        {
                                            ChessGame *gameCopy = copyChessGame(game);
                                            PlayPriseEnPassantMove(gameCopy, move);

                                            if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                            {
                                                ChessFree(gameCopy);
                                                free(move);
                                                return true;
                                            }
                                            ChessFree(gameCopy);
                                        }

                                        free(move);
                                    }

                                }
                            }
                            // Vérification du coup avancé
                            k = i + 1;
                            l = j;

                            Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                            if (!move)
                            {
                                printf("Erreur pour allouer la mémoire du move (fonction : AutreCoupPossible)\n");
                                return NULL;
                            }

                            

                            if (GameIsPawnMoveValid(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                {
                                    free(move);
                                    ChessFree(gameCopy);
                                    return true;
                                }
                                ChessFree(gameCopy);
                            }

                            k = i + 2;

                            newEndRow(move, k);

                            if (GameIsPawnMoveValid(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                {
                                    free(move);
                                    ChessFree(gameCopy);
                                    return true;
                                }
                                ChessFree(gameCopy);
                            }

                            k = i + 1;
                            l = j - 1;

                            newEndRow(move, k);
                            newEndCol(move, l);

                            if (GameIsPawnMoveValid(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                {
                                    free(move);
                                    ChessFree(gameCopy);
                                    return true;
                                }
                                ChessFree(gameCopy);
                            }

                            l = j + 1;

                            newEndCol(move, l);

                            if (GameIsPawnMoveValid(game,move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                ChessGame *gameCopy = copyChessGame(game);
                                movePieceWithoutVerif(gameCopy, i, j, k, l, false);

                                if (!GameIsCheck(gameCopy, PLAYER_BLACK)) // Un autre coup est possible si on entre dans le if
                                {
                                    free(move);

                                    ChessFree(gameCopy);
                                    return true;
                                }
                                ChessFree(gameCopy);
                            }

                            free(move);

                            break;
                        }

                    }
                }
            }
        }

        return false;
    }

    return false;
        
}

bool ChessGameIsPat(ChessGame *game, bool printWhy)
{
    Player current_player = game->current_player;

    int i, j;
    if(current_player == PLAYER_WHITE)
    {
        for(i = 0; i<BOARD_SIZE; i++)
        {
            for(j=0; j<BOARD_SIZE; j++)
            {
                Piece current_piece = GetPieceOnCase(game, i, j);

                if(current_piece > 0)
                {
                    switch(current_piece)
                    {
                        case EMPTY_CASE:
                        case BLACK_QUEEN:
                        case BLACK_KING:
                        case BLACK_BISHOP:
                        case BLACK_KNIGHT:
                        case BLACK_ROOK:
                        case BLACK_PAWN:                        
                        {
                            break;
                        }
                        case WHITE_QUEEN:
                        {
                            int k, l;
                            // fprintf(stderr, "WHITE QUEEN \n");
                            for(k=0; k<BOARD_SIZE; k++)
                            {
                                for(l = 0; l<BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    } 

                                    

                                    if(GameIsValidMove(game, move))
                                    {
                                        free(move);
                                        return false;
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }
                        

                        case WHITE_ROOK:
                        {
                            int k, l;
                            // fprintf(stderr, "WHITE ROOK\n");

                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    } 

                                    

                                    if (GameIsValidMove(game, move))
                                    {
                                        free(move);
                                        return false;
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }

                        case WHITE_BISHOP:
                        {
                            int k, l;
                            // fprintf(stderr, "WHITE BISHOP \n");

                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    } 

                                  

                                    if (GameIsValidMove(game,  move))
                                    {
                                        free(move);
                                        return false;
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }


                        case WHITE_KNIGHT:
                        {
                            int k, l;
                            int knightMoves[8][2] = { {-2, -1}, {-1, -2}, {1, -2}, {2, -1}, {-2, 1}, {-1, 2}, {1, 2}, {2, 1} };
                            // fprintf(stderr, "WHITE KNHIGHT \n");

                            for (int m = 0; m < 8; m++)
                            {
                                k = i + knightMoves[m][0];
                                l = j + knightMoves[m][1];

                                if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    }

                                    

                                    if (GameIsValidMove(game, move))
                                    {
                                        free(move);
                                        return false;
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }

                        case WHITE_KING:
                        {
                            int k, l;
                            int kingMoves[8][2] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };
                            // fprintf(stderr, " WHITE KING\n");
                            for (int m = 0; m < 8; m++)
                            {
                                k = i + kingMoves[m][0];
                                l = j + kingMoves[m][1];

                                if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                {
                                    Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    }

                                    

                                    if (GameIsKingMoveValid(game,  move))
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        
                                        movePieceWithoutVerif(gameCopy,  i, j, k, l, false);
                                        if(! GameIsCheck(gameCopy, PLAYER_WHITE))
                                        {
                                            free(move);
                                            ChessFree(gameCopy);
                                            return false;
                                        }

                                        ChessFree(gameCopy);
                                    }

                                    free(move);
                                }
                            }

                            break;
                        }

                        case WHITE_PAWN: // Implémente ici
                        {
                            int k, l;

                            if (game->epWhiteCol != -1) // les blancs peuvent prendre en passant
                            {
                                int pawnMoves[2][2] = { {-1, -1}, {-1, 1} }; // Mouvements diagonaux pour la prise en passant

                                // Vérification des coups diagonaux pour la prise en passant
                                for (int m = 0; m < 2; m++)
                                {
                                    k = i + pawnMoves[m][0];
                                    l = j + pawnMoves[m][1];

                                    if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                    {
                                        Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                                        if (!move)
                                        {
                                            printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                            return NULL;
                                        }

                                        

                                        if (PriseEnPassantValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                                        {
                                            free(move);
                                            return false;
                                        }

                                        free(move);
                                    }
                                }
                            }
                            // Vérification du coup avancé
                            k = i - 1;
                            l = j;

                            Move *move = CreateMove(PLAYER_WHITE, i, j, k, l);
                            if (!move)
                            {
                                printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                return NULL;
                            }

                           
                            if (GameIsValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                free(move);
                                return false;
                            }

                            k = i - 2;

                            newEndRow(move, k);

                            if (GameIsValidMove(game,  move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                // fprintf(stderr, "dans le if 2 \n");
                                free(move);
                                return false;
                            }

                            k = i - 1;
                            l = j - 1;

                            newEndRow(move, k);
                            newEndCol(move, l);

                            if (GameIsValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                free(move);
                                return false;
                            }

                            l = j + 1;

                            newEndCol(move, l);

                            if (GameIsValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                            {
                                free(move);
                                return false;
                            }

                            free(move);

                            break;
                        }


                    }
                }
            }
        }
        if(printWhy)
            printf("Les Blancs n'ont aucun coup légal, PAT => match nul\n");
        return true;
    }

    if (current_player == PLAYER_BLACK)  
    {
        for (i = 0; i < BOARD_SIZE; i++)
        {
            for (j = 0; j < BOARD_SIZE; j++)
            {
                Piece current_piece = GetPieceOnCase(game, i, j);

                if (current_piece < 0)
                {
                    switch (current_piece)
                    {
                        case EMPTY_CASE:
                        case WHITE_QUEEN:
                        case WHITE_KING:
                        case WHITE_BISHOP:
                        case WHITE_KNIGHT:
                        case WHITE_ROOK:
                        case WHITE_PAWN:                        
                        {
                            break;
                        }
                        case BLACK_QUEEN:
                        {
                            int k, l;

                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    }

                                   
                                    if (GameIsValidMove(game,move))
                                    {
                                        free(move);
                                        return false;
                                    }
                                    free(move);
                                }
                            }

                            break;
                        }

                        case BLACK_ROOK:
                        {
                            int k, l;

                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    }   

                                   
                                    if (GameIsValidMove(game, move))
                                    {
                                        free(move);
                                        return false;
                                    }

                                
                                    free(move);
                                }
                            }

                            break;
                        }

                        case BLACK_BISHOP:
                        {
                            int k, l;

                            for (k = 0; k < BOARD_SIZE; k++)
                            {
                                for (l = 0; l < BOARD_SIZE; l++)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    }

                                    

                                    if (GameIsValidMove(game, move))
                                    {
                                        free(move);
                                        return false;
                                    }
                                    free(move);
                                }
                            }

                            break;
                        }


                        case BLACK_KNIGHT:
                        {
                            int k, l;
                            int knightMoves[8][2] = { {-2, -1}, {-1, -2}, {1, -2}, {2, -1}, {-2, 1}, {-1, 2}, {1, 2}, {2, 1} };

                            for (int m = 0; m < 8; m++)
                            {
                                k = i + knightMoves[m][0];
                                l = j + knightMoves[m][1];

                                if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    }

                                   

                                    if (GameIsValidMove(game, move))
                                    {
                                        free(move);
                                        return false;
                                    }
                                    free(move);
                                }
                            }

                            break;
                        }

                        case BLACK_KING:
                        {
                            int k, l;
                            int kingMoves[8][2] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };

                            for (int m = 0; m < 8; m++)
                            {
                                k = i + kingMoves[m][0];
                                l = j + kingMoves[m][1];

                                if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                {
                                    Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                    if (!move)
                                    {
                                        printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                        return NULL;
                                    }

                                    

                                    if (GameIsKingMoveValid(game,  move))
                                    {
                                        ChessGame *gameCopy = copyChessGame(game);
                                        
                                        movePieceWithoutVerif(gameCopy, i, j, k, l, false);
                                        if(! GameIsCheck(gameCopy, PLAYER_BLACK))
                                        {
                                            
                                            free(move);
                                            ChessFree(gameCopy);
                                            
                                            
                                            return false;
                                        }

                                        ChessFree(gameCopy);
                                    }
                                    free(move);
                                }
                            }

                            break;
                        }


                        case BLACK_PAWN: // Implémente ici
                        {
                            int k, l;

                            if (game->epBlackCol != -1) // les noirs peuvent prendre en passant => vérifier si ça annule l'échec
                            {
                                int pawnMoves[2][2] = { {1, -1}, {1, 1} }; // Mouvements diagonaux pour la prise en passant

                                // Vérification des coups diagonaux pour la prise en passant
                                for (int m = 0; m < 2; m++)
                                {
                                    k = i + pawnMoves[m][0];
                                    l = j + pawnMoves[m][1];

                                    if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
                                    {
                                        Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                                        if (!move)
                                        {
                                            printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                            return NULL;
                                        }

                                        

                                        if (PriseEnPassantValidMove(game,  move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                                        {
                                            free(move);
                                            return false;
                                        }
                                    }
                                }
                            }
                            // Vérification du coup avancé
                            k = i + 1;
                            l = j;

                            Move *move = CreateMove(PLAYER_BLACK, i, j, k, l);
                            if (!move)
                            {
                                printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                                return NULL;
                            }

                            

                            if (GameIsValidMove(game,move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                free(move);
                                return false;
                            }

                            k = i + 2;

                            newEndRow(move, k);

                            if (GameIsValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                free(move);
                                return false;
                            }

                            k = i + 1;
                            l = j - 1;

                            newEndRow(move, k);
                            newEndCol(move, l);
                            
                            

                            if (GameIsValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                free(move);
                                return false;
                            }

                            l = j + 1;

                            newEndCol(move, l);

                            if (GameIsValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                            {
                                free(move);
                                return false;
                            }
                            
                            free(move);

                            break;
                        }


                    }
                }
            }
        }
        if(printWhy)
            printf("Les Noirs n'ont aucun coup légal, PAT => match nul\n");
        return true;
    }

    return true;
}


//vérifie si il n'y a pas nulle par manque de matériel
bool ChessGameNoMatMin(ChessGame *game, bool printWhy)
{
    bool WhiteKnightOrBishop = false;
    bool BlackKnightOrBishop = false;

    int i, j;
    for(i=0; i<BOARD_SIZE; i++)
    {
        for(j=0; j<BOARD_SIZE; j++)
        {
            Piece current_piece = GetPieceOnCase(game, i, j);

            if(current_piece == WHITE_QUEEN || current_piece == WHITE_ROOK || current_piece == WHITE_PAWN)
            {
                return false;
            }

            if(current_piece == BLACK_QUEEN || current_piece == BLACK_ROOK || current_piece == BLACK_PAWN)
            {
                return false;
            }

            if(current_piece == WHITE_BISHOP || current_piece == WHITE_KNIGHT)
            {
                if(WhiteKnightOrBishop)
                {
                    return false;
                }

                WhiteKnightOrBishop = true;
            }

            if(current_piece == BLACK_BISHOP || current_piece == BLACK_KNIGHT)
            {
                if(BlackKnightOrBishop)
                {
                    return false;
                }

                BlackKnightOrBishop = true;
            }

        }
    }
    if(printWhy)
        printf("Match Nul par manque de matériel\n");
    return true;
}


bool ChessGameRepetition(ChessGame *game, bool printWhy)
{
    int count = 0;

    unsigned long long currentZobristKey = getCurrentZobristKey(game);
    
    if(game->zobristList == NULL)
    {
        return false;
    }

    for(GList *l = game->zobristList; l != NULL; l = l->next)
    {
        // Récupérer la clé Zobrist pointée dynamiquement
        unsigned long long *loopKeyPtr = (unsigned long long *)l->data;
        if(loopKeyPtr == NULL)
        {
            continue;  // Ignore si le pointeur est NULL
        }

        // Comparer la valeur de la clé Zobrist
        if(currentZobristKey == *loopKeyPtr)
        {
            count++;
        }

        // Si la répétition est trouvée 3 fois
        if(count >= 3)
        {
            if(printWhy)
            {
                fprintf(stderr, "Match Nul par répétition\n");
            }

            return true;
        }
    }

    return false;
}
bool CompareGame(ChessGame *g1, ChessGame *g2)
{
    int i, j;
    
    for(i = 0; i<BOARD_SIZE; i++)
    {
        for(j=0; j<BOARD_SIZE; j++)
        {
            
            Piece p1 = GetPieceOnCase(g1, i, j);  
           
          
            Piece p2 = GetPieceOnCase(g2, i, j);
            

            if(p1 != p2)
            {
                return false;
            }
            
        }
    }
    return true;
}


GList *GetALLMove(ChessGame *game, bool onlyCaptures)
{
    GList *moveList = NULL;
    for(int i=0; i<BOARD_SIZE; i++)
    {
        for(int j=0; j<BOARD_SIZE;j++)
        {
            Piece piece = GetPieceOnCase(game, i, j);
            
            if( (GetCurrentPlayer(game) == PLAYER_WHITE && piece > 0) || (GetCurrentPlayer(game) == PLAYER_BLACK && piece < 0) )
            {
                GList *temp = GetALLMoveFromCase(game, i, j, onlyCaptures);   
            
                moveList = g_list_concat(moveList, temp);
                
            }
        }
    }   
    

    return g_list_sort(moveList, (GCompareFunc)compareMoves);
}
GList *GetALLMoveFromCase(ChessGame *game, int start_row, int start_col, bool onlyCaptures)
{
    
    if(start_row < 0 || start_row >= BOARD_SIZE)
        return NULL;
   
    Piece start_piece = GetPieceOnCase(game, start_row, start_col);
    if(start_piece == EMPTY_CASE)
        return NULL;

    
    if(GetCurrentPlayer(game) == PLAYER_WHITE && start_piece < 0)
        return NULL;

    
    if(GetCurrentPlayer(game) == PLAYER_BLACK && start_piece > 0)
        return NULL;

   
    switch(start_piece)
    {
        case WHITE_PAWN:
        case BLACK_PAWN:
        {
            // fprintf(stderr, "before GetAllPawnMove\n");
            GList *moveList = GetALLPawnMove(game, start_row, start_col, onlyCaptures);
            // fprintf(stderr, "length of moveListPawn = %d\n", g_list_length(moveList));

            return moveList;
            break;
        }

        case WHITE_KING:
        case BLACK_KING:
        {
            GList *moveList = GetALLKingMove(game, start_row, start_col, onlyCaptures);
            
            return moveList;
            break;

        }
        case WHITE_QUEEN:
        case BLACK_QUEEN:
        {
            GList *moveList = GetALLQueenMove(game, start_row, start_col, onlyCaptures);
            
            return moveList;
            break;
        }

        case WHITE_ROOK:
        case BLACK_ROOK:
        {
            GList *moveList = GetALLRookMove(game, start_row, start_col, onlyCaptures);
            
            return moveList;
            break;
        }

        case WHITE_BISHOP:
        case BLACK_BISHOP:
        {
            
            GList *moveList = GetALLBishopMove(game, start_row, start_col, onlyCaptures);
            // fprintf(stderr, "length of BISHOPmoveList = %d\n", g_list_length(moveList));
            // PrintAllMoves(moveList);

            
            return moveList;
            break;
        }

        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
        {
            GList *moveList = GetALLKnightMove(game, start_row, start_col, onlyCaptures);
           
            return moveList;
            break;
        }

        default:
        {
            
            return NULL;
            break;
        }
    }

    

    return NULL;
}

GList* GetALLPawnMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures) 
{
    GList *moveList = NULL;

   
    int k, l;
    if (game->epBlackCol != -1 && GetCurrentPlayer(game) == PLAYER_BLACK) // les noirs peuvent prendre en passant
    {
        int pawnMoves[2][2] = { {1, -1}, {1, 1} }; // Mouvements diagonaux pour la prise en passant

        // Vérification des coups diagonaux pour la prise en passant
        for (int m = 0; m < 2; m++)
        {
            k = start_row + pawnMoves[m][0];
            l = start_col + pawnMoves[m][1];

            if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
            {
                Move *move = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);
                if (!move)
                {
                    printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                    return NULL;
                }

                

                if (PriseEnPassantValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
                {
                    newMoveType(move, PriseEnPassant);
                    
                    moveList = g_list_append(moveList, move); 
                    
                }
                else 
                    FreeMove(move);
                
                
            }
        }
    }

    if (game->epWhiteCol != -1 && GetCurrentPlayer(game) == PLAYER_WHITE) // les blancs peuvent prendre en passant
    {
        int pawnMoves[2][2] = { {-1, -1}, {-1, 1} }; // Mouvements diagonaux pour la prise en passant

        // Vérification des coups diagonaux pour la prise en passant
        for (int m = 0; m < 2; m++)
        {
            k = start_row + pawnMoves[m][0];
            l = start_col + pawnMoves[m][1];

            if (k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
            {
                Move *move = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);
                if (!move)
                {
                    printf("Erreur pour allouer la mémoire du move (fonction : ChessGameIsPat)\n");
                    return NULL;
                }

                
                
                if(PriseEnPassantValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
                {
                    newMoveType(move, PriseEnPassant);
                    
                    moveList = g_list_append(moveList, move);
                       
                }
                else
                    FreeMove(move);
                
                

            }
        }
    }
    // fprintf(stderr, "apres le if de la prise en passant \n");
    // Vérification du coup avancé
    if(GetCurrentPlayer(game) == PLAYER_WHITE)
    {
        k = start_row - 1;
        l = start_col;
        
        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {
            Move *move = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);
            if (!move)
            {
                printf("Erreur pour allouer la mémoire du move (fonction : GetAllPawnMoves)\n");
                return NULL;
            }

            
            if (GameIsValidMove(game, move)) 
            {
                if(k == 0) //promotion => ajouter les 4 choix dans la liste des coups
                {
                    FreeMove(move);
                    for(int response = 1; response < 5; response++)
                    {
                        Move *loopMove = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);
                        newMoveType(loopMove, Promotion);
                        newResponse(loopMove, response);
                        moveList = g_list_append(moveList, loopMove);
                    }

                }
                else
                {
                    if(onlyCaptures)
                        FreeMove(move);
                    else
                    {
                        newMoveType(move, Classique);
                        moveList = g_list_append(moveList, move);
                    }
                }
            }
            else
                FreeMove(move);
            // fprintf(stderr, "1 \n");
        }

        k = start_row - 2;

        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {
            
            Move *new_move = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);

            if (GameIsValidMove(game, new_move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
            {
                if(onlyCaptures)
                    FreeMove(new_move);
                else
                {
                    newMoveType(new_move, Classique);
                    moveList = g_list_append(moveList, new_move);
                }
                
            }
            else 
                FreeMove(new_move);


        }

        k = start_row - 1;
        l = start_col - 1;

        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {

            

            Move *new_move2 = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);
            
        

            
            if (GameIsValidMove(game, new_move2)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
            {
                if(k == 0) //promotion => ajouter les 4 choix dans la liste des coups
                {
                    FreeMove(new_move2);
                    for(int response = 1; response < 5; response++)
                    {
                        Move *loopMove = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);
                        newMoveType(loopMove, Promotion);
                        newResponse(loopMove, response);
                        moveList = g_list_append(moveList, loopMove);
                    }

                }
                else
                {
                    newMoveType(new_move2, Prise);
                    moveList = g_list_append(moveList, new_move2);
                }
            }
            else 
                FreeMove(new_move2);

        }

       

        

        l = start_col + 1;

        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {

            Move *new_move3 = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);

            if (GameIsValidMove(game, new_move3)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les blancs sont encore en échecs
            {
                
                if(k == 0) //promotion => ajouter les 4 choix dans la liste des coups
                {
                    FreeMove(new_move3);
                    for(int response = 1; response < 5; response++)
                    {
                        Move *loopMove = CreateMove(PLAYER_WHITE, start_row, start_col, k, l);
                        newMoveType(loopMove, Promotion);
                        newResponse(loopMove, response);
                        moveList = g_list_append(moveList, loopMove);
                    }

                }
                else
                {
                    
                    newMoveType(new_move3, Prise);
                    moveList = g_list_append(moveList, new_move3);
                }
            }
            else
                FreeMove(new_move3);

        }

        
    }

    if(GetCurrentPlayer(game) == PLAYER_BLACK)
    {
        k = start_row + 1;
        l = start_col;

        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {

                
            Move *move = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);
            if (!move)
            {
                printf("Erreur pour allouer la mémoire du move (fonction : GetAllPawnMoves)\n");
                return NULL;
            }

            if (GameIsValidMove(game, move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
            {
                if(k == 7) //promotion => ajouter les 4 choix dans la liste des coups
                {
                    FreeMove(move);
                    for(int response = 1; response < 5; response++)
                    {
                        Move *loopMove = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);
                        newResponse(loopMove, response);
                        newMoveType(loopMove, Promotion);
                        moveList = g_list_append(moveList, loopMove);
                    }

                }
                else
                {
                    if(onlyCaptures)
                        FreeMove(move);
                    else
                    {
                        newMoveType(move, Classique);
                        moveList = g_list_append(moveList, move);
                    }
                }
                
            }
            else
                FreeMove(move);



        }

        k = start_row + 2;

        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {
            Move *new_move = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);

            if (GameIsValidMove(game, new_move)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
            {
                
                if(onlyCaptures)
                    FreeMove(new_move);
                else
                {
                    newMoveType(new_move, Classique);
                    moveList = g_list_append(moveList, new_move);
                }
                
            }
            else 
                FreeMove(new_move);

        }
        
        
        k = start_row + 1;
        l = start_col - 1;

        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {

            Move *new_move2 = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);

            if (GameIsValidMove(game, new_move2)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
            {
                if(k == 7) //promotion => ajouter les 4 choix dans la liste des coups
                {
                    FreeMove(new_move2);
                    for(int response = 1; response < 5; response++)
                    {
                        Move *loopMove = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);
                        newResponse(loopMove, response);
                        newMoveType(loopMove, Promotion);
                        moveList = g_list_append(moveList, loopMove);
                    }

                }
                else
                {
                    
                    newMoveType(new_move2, Prise);
                    moveList = g_list_append(moveList, new_move2);
                }
            }
            else 
                FreeMove(new_move2);

        }

       

        l = start_col + 1;

        if(k >= 0 && k < BOARD_SIZE && l >= 0 && l < BOARD_SIZE)
        {
            Move *new_move3 = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);

            if (GameIsValidMove(game, new_move3)) // Le pion peut jouer un coup, jouer le coup dans une copie et vérifier si les noirs sont encore en échecs
            {
                if(k == 7) //promotion => ajouter les 4 choix dans la liste des coups
                {
                    FreeMove(new_move3);
                    for(int response = 1; response < 5; response++)
                    {
                        Move *loopMove = CreateMove(PLAYER_BLACK, start_row, start_col, k, l);
                        newResponse(loopMove, response);
                        newMoveType(loopMove, Promotion);
                        moveList = g_list_append(moveList, loopMove);
                    }

                }
                else
                {
                    
                    newMoveType(new_move3, Prise);
                    moveList = g_list_append(moveList, new_move3);
                }
            }
            else
                FreeMove(new_move3);
        }

        
    }


    return moveList;




}




GList* GetALLKingMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures) 
{
    Piece piece = GetPieceOnCase(game, start_row, start_col);
    if(piece != WHITE_KING && piece != BLACK_KING)
    {
        fprintf(stderr, "Ne peut pas renvoyer les coups d'un roi car ce n'est pas un roi sur la case");
        return NULL;
    }

    GList *moveList = NULL;
    int directions[8][2] = {{1, 0}, {-1, 0}, {0, +1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    
    Player player = GetCurrentPlayer(game);

    for(int i = 0; i < 8; i++)
    {
        int end_row = start_row + directions[i][0];
        int end_col = start_col + directions[i][1];

        // Vérifiez que le mouvement est à l'intérieur des limites du plateau
        if(end_row >= 0 && end_row < BOARD_SIZE && end_col >= 0 && end_col < BOARD_SIZE)
        {
            Move *move = CreateMove(player, start_row, start_col, end_row, end_col);
            if (!move)
            {
                fprintf(stderr, "Erreur d'allocation de mémoire pour le move\n");
                return NULL; // Retourne les mouvements déjà trouvés
            }
            Piece arrivePiece = GetPieceOnCase(game, end_row, end_col);

            if((player == PLAYER_WHITE && arrivePiece <= 0) || (player == PLAYER_BLACK && arrivePiece >= 0))
            {
                if(!IsCheckMove(game, move))
                {
                    if(arrivePiece != 0)
                        newMoveType(move, Prise);
                    
                    if(onlyCaptures && arrivePiece == 0)
                        FreeMove(move);
                    else
                        moveList = g_list_append(moveList, move);
                }
                else 
                    FreeMove(move);
            }
            else
                FreeMove(move);
            
            
        }
    }
    if(! onlyCaptures)
    {
        //ROQUE BLANC

        if(player == PLAYER_WHITE)
        {
            //Petit roque
            int end_row, end_col;
            end_row = 7;
            end_col = 6;

            Move *move = CreateMove(player, start_row, start_col, end_row, end_col);

            if(RoqueIsValidMove(game, move))
            {
                newMoveType(move, Roque);
                moveList = g_list_append(moveList, move);
            }
            else    
                FreeMove(move);

            //Grand roque
            end_row = 7;
            end_col = 2;
            Move *move2 = CreateMove(player, start_row, start_col, end_row, end_col);

            if(RoqueIsValidMove(game, move2))
            {
                newMoveType(move2, Roque);
                moveList = g_list_append(moveList, move2);
            }
            else    
                FreeMove(move2);
        }

        //ROQUE NOIR
        if(player == PLAYER_BLACK)
        {
            //Petit roque
            int end_row, end_col;
            end_row = 0;
            end_col = 6;
            Move *move = CreateMove(player, start_row, start_col, end_row, end_col);

            if(RoqueIsValidMove(game, move))
            {
                newMoveType(move, Roque);
                moveList = g_list_append(moveList, move);
            }
            else    
                FreeMove(move);

            //Grand roque
            end_row = 0;
            end_col = 2;
            Move *move2 = CreateMove(player, start_row, start_col, end_row, end_col);

            if(RoqueIsValidMove(game, move2))
            {
                newMoveType(move2, Roque);
                moveList = g_list_append(moveList, move2);
            }
            else    
                FreeMove(move2);
        }
    }

    return moveList;

}




GList* GetALLQueenMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures) 
{
    Piece piece = GetPieceOnCase(game, start_row, start_col);
    if(piece != WHITE_QUEEN && piece != BLACK_QUEEN)
    {
        fprintf(stderr, "Ne peut pas renvoyer les coups d'une reine car ce n'est pas une reine sur la case");
        return NULL;
    }

    GList *moveList = NULL;

    // Obtenir tous les mouvements de la tour
    GList *rookMoves = GetALLRookMove(game, start_row, start_col, onlyCaptures);
    moveList = g_list_concat(moveList, rookMoves);

    // Obtenir tous les mouvements du fou
    GList *bishopMoves = GetALLBishopMove(game, start_row, start_col, onlyCaptures);
    moveList = g_list_concat(moveList, bishopMoves);

   

    return moveList;
}



GList* GetALLRookMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures) 
{
    Piece piece = GetPieceOnCase(game, start_row, start_col);
    if(piece != WHITE_ROOK && piece != BLACK_ROOK && piece != WHITE_QUEEN && piece != BLACK_QUEEN)
    {
        fprintf(stderr, "Ne peut pas renvoyer les coups d'une tour car ce n'est pas une tour sur la case");
        return NULL;
    }

    GList *moveList = NULL;
    int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; // Déplacements verticaux et horizontaux
    Player player = GetCurrentPlayer(game);

    for(int i = 0; i < 4; i++)
    {
        int end_row = start_row;
        int end_col = start_col;

        while(true)
        {
            end_row += directions[i][0];
            end_col += directions[i][1];

            // Vérifiez que le mouvement est à l'intérieur des limites du plateau
            if(end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col >= BOARD_SIZE)
            {
                break;
            }
            Piece arrivePiece = GetPieceOnCase(game, end_row, end_col);

            Move *move = CreateMove(player, start_row, start_col, end_row, end_col);
            if (!move)
            {
                fprintf(stderr, "Erreur d'allocation de mémoire pour le move\n");
                return moveList; // Retourne les mouvements déjà trouvés
            }

            

            if(GameIsValidMove(game, move))
            {
                if(arrivePiece != 0)
                    newMoveType(move, Prise);
                if(onlyCaptures && arrivePiece == 0)
                {
                    FreeMove(move);
                }
                else
                    moveList = g_list_append(moveList, move);


                // Si la case est occupée par une pièce ennemie, arrêtez de vérifier dans cette direction
                if(GetPieceOnCase(game, end_row, end_col) != EMPTY_CASE)
                {
                    break;
                }
            }
            else
            {
                FreeMove(move);
                if(GetPieceOnCase(game, end_row, end_col) != EMPTY_CASE)
                {
                    break;
                }
                // break;
            }
            
        }
    }

    return moveList;
}






GList* GetALLBishopMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures) 
{
    Piece piece = GetPieceOnCase(game, start_row, start_col);
    if(piece != WHITE_BISHOP && piece != BLACK_BISHOP && piece != WHITE_QUEEN && piece != BLACK_QUEEN)
    {
        fprintf(stderr, "Ne peut pas renvoyer les coups d'un fou car ce n'est pas un fou sur la case");
        return NULL;
    }

    GList *moveList = NULL;
    int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}; // Déplacements diagonaux

    Player player = GetCurrentPlayer(game);

    for(int i = 0; i < 4; i++)
    {
        int end_row = start_row;
        int end_col = start_col;

        while(true)
        {
            end_row += directions[i][0];
            end_col += directions[i][1];



            // Vérifiez que le mouvement est à l'intérieur des limites du plateau
            if(end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col >= BOARD_SIZE)
            {

                break;
            }

            Piece arrivePiece = GetPieceOnCase(game, end_row, end_col);

            Move *move = CreateMove(player, start_row, start_col, end_row, end_col);
            if (!move)
            {
                fprintf(stderr, "Erreur d'allocation de mémoire pour le move\n");
                return moveList; // Retourne les mouvements déjà trouvés
            }

            
            if(GameIsValidMove(game, move))
            {
                if(arrivePiece != 0)
                    newMoveType(move, Prise);

                if(onlyCaptures && arrivePiece == 0)
                    FreeMove(move);
                else
                    moveList = g_list_append(moveList, move);

                // Si la case est occupée par une pièce ennemie, arrêtez de vérifier dans cette direction
                if(GetPieceOnCase(game, end_row, end_col) != EMPTY_CASE)
                {
                    break;
                }
            }
            else
            {
                // Libérez la mémoire du mouvement invalide
                FreeMove(move);

                // Si la case est occupée par une pièce ennemie, arrêtez de vérifier dans cette direction
                if(GetPieceOnCase(game, end_row, end_col) != EMPTY_CASE)
                {
                    break;
                }

                // break;
            }
        }
    }

    return moveList;
}


GList* GetALLKnightMove(ChessGame *game, int start_row, int start_col, bool onlyCaptures) 
{
    // fprintf(stderr, "1\n");
    Piece piece = GetPieceOnCase(game, start_row, start_col);
    if(piece != WHITE_KNIGHT && piece != BLACK_KNIGHT)
    {
        fprintf(stderr, "Ne peut pas renvoyer les coups d'un cavalier car ce n'est pas un cavalier sur la case");
        return NULL;
    }

    GList *moveList = NULL;
    int knightMoves[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };

    Player player = GetCurrentPlayer(game);
    // fprintf(stderr, "2\n");

    for(int i = 0; i < 8; i++)
    {
        int end_row = start_row + knightMoves[i][0];
        int end_col = start_col + knightMoves[i][1];

        // Vérifier que la destination est sur l'échiquier

        if(end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col >= BOARD_SIZE)
            continue;

        // fprintf(stderr, "3\n");

        Piece endPiece = GetPieceOnCase(game, end_row, end_col);
        // fprintf(stderr, "3.5\n");
        if(player == PLAYER_WHITE && endPiece > 0)
            continue;
        else if(player == PLAYER_BLACK && endPiece < 0)
            continue;

        // fprintf(stderr, "4\n");
     

        Move *move = CreateMove(player, start_row, start_col, end_row, end_col);
        if (!move)
        {
            printf("Erreur pour allouer la mémoire du move (fonction : GetALLKnightMove)\n");
            return NULL;
        }

        if(IsCheckMove(game, move))
        {
            FreeMove(move);
            continue;
        }
        else
        {
            if(endPiece != 0)
                newMoveType(move, Prise);

            if(onlyCaptures && endPiece == 0)
                FreeMove(move);
            else
                moveList = g_list_append(moveList, move);
        }        
            
        
    }

    return moveList;
}





bool IsCheckMove(ChessGame *game, Move *move) //joue le coup dans une copie du jeu et renvoie si le joueur s'est mis en échec (renvoie true si il y a échec)
{
    ChessGame *gameCopy = copyChessGame(game);
    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);
    end_col = getEndColFromMove(move);
    // Player player = GetCurrentPlayer(game);
    
    movePieceWithoutVerif(gameCopy, start_row, start_col,end_row, end_col, false);
    // PrintMove(move);

    
    // PlayMoveAI(gameCopy, move);


    if(GameIsCheck(gameCopy, GetCurrentPlayer(game)))
    {
        ChessFree(gameCopy);
        return true;

    }

    // fprintf(stderr, "1\n");

    ChessFree(gameCopy);
    // fprintf(stderr, "2\n");
    
    return false;
}
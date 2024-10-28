#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>


#include "Zobrist.h"


#define BOARD_SIZE 8

unsigned long long piecesArray[8][8][12];
unsigned long long castlingRights[4]; //WK, WQ, BK, BQ
unsigned long long enPassantCol[8];
unsigned long long sideToMove[2]; //0 pour blanc //1 pour noir



ZobristMove *createZobristMove(unsigned long long key, double eval, int depth, Move *bestMove)
{
    ZobristMove *move = malloc(sizeof(ZobristMove));
    if(move == NULL)
    {
        fprintf(stderr, "Error in createZobristMove\n");
        return NULL;
    }

    move->key = key;
    move->eval = eval;
    move->depth = depth;
    move->bestMove = bestMove;

    return move;
}

void ZobristInit(void)
{
    const int seed = 29426028;
    srand(seed);  

    for(int row = 0; row < 8; row++) 
    {
        for(int col = 0; col < 8; col++) 
        {
            for(int piece = 0; piece < 12; piece++) 
            {
                piecesArray[row][col][piece] = RandomUnsigned64BitNumber();
            }
        }
    }

    for(int i = 0; i < 4; i++) {
        castlingRights[i] = RandomUnsigned64BitNumber();
    }

    for(int i = 0; i < 8; i++) {
        enPassantCol[i] = RandomUnsigned64BitNumber();
    }

    sideToMove[0] = RandomUnsigned64BitNumber();
    sideToMove[1] = RandomUnsigned64BitNumber();

    return ;

}

unsigned long long getZobristKey(ChessGame *game)
{
    unsigned long long key = 0;

    for(int row = 0; row < BOARD_SIZE; row++)
    {
        for(int col = 0; col < BOARD_SIZE; col ++)
        {
            Piece piece = GetPieceOnCase(game, row, col);
            if(piece != EMPTY_CASE)
            {
                key ^= piecesArray[row][col][pieceToValue(piece)];
            }
        }
    }

    if(getEpWhiteCol(game) != -1)
    {
        
        int start_row = 3;

        int end_col = getEpWhiteCol(game);
        int start_col1 = end_col -1;
        int start_col2 = end_col +1;
        
        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_WHITE, start_row, start_col1, 2, end_col);
            if(PriseEnPassantValidMove(game, move1))
            {
                key ^= enPassantCol[end_col];
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8)
        {
            Move *move2 = CreateMove(PLAYER_WHITE, start_row, start_col2, 2, end_col);
            if(PriseEnPassantValidMove(game, move2))
            {
                key ^= enPassantCol[end_col];
            }
            free(move2);
        }
    }
    else if(getEpBlackCol(game) != -1)
    {

        int end_col = getEpBlackCol(game);
        int start_row = 4;

        int start_col1 = end_col + 1;
        int start_col2 = end_col -1;


        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_BLACK, start_row, start_col1, 5, end_col);
            if(PriseEnPassantValidMove(game, move1))
            {
                key ^= enPassantCol[end_col];
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8)
        {
            Move *move2 = CreateMove(PLAYER_BLACK, start_row, start_col2, 5, end_col);
            if(PriseEnPassantValidMove(game, move2))
            {
                key ^= enPassantCol[end_col];
            }
            free(move2);
        }
    }

    if(getWK(game)) //petit roque blanc
    {
        key ^= castlingRights[0];
    }
    if(getWQ(game)) //grand roque blanc
    {
        key ^= castlingRights[1];
    }
    if(getBK(game)) //petit roque noir
    {
        key ^= castlingRights[2];
    }
    if(getBQ(game)) //grand roque noir
    {
        key ^= castlingRights[3];
    }

    if(GetCurrentPlayer(game) == PLAYER_WHITE)
    {
        key ^= sideToMove[0];
    }
    else
        key ^= sideToMove[1];
    
    return key;
    
}

unsigned long long getUpdZobristKey(ChessGame *game, Move *move) //prend en argument la game avant que le coup ne soit joué 
{
    unsigned long long newKey = getCurrentZobristKey(game);

    int start_row, start_col;
    int end_row, end_col;
    start_row = getStartRowFromMove(move);
    start_col = getStartColFromMove(move);
    end_row = getEndRowFromMove(move);   
    end_col = getEndColFromMove(move);
    Player player = getPlayerFromMove(move);
    MoveType moveType = getMoveTypeFromMove(move);

    Piece startPiece = GetPieceOnCase(game, start_row, start_col);
    Piece endPiece = GetPieceOnCase(game, end_row, end_col);


    // enlever la piece qui bouge de sa case initiale
    newKey ^= piecesArray[start_row][start_col][pieceToValue(startPiece)];

    if(moveType == Classique)
    {
        newKey ^= piecesArray[end_row][end_col][pieceToValue(startPiece)];
    }
    else if(moveType == Prise)
    {
        newKey ^= piecesArray[end_row][end_col][pieceToValue(endPiece)];
        newKey ^= piecesArray[end_row][end_col][pieceToValue(startPiece)];
    }
    else if(moveType == Roque)
    {
        if(start_row == 7 && end_col == 6) //petit roque blanc
        {
            newKey ^= piecesArray[7][7][VALUE_WHITE_ROOK];
            newKey ^= piecesArray[7][5][VALUE_WHITE_ROOK];
        }
        else if(start_row == 7 && end_col == 2) //grand roque blanc
        {
            newKey ^= piecesArray[7][0][VALUE_WHITE_ROOK];
            newKey ^= piecesArray[7][3][VALUE_WHITE_ROOK];
        }
        else if(start_row == 0 && end_col == 6) //petit roque noir
        {
            newKey ^= piecesArray[0][7][VALUE_BLACK_ROOK];
            newKey ^= piecesArray[0][5][VALUE_BLACK_ROOK];
        }
        else if(start_row == 0 && end_col == 2) //grand roque noir
        {
            newKey ^= piecesArray[0][0][VALUE_BLACK_ROOK];
            newKey ^= piecesArray[0][3][VALUE_BLACK_ROOK];
        }

        newKey ^= piecesArray[end_row][end_col][pieceToValue(startPiece)];
    }
    else if(moveType == Promotion)
    {
        if(endPiece != EMPTY_CASE)
        {
            newKey ^= piecesArray[end_row][end_col][pieceToValue(endPiece)];
        }

        newKey ^= piecesArray[end_row][end_col][pieceToValue(getPromotedPiece(player, getResponseFromMove(move)))];
    }
    else if(moveType == PriseEnPassant)
    {
        int capturedPawnRow;
        if(player == PLAYER_WHITE)
        {
            capturedPawnRow = 3;
            newKey ^= piecesArray[capturedPawnRow][end_col][pieceToValue(BLACK_PAWN)];
        }
        else 
        {
            capturedPawnRow = 4;
            newKey ^= piecesArray[capturedPawnRow][end_col][pieceToValue(WHITE_PAWN)];
        }

        newKey ^= piecesArray[end_row][end_col][pieceToValue(startPiece)];        
    }

    //changer le tour du joueur : 

    if(player == PLAYER_WHITE)
    {
        newKey ^= sideToMove[0]; //pour annuler la fait que c'était aux blancs
        newKey ^= sideToMove[1];
    }
    else
    {
        newKey ^= sideToMove[1]; //pour annuler la fait que c'était aux noirs
        newKey ^= sideToMove[0]; 
    }

    //cas du roque

    // annuler les droits de prise de roque si nécéssaire:
    //attention il ne faut pas annuler si c'était deja pas permis, sinon on réautoriserait

    if(getWK(game)) // => dans le if implique que le petit roque blanc était possible => vérifier s'il ne faut pas l'empêcher
    {
        if(startPiece == WHITE_KING || (startPiece == WHITE_ROOK && start_row == 7 && start_col == 7))
            newKey ^= castlingRights[0];
    }
    if(getWQ(game)) //grand roque blanc 
    {
        if(startPiece == WHITE_KING || (startPiece == WHITE_ROOK && start_row == 7 && start_col == 0))
            newKey ^= castlingRights[1];
    }
    if(getBK(game)) //petit roque noir
    {
        if(startPiece == BLACK_KING || (startPiece == BLACK_ROOK && start_row == 0 && start_col == 7))
            newKey ^= castlingRights[2];
    }
    if(getBQ(game)) //grand roque noir
    {
        if(startPiece == BLACK_KING || (startPiece == BLACK_ROOK && start_row == 0 && start_col == 0))
            newKey ^= castlingRights[3];
    }

    //cas de la prise en Passant :
    //dabord annuler les anciens droits de prise en passant

    if(getEpWhiteCol(game) != -1)
    {
        
        int start_row = 3;

        int end_col = getEpWhiteCol(game);
        int start_col1 = end_col -1;
        int start_col2 = end_col +1;
        


        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_WHITE, start_row, start_col1, 2, end_col);
            if(PriseEnPassantValidMove(game, move1))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8)
        {
            Move *move2 = CreateMove(PLAYER_WHITE, start_row, start_col2, 2, end_col);
            if(PriseEnPassantValidMove(game, move2))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move2);
        }
    }
    else if(getEpBlackCol(game) != -1)
    {

        int end_col = getEpBlackCol(game);
        int start_row = 4;

        int start_col1 = end_col + 1;
        int start_col2 = end_col -1;


        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_BLACK, start_row, start_col1, 5, end_col);
            if(PriseEnPassantValidMove(game, move1))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8)
        {
            Move *move2 = CreateMove(PLAYER_BLACK, start_row, start_col2, 5, end_col);
            if(PriseEnPassantValidMove(game, move2))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move2);
        }
    }
    //maintenant actualiser les possibles prises en passant :

    ChessGame *copy = copyChessGame(game);
    // fprintf(stderr, "1\n");
    PlayMove2(copy, move, false);
    // fprintf(stderr, "2\n");
    
    
    if(getEpWhiteCol(copy) != -1)
    {
        
        int start_row = 3;

        int end_col = getEpWhiteCol(copy);
        int start_col1 = end_col -1;
        int start_col2 = end_col +1;
        


        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_WHITE, start_row, start_col1, 2, end_col);
            if(PriseEnPassantValidMove(copy, move1))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8)
        {
            Move *move2 = CreateMove(PLAYER_WHITE, start_row, start_col2, 2, end_col);
            if(PriseEnPassantValidMove(copy, move2))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move2);
        }
    }
    else if(getEpBlackCol(copy) != -1)
    {

        int end_col = getEpBlackCol(copy);
        int start_row = 4;

        int start_col1 = end_col + 1;
        int start_col2 = end_col -1;


        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_BLACK, start_row, start_col1, 5, end_col);
            if(PriseEnPassantValidMove(copy, move1))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8)
        {
            Move *move2 = CreateMove(PLAYER_BLACK, start_row, start_col2, 5, end_col);
            if(PriseEnPassantValidMove(copy, move2))
            {
                newKey ^= enPassantCol[end_col];
            }
            free(move2);
        }
    }

    ChessFree(copy);
    return newKey;


}

void initTranspositionTable(void)
{
    transpositionTable = g_hash_table_new_full( g_int64_hash, g_int64_equal, free, FreeZobristMove);
}

void insertTranspositionTable(ZobristMove *move)
{

    unsigned long long *key = malloc(sizeof(unsigned long long)); // Alloue de la mémoire pour la clé
    *key = getKeyFromZobristMove(move);   
    
    ZobristMove *already = (ZobristMove *)g_hash_table_lookup(transpositionTable, key);

    if(already != NULL)
    {
        // fprintf(stderr, "here\n");
       
        if(getDepthFromZobristMove(already) >= getDepthFromZobristMove(move))
        {               
            free(key);
            FreeZobristMove(move);
            return ;
        }
    }

    // attention, cette fonction écrase l'élement à cette clé si il y a deja qq chose à cette clé.
    // => il faut vérifier s'il y a pas deja du content à cette clé, si c'est le cas, on compare les profondeurs et on garde la meilleur profondeur. 
    g_hash_table_insert( transpositionTable, key, move);
}


void printZobristValues(void) 
{
    // Affichage des valeurs des pièces sur chaque case
    printf("Zobrist Values for piecesArray:\n");
    for (int piece = 0; piece < 12; piece++) {
        printf("Piece %d:\n", piece);
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                printf("%llu ", piecesArray[row][col][piece]);
            }
            printf("\n");
        }
        printf("\n");
    }

    // Affichage des valeurs pour les droits de roque
    printf("Zobrist Values for castlingRights:\n");
    for (int i = 0; i < 4; i++) {
        printf("castlingRights[%d] = %llu\n", i, castlingRights[i]);
    }

    // Affichage des valeurs pour les colonnes de prise en passant
    printf("Zobrist Values for enPassantCol:\n");
    for (int i = 0; i < 8; i++) {
        printf("enPassantCol[%d] = %llu\n", i, enPassantCol[i]);
    }

    // Affichage de la valeur pour le côté à jouer
    printf("Zobrist Value for sideToMove = %llu\n", sideToMove);

    return ;
}

unsigned long long RandomUnsigned64BitNumber(void) 
{
    return ((unsigned long long)rand() << 32) | rand();
}



unsigned long long getKeyFromZobristMove(ZobristMove *move)
{
    return move->key;
}

double getEvalFromZobristMove(ZobristMove *move)
{
    return move->eval;
}

int getDepthFromZobristMove(ZobristMove *move)
{
    return move->depth;
}
Move *getBestMoveFromZobristMove(ZobristMove *move)
{
    return move->bestMove;
}

void FreeZobristMove(ZobristMove *move)
{
    free(getBestMoveFromZobristMove(move));
    free(move);
    return ;
}

void freeTranspositionTable(void) 
{
    g_hash_table_destroy(transpositionTable);
}

//with un passage par valeur
// GList *copyGlistOfZobrist(GList *zobristList) 
// {
//     GList *copiedList = NULL;

//     for (GList *node = zobristList; node != NULL; node = node->next) {
//         // Ajoute l'élément actuel à la nouvelle liste
//         // Ici, on copie la valeur par valeur, donc pas d'allocation dynamique
//         copiedList = g_list_append(copiedList, node->data);
//     }

//     return copiedList;
// }


//with the allocatedKey
GList *copyGlistOfZobrist(GList *zobristList) 
{
    if (zobristList == NULL) {
        return NULL;
    }

    GList *newList = NULL;
    for (GList *current = zobristList; current != NULL; current = current->next) {
        // Copier chaque élément de type 'unsigned long long'
        unsigned long long *valueCopy = (unsigned long long *)malloc(sizeof(unsigned long long));
        if (valueCopy == NULL) {
            fprintf(stderr, "Erreur d'allocation de mémoire dans copyGlistOfZobrist\n");
            return NULL; // Gestion d'erreur simplifiée
        }

        *valueCopy = *(unsigned long long *)(current->data);
        newList = g_list_append(newList, valueCopy);
    }

    return newList;
}


// GList *copyGlistOfZobrist(GList *zobristList) 
// {
//     if (zobristList == NULL) {
//         return NULL;
//     }

//     GList *newList = NULL;
//     for (GList *current = zobristList; current != NULL; current = current->next) {
//         // Récupérer la clé et l'ajouter à la nouvelle liste sans allocation dynamique
//         unsigned long long key = GPOINTER_TO_UINT(current->data);
//         newList = g_list_append(newList, GUINT_TO_POINTER(key));
//     }

   

//     return newList;
// }

void freeGlistOfZobrist(GList *zobristList) 
{
    // g_list_free(zobristList);
    g_list_free_full(zobristList, free);
}

void printZobristList(ChessGame *game)
{
    GList *zobristList = getZobristList(game);
    if (zobristList == NULL) {
        return;
    }

    for (GList *l = zobristList; l != NULL; l = l->next) {
        // Récupérer la clé Zobrist et l'écrire dans le fichier
        unsigned long long *key = (unsigned long long *)l->data;
        fprintf(stderr, "%llu\n", *key);
    }
}
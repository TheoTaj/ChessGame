#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include <glib.h>
#include<string.h>
#include<ctype.h>

#include "fen.h"
#include "chess.h"

#define BOARD_SIZE 8


Piece pieceFromChar(char pieceChar)
{
    switch (pieceChar)
    {
    case 'K': return WHITE_KING;
    case 'Q': return WHITE_QUEEN;
    case 'B': return WHITE_BISHOP;
    case 'N': return WHITE_KNIGHT;
    case 'R': return WHITE_ROOK;
    case 'P': return WHITE_PAWN;
    case 'k': return BLACK_KING;
    case 'q': return BLACK_QUEEN;
    case 'b': return BLACK_BISHOP;
    case 'n': return BLACK_KNIGHT;
    case 'r': return BLACK_ROOK;
    case 'p': return BLACK_PAWN;
    default: return EMPTY_CASE;
    }
}

char pieceToChar(Piece piece) 
{
    switch (piece) {
        case WHITE_KING: return 'K';
        case WHITE_QUEEN: return 'Q';
        case WHITE_BISHOP: return 'B';
        case WHITE_KNIGHT: return 'N';
        case WHITE_ROOK: return 'R';
        case WHITE_PAWN: return 'P';
        case BLACK_KING: return 'k';
        case BLACK_QUEEN: return 'q';
        case BLACK_BISHOP: return 'b';
        case BLACK_KNIGHT: return 'n';
        case BLACK_ROOK: return 'r';
        case BLACK_PAWN: return 'p';
        default: return ' ';
    }
}
void printFEN(char *fen) 
{
    if (fen == NULL) {
        fprintf(stderr, "FEN is NULL\n");
        return;
    }

    fprintf(stderr, "FEN: %s\n", fen);
}

char *copyString(const char *str) 
{
    if (str == NULL) {
        return NULL;  // Si la chaîne d'entrée est NULL, on retourne NULL
    }
    
    // Alloue de la mémoire pour la copie de la chaîne, y compris le caractère nul ('\0')
    char *copy = malloc(strlen(str) + 1);
    
    if (copy == NULL) {
        return NULL;  // Si malloc échoue, retourne NULL
    }
    
    // Copie la chaîne source dans la chaîne allouée
    strcpy(copy, str);
    
    return copy;
}

char getColumnLetter(int col) 
{
    if (col >= 0 && col <= 7) {
        return 'a' + col;
    } else {
        fprintf(stderr, "error in getColumnLetter\n");
        return '?'; // Valeur de retour pour des colonnes invalides
    }
}

int getColumnIndex(char letter) 
{
    if (letter >= 'a' && letter <= 'h') {
        return letter - 'a';
    } else {
        fprintf(stderr, "error in getColumnIndex\n");
        return -1; // Valeur de retour pour des lettres invalides
    }
}

int convertClassicRowToInternal(char c) 
{
    int classicRow = charToInt(c);
    return 8 - classicRow;
}
int charToInt(char c) 
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else {
        fprintf(stderr, "Erreur : le caractère fourni n'est pas un chiffre.\n");
        return -1; // Valeur d'erreur si le caractère n'est pas un chiffre
    }
}
char *intToString(int num)
{
    // Estime la longueur nécessaire pour l'entier en ajoutant 1 pour le '\0'
    int length = snprintf(NULL, 0, "%d", num) + 1;

    // Alloue la mémoire pour la chaîne
    char *str = (char *)malloc(length * sizeof(char));

    if (str != NULL) {
        // Formate l'entier dans la chaîne allouée
        snprintf(str, length, "%d", num);
    }

    return str;  // Retourne la chaîne allouée dynamiquement
}

char* createFEN(ChessGame *game) 
{
    char fen[100]; // Tableau temporaire pour construire la FEN
    int pos = 0;

    // Construction de la FEN
    for (int row = 0; row < BOARD_SIZE; row++) {
        int empty_count = 0;
        for (int col = 0; col < BOARD_SIZE; col++) {
            Piece piece = GetPieceOnCase(game, row, col);
            if (piece == EMPTY_CASE) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    fen[pos++] = '0' + empty_count; // Ajouter le nombre de cases vides
                    empty_count = 0;
                }
                fen[pos++] = pieceToChar(piece); // Ajouter la pièce
            }
        }
        if (empty_count > 0) {
            fen[pos++] = '0' + empty_count;
        }
        if (row < BOARD_SIZE - 1) {
            fen[pos++] = '/';
        }
    }

    // Ajouter le joueur courant
    fen[pos++] = ' ';
    fen[pos++] = (GetCurrentPlayer(game) == PLAYER_WHITE) ? 'w' : 'b';
    
    // Roque
    fen[pos++] = ' ';

    bool jeDoisMettreUnTiret = true;
    
    if(getWK(game))
    {
        fen[pos++] = 'K';
        jeDoisMettreUnTiret = false;
    }
    if(getWQ(game))
    {
        fen[pos++] = 'Q';
        jeDoisMettreUnTiret = false;
    }
    if(getBK(game))
    {
        fen[pos++] = 'k';
        jeDoisMettreUnTiret = false;
    }
    if(getBQ(game))
    {
        fen[pos++] = 'q';
        jeDoisMettreUnTiret = false;
    }

    if(jeDoisMettreUnTiret == true)
        fen[pos++] = '-';

    //PriseEnPassant

    fen[pos++] = ' ';
    bool caca = false;

    if(getEpWhiteCol(game) != -1)
    {
        int end_col = getEpWhiteCol(game);
        int start_row = 3;

        int start_col1 = end_col + 1;
        int start_col2 = end_col -1;

        bool temp = false;

        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_WHITE, start_row, start_col1, 2, end_col);
            if(PriseEnPassantValidMove(game, move1))
            {
                fen[pos++] = getColumnLetter(end_col);
                fen[pos++] = '6';
                temp = true;
                caca = true;
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8 && temp == false)
        {
            Move *move2 = CreateMove(PLAYER_WHITE, start_row, start_col2, 2, end_col);
            if(PriseEnPassantValidMove(game, move2))
            {
                fen[pos++] = getColumnLetter(end_col);
                fen[pos++] = '6';
                caca = true;
            }
            free(move2);
        }
        
    }
    if(getEpBlackCol(game) != -1)
    {
        int end_col = getEpBlackCol(game);
        int start_row = 4;

        int start_col1 = end_col + 1;
        int start_col2 = end_col -1;

        bool temp = false;

        if(start_col1 >= 0 && start_col1 < 8)
        {
            Move *move1 = CreateMove(PLAYER_BLACK, start_row, start_col1, 5, end_col);
            if(PriseEnPassantValidMove(game, move1))
            {
                fen[pos++] = getColumnLetter(end_col);
                fen[pos++] = '6';
                temp = true;
                caca = true;
            }
            free(move1);
        }
        if(start_col2 >= 0 && start_col2 < 8 && temp == false)
        {
            Move *move2 = CreateMove(PLAYER_BLACK, start_row, start_col2, 5, end_col);
            if(PriseEnPassantValidMove(game, move2))
            {
                fen[pos++] = getColumnLetter(end_col);
                fen[pos++] = '6';
                caca = true;
            }
            free(move2);
        }

    }
    if(caca == false)
    {
        fen[pos++] = '-';
    }

    //nbCoupInutile

    fen[pos++] = ' ';

    char *temp = intToString(getNbCoupInutile(game));

    int length = strlen(temp);

    for(int j = 0; j<length; j++)
    {
        fen[pos++] = temp[j];
    }

    free(temp);
    


    

    fen[pos] = '\0'; // Terminer la chaîne

    // Allouer dynamiquement de la mémoire pour la FEN
    char *fen_copy = malloc((strlen(fen) + 1) * sizeof(char));
    if (fen_copy != NULL) {
        strcpy(fen_copy, fen);
    }

    return fen_copy;
}

char* copyFen(char *str) 
{
    if (str == NULL) {
        return NULL;
    }
    char *copy = malloc(strlen(str) + 1);
    if (copy != NULL) {
        strcpy(copy, str);
    }
    return copy;
}

GList* copyGListOfFen(GList *list) 
{
    GList *new_list = NULL;
    for (GList *l = list; l != NULL; l = l->next) {
        char *data_copy = copyFen((char*)l->data); // Dupliquer l'élément
        new_list = g_list_append(new_list, data_copy); // Ajouter l'élément dupliqué à la nouvelle liste
    }
    return new_list;
}

void freeGListOfFen(GList *list) 
{
    g_list_free_full(list, free); // Utiliser free pour libérer chaque élément
}

bool isPiece(char c) 
{
    // Convertir le caractère en minuscule pour simplifier la vérification
    char lower = tolower(c);

    // Vérifier si le caractère est l'une des pièces d'échecs valides
    if (lower == 'p' || lower == 'r' || lower == 'n' || 
        lower == 'b' || lower == 'q' || lower == 'k') {
        return true;
    }

    // Si ce n'est pas une pièce d'échecs, retourner false
    return false;
}



char *getPartOfFen(char *fen) 
{
    if (fen == NULL) {
        return NULL;  // Retourne NULL si la chaîne fen est NULL
    }

    int spaceCount = 0;
    int len = 0;

    // Compter jusqu'à quatre espaces
    while (fen[len] != '\0' && spaceCount < 4) {
        if (fen[len] == ' ') {
            spaceCount++;
        }
        len++;
    }

    // Allouer de la mémoire pour la nouvelle chaîne (inclut '\0')
    char *result = malloc(sizeof(char) * (len + 1));
    if (result == NULL) {
        return NULL;  // Si malloc échoue, retourne NULL
    }

    // Copier la partie correspondante de fen
    strncpy(result, fen, len);
    result[len] = '\0';  // Ajouter le caractère nul à la fin

    return result;
}
#ifndef FEN_H
#define FEN_H

#include <stdbool.h>
#include <glib.h>

#include "chess.h"

// Conversion entre les pièces et leurs représentations en caractères

Piece pieceFromChar(char pieceChar);
char pieceToChar(Piece piece);

// Fonctions pour gérer la chaîne de caractères de la FEN
char *createFEN(ChessGame *game);
char *getPartOfFen(char *fen);
void printFEN(char *fen);
char *copyString(const char *str);
char *intToString(int num);

// Fonctions pour gérer les colonnes de l'échiquier
char getColumnLetter(int col);
int getColumnIndex(char letter);

// Fonctions pour manipuler la liste de FEN
char* copyFen(char *str);
GList* copyGListOfFen(GList *list);
void freeGListOfFen(GList *list);
int convertClassicRowToInternal(char c);
int charToInt(char c) ;

// Fonction pour vérifier si un caractère correspond à une pièce
bool isPiece(char c);

#endif // FEN_H

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>

#include "chess.h"



#define NUM_PIECE_TYPES 14 // Définissez la constante NUM_PIECE_TYPES ici
#define NUM_SOUNDS 5


void loadPieceTextures(SDL_Renderer *renderer, SDL_Texture *pieceTextures[NUM_PIECE_TYPES]);





void renderChessBoard(SDL_Renderer *renderer, ChessGame *game, SDL_Texture *pieceTextures[NUM_PIECE_TYPES], Player user);
void renderPromotionSelection(SDL_Renderer *renderer, SDL_Texture *pieceTextures[NUM_PIECE_TYPES], Player user);
int handlePromotionClick(SDL_Event *event);
void highlightLastMove(SDL_Renderer *renderer, SDL_Texture *highlightTexture, int start_row, int start_col, int end_row, int end_col, Player user);

void highlightSelectedSquare(SDL_Renderer *renderer, int row, int col, SDL_Texture *highlightTexture, bool selected);



int getPieceTextureIndex(Piece piece);

void loadMP3Files(Mix_Music *sounds[NUM_SOUNDS]);
void playSound(Mix_Music *sounds[NUM_SOUNDS], ChessGame *game, Move *move);


#endif
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "display.h"



#define WINDOW_WIDTH 1150
#define OFFSET_WIDTH 150
#define WINDOW_HEIGHT 1000


#define BOARD_SIZE 8
#define SQUARE_SIZE 125






//new
// void renderChessBoard(SDL_Renderer *renderer, ChessGame *game, SDL_Texture *pieceTextures[NUM_PIECE_TYPES], Player user)
// {
//     // Dessiner l'échiquier
//     SDL_SetRenderDrawColor(renderer, 252, 228, 208, 190); // Combo tah lichess
//     SDL_RenderClear(renderer);

//     // Dessiner les cases noires et blanches
//     for (int row = 0; row < BOARD_SIZE; ++row) {
//         for (int col = 0; col < BOARD_SIZE; ++col) {
//             if ((row + col) % 2 != 0) {
//                 SDL_Rect rect = {OFFSET_WIDTH + col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
//                 SDL_SetRenderDrawColor(renderer, 190, 143, 104, 255); // Combo tah lichess
//                 SDL_RenderFillRect(renderer, &rect);
//             }
//         }
//     }

//     // Dessiner les pièces
//     for (int row = 0; row < BOARD_SIZE; row++) {
//         for (int col = 0; col < BOARD_SIZE; col++) {
//             Piece piece = GetPieceOnCase(game, row, col);
//             if (piece != EMPTY_CASE) {
//                 int pieceIndex = getPieceTextureIndex(piece);

//                 int drawRow, drawCol;
//                 if (user == PLAYER_WHITE) {
//                     drawRow = row;
//                     drawCol = col;
//                 } else {
//                     drawRow = 7 - row;
//                     drawCol = 7 - col;
//                 }

//                 SDL_Rect destRect = {OFFSET_WIDTH + drawCol * SQUARE_SIZE, drawRow * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
//                 SDL_RenderCopy(renderer, pieceTextures[pieceIndex], NULL, &destRect);
//             }
//         }
//     }
// }

void renderChessBoard(SDL_Renderer *renderer, ChessGame *game, SDL_Texture *pieceTextures[NUM_PIECE_TYPES], Player user)
{
    

    // Dessiner l'échiquier
    SDL_SetRenderDrawColor(renderer, 252, 228, 208, 255); // Combo tah lichess
    SDL_RenderClear(renderer);
    
    // Dessiner l'offset bleu
    SDL_SetRenderDrawColor(renderer, 48, 48, 48, 255); // Couleur bleue (RGB : 0, 0, 255)
    SDL_Rect offsetRect = {0, 0, OFFSET_WIDTH, WINDOW_HEIGHT}; // Le rectangle bleu occupe toute la hauteur à gauche
    SDL_RenderFillRect(renderer, &offsetRect); // Dessiner le rectangle bleu

    SDL_SetRenderDrawColor(renderer, 48, 48, 48, 255);
    SDL_Rect offsetRect2 = {OFFSET_WIDTH + SQUARE_SIZE * 8, 0, OFFSET_WIDTH, WINDOW_HEIGHT}; // Le rectangle bleu occupe toute la hauteur à gauche
    SDL_RenderFillRect(renderer, &offsetRect2); // Dessiner le rectangle bleu

    // Dessiner les cases noires et blanches
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if ((row + col) % 2 != 0) {
                SDL_Rect rect = {OFFSET_WIDTH + col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                SDL_SetRenderDrawColor(renderer, 190, 143, 104, 255); // Combo tah lichess
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // Dessiner les pièces
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            Piece piece = GetPieceOnCase(game, row, col);
            if (piece != EMPTY_CASE) {
                int pieceIndex = getPieceTextureIndex(piece);

                int drawRow, drawCol;
                if (user == PLAYER_WHITE) {
                    drawRow = row;
                    drawCol = col;
                } else {
                    drawRow = 7 - row;
                    drawCol = 7 - col;
                }

                SDL_Rect destRect = {OFFSET_WIDTH + drawCol * SQUARE_SIZE, drawRow * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                SDL_RenderCopy(renderer, pieceTextures[pieceIndex], NULL, &destRect);
            }
        }
    }
}


//new

void renderPromotionSelection(SDL_Renderer *renderer, SDL_Texture *pieceTextures[NUM_PIECE_TYPES], Player user)
{
    int promotionPieceSize = SQUARE_SIZE;
    SDL_Rect promotionRect[4];

    // Afficher la dame, tour, fou et cavalier dans la zone de promotion (à gauche)
    for (int i = 0; i < 4; i++) {
        promotionRect[i].x = 0;  // Position fixe à gauche
        promotionRect[i].y = i * promotionPieceSize;  // Une case sous l'autre
        promotionRect[i].w = promotionPieceSize;
        promotionRect[i].h = promotionPieceSize;

        int pieceIndex;
        switch (i) {
            case 0: pieceIndex = (user == PLAYER_WHITE) ? 1 : 7; break;  // Dame
            case 1: pieceIndex = (user == PLAYER_WHITE) ? 4 : 10; break; // Tour
            case 2: pieceIndex = (user == PLAYER_WHITE) ? 2 : 8; break;  // Fou
            case 3: pieceIndex = (user == PLAYER_WHITE) ? 3 : 9; break;  // Cavalier
        }

        SDL_RenderCopy(renderer, pieceTextures[pieceIndex], NULL, &promotionRect[i]);
    }

    SDL_RenderPresent(renderer);
}

//new

int handlePromotionClick(SDL_Event *event)
{
    int promotionPieceSize = SQUARE_SIZE;

    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int x = event->button.x;
        int y = event->button.y;

        if (x < OFFSET_WIDTH) {  // Le clic doit être dans la zone de promotion à gauche
            int clickedIndex = y / promotionPieceSize;  // Calculer l'index de la pièce cliquée

            switch (clickedIndex) {
                case 0: return 1; // Dame
                case 1: return 2; // Tour
                case 2: return 3; // Fou
                case 3: return 4; // Cavalier
                default: return -1;  // Aucun clic valide
            }
        }
    }

    return -1;  // Si aucun clic valide
}


void loadPieceTextures(SDL_Renderer *renderer, SDL_Texture *pieceTextures[NUM_PIECE_TYPES]) 
{
    // Chemins vers les images des pièces
   char *piecePaths[NUM_PIECE_TYPES] = {
    "./images/king_white.bmp",
    "./images/queen_white.bmp",
    "./images/bishop_white.bmp",
    "./images/knight_white.bmp",
    "./images/rook_white.bmp",
    "./images/pawn_white.bmp",
    "./images/king_black.bmp",
    "./images/queen_black.bmp",
    "./images/bishop_black.bmp",
    "./images/knight_black.bmp",
    "./images/rook_black.bmp",
    "./images/pawn_black.bmp",
    "./images/lastMove.bmp",
    "./images/outline.bmp"};


    // Chargement des images des pièces
    for (int i = 0; i < NUM_PIECE_TYPES ; i++) {
        SDL_Surface *surface = SDL_LoadBMP(piecePaths[i]);
        if (!surface) {
            printf("Erreur lors du chargement de l'image : %s\n", SDL_GetError());
            // Gérer l'erreur de chargement de l'image
        }

        pieceTextures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!pieceTextures[i]) {
            printf("Erreur lors de la création de la texture : %s\n", SDL_GetError());
            // Gérer l'erreur de création de texture
        }
    }
}

void loadMP3Files(Mix_Music *sounds[NUM_SOUNDS])
{
    char *soundsPaths[NUM_SOUNDS] = {
        "./sounds/classique.mp3",
        "./sounds/capture.mp3",
        "./sounds/castle.mp3",
        "./sounds/check.mp3",
        "./sounds/promote.mp3"
    };

    for(int i = 0; i < NUM_SOUNDS; i++)
    {
        sounds[i] = Mix_LoadMUS(soundsPaths[i]);
        if(! sounds[i])
        {
            fprintf(stderr, "Failed to load MP3: %s\n", Mix_GetError());
            return ;
        }
    }
    
}



int getPieceTextureIndex(Piece piece)
{
    switch(piece) {
        case WHITE_KING:
            return 0;
        case WHITE_QUEEN:
            return 1;
        case WHITE_BISHOP:
            return 2;
        case WHITE_KNIGHT:
            return 3;
        case WHITE_ROOK:
            return 4;
        case WHITE_PAWN:
            return 5;
        case BLACK_KING:
            return 6;
        case BLACK_QUEEN:
            return 7;
        case BLACK_BISHOP:
            return 8;
        case BLACK_KNIGHT:
            return 9;
        case BLACK_ROOK:
            return 10;
        case BLACK_PAWN:
            return 11;
        default:
            return -1; // Retourne -1 en cas de pièce invalide
    }
}


void highlightSelectedSquare(SDL_Renderer *renderer, int row, int col, SDL_Texture *highlightTexture, bool selected) 
{
    // Afficher la case sélectionnée
    if(selected)
    {
        SDL_Rect destRect = {OFFSET_WIDTH + (col * SQUARE_SIZE), row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderCopy(renderer, highlightTexture, NULL, &destRect);
    }
}

void highlightLastMove(SDL_Renderer *renderer, SDL_Texture *highlightTexture, int start_row, int start_col, int end_row, int end_col, Player user)
{
    if(end_row > 7 || end_row < 0 || end_col > 7 || end_col < 0 || start_row < 0 || start_row > 7 || start_col < 0 || start_col > 7)
        return ;
    
    if(user == PLAYER_BLACK)
    {
        start_row = 7 - start_row;
        start_col = 7 - start_col;
        end_row = 7 - end_row;
        end_col = 7 - end_col;
    }
    SDL_Rect startRect = {OFFSET_WIDTH + (start_col * SQUARE_SIZE), start_row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
    SDL_RenderCopy(renderer, highlightTexture, NULL, &startRect);


    SDL_Rect endRect = {OFFSET_WIDTH + (end_col * SQUARE_SIZE), end_row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
    SDL_RenderCopy(renderer, highlightTexture, NULL, &endRect);
}

//ajouter ici peut etre le numero du son ou un bail comme ca 
void playSound(Mix_Music *sounds[NUM_SOUNDS], ChessGame *game, Move *move)
{   
    // fprintf(stderr, "1\n");

    MoveType moveType = getMoveTypeFromMove(move);
    ChessGame *copy = copyChessGame(game);
    PlayMoveAI(copy, move);
    bool temp = GameIsCheck(copy, GetCurrentPlayer(copy));
    ChessFree(copy);
    if(temp)
    {
        if(Mix_PlayMusic(sounds[3], 0)) 
        {
            // fprintf(stderr, "2\n");
            fprintf(stderr, "Failed to play MP3: %s\n", Mix_GetError());
            return ;
        }
    }
    else if(moveType == Classique)
    {
        if(Mix_PlayMusic(sounds[0], 0)) 
        {
            // fprintf(stderr, "2\n");
            fprintf(stderr, "Failed to play MP3: %s\n", Mix_GetError());
            return ;
        }        
    }
    else if(moveType == Prise)
    {
        if(Mix_PlayMusic(sounds[1], 0)) 
        {
            // fprintf(stderr, "2\n");
            fprintf(stderr, "Failed to play MP3: %s\n", Mix_GetError());
            return ;
        }
    }
    else if(moveType == Roque)
    {
        if(Mix_PlayMusic(sounds[2], 0)) 
        {
            // fprintf(stderr, "2\n");
            fprintf(stderr, "Failed to play MP3: %s\n", Mix_GetError());
            return ;
        }
    }
    else if(moveType == Promotion)
    {
        if(Mix_PlayMusic(sounds[4], 0)) 
        {
            // fprintf(stderr, "2\n");
            fprintf(stderr, "Failed to play MP3: %s\n", Mix_GetError());
            return ;
        }
    }
    

    SDL_Delay(90);
    // fprintf(stderr, "3\n");
}
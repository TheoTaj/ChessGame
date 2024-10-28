// #define WINDOW_WIDTH 800
// #define WINDOW_HEIGHT 800


// #define BOARD_SIZE 8
// #define SQUARE_SIZE 100

#define WINDOW_WIDTH 1150
#define OFFSET_WIDTH 150
#define WINDOW_HEIGHT 1000

#define BOARD_SIZE 8
#define SQUARE_SIZE 125


#include <stdio.h>
#include <stdbool.h>
#include<stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <unistd.h> 
#include <glib.h>
#include<time.h>

#include "display.h"
#include "IA.h"
#include "Zobrist.h"
#include "fen.h"
#include "GMmove.h"

void play_game(int ai_enabled, char *fen);



void play_game(int ai_enabled, char *fen)
{
    Player AIcolor = PLAYER_EMPTY;
    Player PlayerColor = PLAYER_EMPTY;
    if(ai_enabled == 1)
    {
        AIcolor = PLAYER_WHITE;
        PlayerColor = PLAYER_BLACK;
    }
    else if(ai_enabled == -1)
    {
        AIcolor = PLAYER_BLACK;
        PlayerColor = PLAYER_WHITE;
    }

    if(AIcolor != PLAYER_EMPTY) //AI play
    {
        if(AIcolor == PLAYER_WHITE)
        {
            printf("AI play white\n");
        }
        else
        {
            printf("AI play black\n");
        }       
        
        
        ChessGame *game = createChessGameFromFEN(fen);
        if (!game) {
            printf("Erreur lors de la création du jeu.\n");
            return ;
        }


        

        // Initialisation de SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("Erreur lors de l'initialisation de SDL : %s\n", SDL_GetError());
            return ;
        }

        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            printf("SDL Error: %s\n", SDL_GetError());
            return ;
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            fprintf(stderr, "SDL_mixer Error: %s\n", Mix_GetError());
            return ;
        }

        Mix_Music *sounds[NUM_SOUNDS];
        loadMP3Files(sounds);
    

        // Création de la fenêtre SDL
        SDL_Window *window = SDL_CreateWindow("Jeu d'échecs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH + OFFSET_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            printf("Erreur lors de la création de la fenêtre SDL : %s\n", SDL_GetError());
            return ;
        }

        // Création du rendu SDL
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            printf("Erreur lors de la création du rendu SDL : %s\n", SDL_GetError());
            return ;
        }

        SDL_Texture *pieceTextures[NUM_PIECE_TYPES];
        loadPieceTextures(renderer, pieceTextures);

        bool quit = false;
    

        int *tab = malloc(4 * sizeof(int)); // Allouer dynamiquement de la mémoire pour le tableau
        if (!tab) {
            printf("Erreur lors de l'allocation mémoire.\n");
            return ;
        }

        int clickCount = 0;
        
        bool selected = false;
        int row, col;
        
        

        
        bool playerJustPlayed = false;
        bool endGame = false;

        int hstart_row, hstart_col, hend_row, hend_col;

        hstart_row = -1;
        hend_row = -1;
        hend_col = -1;
        hstart_col = -1;


        
        

        double eval = getEval(game);
        fprintf(stderr, "Evaluation = %lf\n", eval);


        unsigned long long currentZobristKey = getCurrentZobristKey(game);
        fprintf(stderr, "\ncurrentZobristKey = %llu\n", currentZobristKey);
        if(getEndGame(game))
        {
            fprintf(stderr, "EndGame\n\n");
        }
        else
            fprintf(stderr, "StartGame\n\n");

        
        while (!quit) // Boucle du jeu
        {
            

            playerJustPlayed = false;

            

            
            

            
            SDL_Event event;
            while (SDL_PollEvent(&event)) 
            {
                // if (event.type == SDL_QUIT) 
                // {
                //     fprintf(stderr, "SQL_QUIT\n");
                //     quit = true;
                // } 
                if (event.type == SDL_KEYDOWN) 
                {
                    if(event.key.keysym.sym == SDLK_m)
                    {
                        quit = true;
                        fprintf(stderr, "clicked on 'M'\n");
                    }
                        
                    else if(event.key.keysym.sym == SDLK_LEFT)
                    {
                        UndoMove2(game);
                        
                    }

                } 
                else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && GetCurrentPlayer(game) == PlayerColor && endGame == false) 
                {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Convertir les coordonnées de la souris en indices de la case de l'échiquier
                    row = mouseY / SQUARE_SIZE;
                    col = (mouseX - OFFSET_WIDTH) / SQUARE_SIZE;

                    if (clickCount == 0) 
                    {
                        // Premier clic, enregistrer les coordonnées
                        tab[0] = row;
                        tab[1] = col;
                        clickCount++;

                        selected = true;
                    
                    } 
                    else if (clickCount == 1) 
                    {
                        // Deuxième clic, enregistrer les coordonnées
                        
                        tab[2] = row;
                        tab[3] = col;
                        clickCount++;

                        
                        selected = false;

                        // Traiter les clics et réinitialiser le compteur
                        Move *playMove = convertMouseClicksToMove(tab, game, PlayerColor);


                        
                        
                        Piece piece = GetPieceOnCase(game, getStartRowFromMove(playMove), getStartColFromMove(playMove)); //ici appeller getStartRowFromMove peut sembler inutile mais c'est important par rapport à l'affichage quand on joue les noirs
                        if( (GetCurrentPlayer(game) == PLAYER_WHITE && piece == WHITE_PAWN && getEndRowFromMove(playMove) == 0) || (GetCurrentPlayer(game) == PLAYER_BLACK && getEndRowFromMove(playMove) == 7 && piece == BLACK_PAWN))
                        {
                            renderPromotionSelection(renderer, pieceTextures, GetCurrentPlayer(game));

                            int response = -1;
                            SDL_Event event;
                            while (response == -1)
                            {
                                while (SDL_PollEvent(&event)) 
                                {
                                    if (event.type == SDL_QUIT) {
                                        quit = true;
                                        response = -2;  // Quitter si l'utilisateur ferme la fenêtre
                                        break ;
                                    }
                                    else if (event.type == SDL_KEYDOWN) 
                                    {
                                        if(event.key.keysym.sym == SDLK_m)
                                        {
                                            quit = true;
                                            response = -2;
                                            break;
                                        }
                                    }
                                    else
                                        response = handlePromotionClick(&event);
                                }
                            }
                            newResponse(playMove, response);
                                                                
                        }

                        bool wasValidMove = false;
                        if(GameIsValidMove(game, playMove) || PriseEnPassantValidMove(game, playMove) || RoqueIsValidMove(game, playMove))
                        {
                            playSound(sounds, game, playMove);
                            PlayMove(game, playMove, true);
                            wasValidMove = true;
                            
                        }
                       
                    


                       

                        if(wasValidMove)
                        {
                            hstart_row = getStartRowFromMove(playMove);
                            hend_row = getEndRowFromMove(playMove);
                            hstart_col = getStartColFromMove(playMove);
                            hend_col = getEndColFromMove(playMove);
                        }
                        
                        free(playMove);
                        
                        clickCount = 0;
                        for(int i =0; i<4; i++)
                        {
                            tab[i]=0;
                        }

                        
                        

                        

                        double eval = getEval(game);
                        fprintf(stderr, "Evaluation = %lf\n", eval);

                        unsigned long long currentZobristKey = getCurrentZobristKey(game);
                        fprintf(stderr, "FromUPDTcurrentZobristKey = %llu\n", currentZobristKey);
                        unsigned long long currentZobristKey2 = getZobristKey(game);
                        fprintf(stderr, "FromGETcurrentZobristKey = %llu\n\n", currentZobristKey2);

                        if (ChessGameIsOver(game, true) ) 
                        {
                            printf("Fin de la partie\n");
                            printf("Vous pouvez fermer la fenetre\n");
                            endGame = true;
                            
                            
                        }
                        
                        
                    

                    }

                    playerJustPlayed = true;
                }
                else if(GetCurrentPlayer(game) == AIcolor && playerJustPlayed == false && endGame == false)
                {
                    nbPositions = 0;
                    nbClassique = 0;
                    nbPromotion = 0;
                    nbRoque = 0;
                    nbPriseEnPassant = 0;
                    nbPrise = 0;
                    totalLength = 0;
                    nbZobrist = 0;
                    nbZobrist2 = 0;
                    clock_t start_time = clock();
                    Move *playMove = IAGetMove(game); 
                    clock_t end_time = clock();
                    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;

                    fprintf(stderr, "nbPositions = %d à la depth = %d in %lf ms \n", nbPositions, depth, time_spent);
                    // fprintf(stderr, "nbClassique = %d, nbPromotion = %d, nbRoque = %d, nbPriseEnPassant = %d\n", nbClassique, nbPromotion, nbRoque, nbPriseEnPassant);
                    // fprintf(stderr, "totalLength = %d\n", totalLength);
                    fprintf(stderr, "nbZobrist : %d\n", nbZobrist);
                    fprintf(stderr, "nbZobrist2 : %d\n", nbZobrist2);

                    int nbTotal = nbPositions + nbZobrist + nbZobrist2;

                    fprintf(stderr, "nbTotal = %d\n", nbTotal);

                    bool wasValidMove = false;
                    playSound(sounds, game, playMove);
                    PlayMove(game, playMove, false);
                    wasValidMove = true;


                

                    if(wasValidMove)
                    {
                        hstart_row = getStartRowFromMove(playMove);
                        hend_row = getEndRowFromMove(playMove);
                        hstart_col = getStartColFromMove(playMove);
                        hend_col = getEndColFromMove(playMove);
                    }
                    
                    FreeMove(playMove);
                    
                    
                    clickCount = 0;
                   

                    
                    

                    
                
                    double eval = getEval(game);
                    fprintf(stderr, "Evaluation = %lf\n", eval);

                    unsigned long long currentZobristKey = getCurrentZobristKey(game);
                    fprintf(stderr, "FromUPDTcurrentZobristKey = %llu\n", currentZobristKey);
                    unsigned long long currentZobristKey2 = getZobristKey(game);
                    fprintf(stderr, "FromGetZobristKey = %llu\n", currentZobristKey2);
                    if(getEndGame(game))
                    {
                        fprintf(stderr, "EndGame\n\n");
                    }
                    else
                        fprintf(stderr, "StartGame\n\n");


                    if (ChessGameIsOver(game, true) ) 
                    {
                        printf("Fin de la partie\n");
                        printf("Vous pouvez fermer la fenetre\n");
                        endGame = true;
                        
                        
                    }                 


                }
            }

            // Affichage du jeu d'échecs
            renderChessBoard(renderer, game, pieceTextures, PlayerColor);
            
            highlightLastMove(renderer, pieceTextures[NUM_PIECE_TYPES-2], hstart_row, hstart_col, hend_row, hend_col, PlayerColor);
            highlightSelectedSquare(renderer, row, col, pieceTextures[NUM_PIECE_TYPES-1], selected);
            


            // Rafraîchissement de la fenêtre
            SDL_RenderPresent(renderer);

            // Attendre un court instant pour limiter la vitesse de la boucle
            SDL_Delay(100);
            // fprintf(stderr, "1\n");

            
        }

        

       

        // Libération des ressources SDL
        fprintf(stderr, "libération\n");

        for (int i = 0; i < NUM_PIECE_TYPES; i++) {
            if (pieceTextures[i]) {
                SDL_DestroyTexture(pieceTextures[i]);
            }
        }
        for(int i  = 0; i < NUM_SOUNDS; i++)
        {
            if(sounds[i])
            {
                Mix_FreeMusic(sounds[i]);
            }
        }
        Mix_CloseAudio();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        // Libération des ressources du jeu
        ChessFree(game);
        free(tab);
        return ;
    }
    

    else                //AI doesn't play
    {
        printf("AI doesn't play\n");
        ChessGame *game = createChessGameFromFEN(fen);
        if (!game) {
            fprintf(stderr, "Erreur lors de la création du jeu.\n");
            return ;
        }

        
       

        // Initialisation de SDL


        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            fprintf(stderr, "Erreur lors de l'initialisation de SDL : %s\n", SDL_GetError());
            return ;
        }

        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            printf("SDL Error: %s\n", SDL_GetError());
            return ;
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            fprintf(stderr, "SDL_mixer Error: %s\n", Mix_GetError());
            return ;
        }

        Mix_Music *sounds[NUM_SOUNDS];
        loadMP3Files(sounds);
    

        // Création de la fenêtre SDL

        SDL_Window *window = SDL_CreateWindow("Jeu d'échecs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH + OFFSET_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

        if (!window) {
            fprintf(stderr, "Erreur lors de la création de la fenêtre SDL : %s\n", SDL_GetError());
            return ;
        }

        // // Création du rendu SDL
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            fprintf(stderr, "Erreur lors de la création du rendu SDL : %s\n", SDL_GetError());
            return ;
        }

        SDL_Texture *pieceTextures[NUM_PIECE_TYPES];
        loadPieceTextures(renderer, pieceTextures);

        // Boucle principale
        bool quit = false;
    

        int *tab = malloc(4 * sizeof(int)); // Allouer dynamiquement de la mémoire pour le tableau
        if (!tab) {
            printf("Erreur lors de l'allocation mémoire.\n");
            return ;
        }

        int clickCount = 0;
        bool selected = false;
        int row, col;
        


        bool endGame = false;

        
        double eval = getEval(game);
        fprintf(stderr, "Evaluation = %lf\n", eval);

        unsigned long long currentZobristKey = getCurrentZobristKey(game);
        fprintf(stderr, "\ncurrentZobristKey = %llu\n\n", currentZobristKey);
                    

        int hstart_row, hstart_col, hend_row, hend_col;

        hstart_row = -1;
        hend_row = -1;
        hend_col = -1;
        hstart_col = -1;


        
        while (!quit) // Boucle du jeu
        {
                      

            
            // Move *playMove = NULL;
            SDL_Event event;
            while (SDL_PollEvent(&event)) 
            {
                if (event.type == SDL_QUIT) 
                {
                    quit = true;
                } 
                else if (event.type == SDL_KEYDOWN) 
                {
                    if(event.key.keysym.sym == SDLK_m)
                        quit = true;
                    else if(event.key.keysym.sym == SDLK_LEFT)
                    {
                        UndoMove(game);
                        
                    }

                } 
                else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && endGame == false)  
                {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Convertir les coordonnées de la souris en indices de la case de l'échiquier
                    row = mouseY / SQUARE_SIZE;
                    col = (mouseX - OFFSET_WIDTH) / SQUARE_SIZE;

                    if (clickCount == 0) 
                    {
                        // Premier clic, enregistrer les coordonnées
                        tab[0] = row;
                        tab[1] = col;
                        clickCount++;

                        selected = true;
                    
                    } 
                    else if (clickCount == 1) 
                    {
                        // Deuxième clic, enregistrer les coordonnées
                        
                        tab[2] = row;
                        tab[3] = col;
                        clickCount++;

                        
                        selected = false;

                        // Traiter les clics et réinitialiser le compteur
                        Move *playMove = convertMouseClicksToMove(tab, game, PLAYER_WHITE);

                        fprintf(stderr, "MoveType = %d\n", getMoveTypeFromMove(playMove));


                        
                        
                        
                        
                        Piece piece = GetPieceOnCase(game, tab[0], tab[1]);


                        //Pormotions
                        if( (GetCurrentPlayer(game) == PLAYER_WHITE && piece == WHITE_PAWN && getEndRowFromMove(playMove) == 0) || (GetCurrentPlayer(game) == PLAYER_BLACK && getEndRowFromMove(playMove) == 7 && piece == BLACK_PAWN))
                        {

                            renderPromotionSelection(renderer, pieceTextures, GetCurrentPlayer(game));

                            int response = -1;
                            SDL_Event event;
                            while (response == -1)
                            {
                                while (SDL_PollEvent(&event)) 
                                {
                                    if (event.type == SDL_QUIT) {
                                        quit = true;
                                        response = -2;  // Quitter si l'utilisateur ferme la fenêtre
                                        break ;
                                    }
                                    else if (event.type == SDL_KEYDOWN) 
                                    {
                                        if(event.key.keysym.sym == SDLK_m)
                                        {
                                            quit = true;
                                            response = -2;
                                            break;
                                        }
                                    }
                                    else
                                        response = handlePromotionClick(&event);
                                }
                            }
                            newResponse(playMove, response);
                                                                
                        }
                        
                        
                        bool wasValidMove = false;


                        if(GameIsValidMove(game, playMove) || PriseEnPassantValidMove(game, playMove) || RoqueIsValidMove(game, playMove))
                        {
                            wasValidMove = true;
                            playSound(sounds, game, playMove);
                            PlayMove(game, playMove, true);
                            

                        }

                        if(wasValidMove)
                        {
                            hstart_row = getStartRowFromMove(playMove);
                            hend_row = getEndRowFromMove(playMove);
                            hstart_col = getStartColFromMove(playMove);
                            hend_col = getEndColFromMove(playMove);
                        }
                        FreeMove(playMove);


                        
                        
                        clickCount = 0;
                        for(int i =0; i<4; i++)
                        {
                            tab[i]=0;
                        }

                        
                        
                        double eval = getEval(game);
                        fprintf(stderr, "Evaluation = %lf\n", eval);
                        unsigned long long currentZobristKey = getCurrentZobristKey(game);
                        fprintf(stderr, "\ncurrentZobristKey = %llu\n\n", currentZobristKey);
                        unsigned long long currentZobristKey2 = getZobristKey(game);
                        fprintf(stderr, "\nFromGETZobristKey = %llu\n\n", currentZobristKey2);

                        // fprintf(stderr, "%s", g_list_last(getFenList(game))->data);

                        if (ChessGameIsOver(game, true)) 
                        {
                            printf("Fin de la partie\n");
                            printf("Vous pouvez fermer la fenetre\n");
                            endGame = true;

                            
                        }

                    }
                }
                

            }

            // Affichage du jeu d'échecs
            
            renderChessBoard(renderer, game, pieceTextures, PLAYER_WHITE);
            highlightLastMove(renderer, pieceTextures[NUM_PIECE_TYPES-2], hstart_row, hstart_col, hend_row, hend_col, PLAYER_WHITE);
            highlightSelectedSquare(renderer, row, col, pieceTextures[NUM_PIECE_TYPES-1], selected);
           
            
            


            // Rafraîchissement de la fenêtre
            SDL_RenderPresent(renderer);

            // Attendre un court instant pour limiter la vitesse de la boucle
            SDL_Delay(100);

            
        }

        

        

        // Libération des ressources SDL

        
        fprintf(stderr, "libération\n");
        for (int i = 0; i < NUM_PIECE_TYPES; i++) {
            if (pieceTextures[i]) {
                SDL_DestroyTexture(pieceTextures[i]);
            }
        }
        for (int i = 0; i < NUM_SOUNDS; i++)
        {
            if(sounds[i])
            {
                Mix_FreeMusic(sounds[i]);
            }
        }
        Mix_CloseAudio();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_VideoQuit();
        SDL_Quit();

        // Libération des ressources du jeu
        
        ChessFree(game);
        free(tab);
        return ;
    
    }

}



int main(int argc, char *argv[]) 
{
    // 1 for AI play WHITE, 0 AI doesn't play, -1 AI play BLACK
    int ai_enabled = 0; 
    char *fen;

    ZobristInit();
    initTranspositionTable();
    initGmTable();
    initGmMove("opening.txt");

    if (argc > 1 && strcmp(argv[1], "--aiW") == 0) 
    {
        ai_enabled = 1;
    }
    else if (argc > 1 && strcmp(argv[1], "--aiB") == 0) 
    {
        ai_enabled = -1;
    }

    if (argc > 2) 
    {
        fen = argv[2]; // Use the provided FEN string if available
    }

    
    play_game(ai_enabled, fen);
    

    //bloc pour compter les noeuds________

    // int profondeur = 4;
    // ChessGame *game = createChessGameFromFEN(fen);
    // clock_t start_time = clock();
    // int nbPositions = countPosition(game, profondeur);
    // clock_t end_time = clock();
    // double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;
    // fprintf(stderr, "\nNbPositions = %d à la depth = %d in %lf ms \n\n", nbPositions, profondeur, time_spent);
    // ChessFree(game);



    freeTranspositionTable();
    // fprintf(stderr, "1\n");
    freeGmTable();
    // fprintf(stderr, "2\n");



    

    fprintf(stderr, "Fin du programme\n");
   
    return 0;
}
#include "GMmove.h"
#include "chess.h" 
#include "Zobrist.h"

#define BOARD_SIZE 8

GMmove *createGMmove(unsigned long long key, int nbMove, char *moves, int *frequencies)
{
    GMmove *move = (GMmove *)malloc(sizeof(GMmove));
    if(move == NULL)
    {
        fprintf(stderr, "Error in createGMmove\n");
        return NULL;
    }   

    move->key = key;
    move->nbMove = nbMove;

    move->moveTab = (Move **)malloc(nbMove * sizeof(Move *));
    if(move->moveTab == NULL)
    {
        free(move);
        fprintf(stderr, "Error in createGMmove2\n");
        return NULL;
    }

    for(int i = 0; i<nbMove; i++)
    {
        char *temp = malloc(sizeof(char) * 5);
        for(int j = 0; j<4; j++)
        {
            temp[j] = moves[(i*4) + j];
        }
        temp[4] = '\0';
        int frequency = frequencies[i];
        move->moveTab[i] = convertCharInMove(temp, frequency);
        free(temp);
    }
    
    return move;
}

unsigned long long getKeyFromGMmove(GMmove *move)
{
    return move->key;
}

int getNbMoveFromGMmove(GMmove *move)
{
    return move->nbMove;
}



void freeGMmove(GMmove *move)
{
    int nbMove = move->nbMove;

    for(int i = 0; i<nbMove; i++)
    {
        free(move->moveTab[i]);
    }

    free(move->moveTab);
    free(move);
    return ;
}

Move *convertCharInMove(char *move, int frequency)
{
    int start_row, start_col, end_row, end_col;

    start_col = getColumnIndex(move[0]);
    start_row = convertClassicRowToInternal(move[1]);
    end_col = getColumnIndex(move[2]);
    end_row = convertClassicRowToInternal(move[3]);

    if(start_row < 0 || start_row >= BOARD_SIZE || start_col < 0 || start_col >= BOARD_SIZE || end_row < 0 || end_row >= BOARD_SIZE || end_col < 0 || end_col>= BOARD_SIZE)
    {
        fprintf(stderr, "Error 1 in convertCharInMove\n");
        fprintf(stderr, "move = %s\n", move);
        fprintf(stderr, "start_col = %d, start_row = %d, end_col = %d, end_row = %d\n", start_col, start_row, end_col, end_row);
        return NULL;
    }
    Move *move2 = CreateMove(PLAYER_EMPTY , start_row, start_col, end_row, end_col);
    newFrequency(move2, frequency);
    if(move == NULL)
    {
        fprintf(stderr, "Error 2 in convertCharInMove\n");
        return NULL;
    }

    return move2;
}

void replaceFenWithZobristKey(const char *inputFile, const char *outputFile) 
{
    FILE *inFile = fopen(inputFile, "r");
    FILE *outFile = fopen(outputFile, "w");

    if (inFile == NULL || outFile == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture des fichiers.\n");
        return;
    }

    char line[256];
    unsigned long long zobristKey;
    char fen[128];

    while (fgets(line, sizeof(line), inFile)) {
        if (strncmp(line, "pos ", 4) == 0) {
            sscanf(line + 4, "%[^\n]", fen);

            // Calculer la clé Zobrist à partir de la FEN
            char *temp = copyString(fen);
            zobristKey = createZobristKeyFromFen(temp);

            // Écrire la clé Zobrist dans le fichier de sortie
            fprintf(outFile, "zobrist %llu\n", zobristKey);
        } else {
            // Copier les coups avec leurs fréquences tels quels dans le fichier de sortie
            fputs(line, outFile);
        }
    }

    fclose(inFile);
    fclose(outFile);
}

unsigned long long createZobristKeyFromFen(char *fen)
{   
    ChessGame *game = createChessGameFromFEN(fen);

    unsigned long long key = getZobristKey(game);
    ChessFree(game);
    return key;
}

void initGmTable(void)
{
    gmTable = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, freeGMmove);
}

void initGmMove(const char *filename) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier %s\n", filename);
        return;
    }

    char line[256];
    unsigned long long zobristKey;
    char moves[1024]; 
    int frequencies[256]; 
    int nbMove = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "zobrist ", 8) == 0) {
            if (nbMove > 0) {
                
                GMmove *gmMove = createGMmove(zobristKey, nbMove, moves, frequencies);
                if(gmMove == NULL)
                {
                    fprintf(stderr, "Error 1 in initGmMove\n");
                }
                insertGmTable(gmMove);
               
            }

            sscanf(line + 8, "%llu", &zobristKey);
            nbMove = 0; 
        } else {
            
            char move[5]; 
            int frequency;

            sscanf(line, "%s %d", move, &frequency);

            strncpy(moves + nbMove * 4, move, 4);

            frequencies[nbMove] = frequency;

            nbMove++;
        }
    }

    if (nbMove > 0) {
        GMmove *gmMove = createGMmove(zobristKey, nbMove, moves, frequencies);
        if(gmMove == NULL)
        {
            fprintf(stderr, "Error 2 in initGmMove\n");
        }
        insertGmTable(gmMove);
    }

    fclose(file);
}

void insertGmTable(GMmove *move)
{

    unsigned long long *key = malloc(sizeof(unsigned long long ));
    *key = move->key;

    GMmove *already = (GMmove *)g_hash_table_lookup(gmTable, key);

    if(already != NULL)
    {
        fprintf(stderr, "Error in insertGmTable, GMmove already in gmTable\n");
        return ;
    }
    else
    {
        g_hash_table_insert(gmTable, key, move);
    }


}

Move *getRandGMmove(GMmove* gmMove)
{
    srand(time(NULL));
    int totalFrequency = 0;
    int nbMove = gmMove->nbMove;
    
    for (int i = 0; i < nbMove; i++) {
        totalFrequency += getFrequencyFromMove(gmMove->moveTab[i]);
    }

    int randVal = rand() % totalFrequency;

    // Sélectionner un coup en fonction de la fréquence cumulative
    int cumulativeFrequency = 0;
    for (int i = 0; i < nbMove; i++) {
        cumulativeFrequency += getFrequencyFromMove(gmMove->moveTab[i]);
        if (randVal < cumulativeFrequency) {
            return gmMove->moveTab[i];
        }
    }

    // Retourner NULL en cas d'erreur, même si cela ne devrait jamais arriver
    return NULL;
}

void freeGmTable(void)
{
    g_hash_table_destroy(gmTable);
}
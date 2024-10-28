#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<glib.h>

GHashTable *repetitionTable;


void initRepetitionTable(void)
{
    repetitionTable = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
}

void freeRepetition(Repetition *rep)
{
    free(rep);
}

unsigned long long getKeyFromRepetition(Repetition *rep)
{
    return rep->key;
}

int getCountFromRepetition(Repetition *rep)
{
    return rep->count;
}

void freeRepetitionTable(void) 
{
    g_hash_table_destroy(repetitionTable);
}

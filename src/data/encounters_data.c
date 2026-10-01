/* FILE GENERATO da tools/genera_database.py - modifica lo script, non questo file. */
#include "encounters_data.h"
#include <stddef.h>
#include <string.h>

static const HabitatData HABITAT[] = {
    { "LUNARIA", "Lunaria", 0, 0, { { 0, 0 } }, 0 },
    { "ROUTE1", "Route 1", 2, 7, { { 10, 30 }, { 13, 30 }, { 16, 25 }, { 18, 10 }, { 28, 5 } }, 5 },
    { "BOSCO_VERDALIA", "Bosco Verdalia", 6, 14, { { 38, 25 }, { 16, 25 }, { 20, 20 }, { 32, 15 }, { 24, 10 }, { 28, 5 } }, 6 },
    { "ARKAS", "Arkas", 20, 32, { { 22, 25 }, { 48, 25 }, { 30, 20 }, { 45, 15 }, { 50, 10 }, { 49, 5 } }, 6 },
    { "NIVAL", "Nival", 30, 40, { { 24, 30 }, { 42, 25 }, { 25, 15 }, { 53, 15 }, { 72, 10 }, { 28, 5 } }, 6 },
    { "TALASSA", "Talassa", 35, 45, { { 13, 30 }, { 37, 25 }, { 59, 20 }, { 51, 15 }, { 69, 10 } }, 5 },
    { "VULKAR", "Vulkar", 40, 50, { { 10, 30 }, { 50, 25 }, { 81, 20 }, { 91, 10 }, { 34, 15 } }, 5 },
    { "ELYS", "Elys", 45, 55, { { 28, 25 }, { 44, 25 }, { 32, 20 }, { 55, 10 }, { 64, 10 }, { 87, 10 } }, 6 },
};

#define NUM_HAB ((int)(sizeof(HABITAT) / sizeof(HABITAT[0])))

const HabitatData *GetHabitat(const char *id) {
    if (!id) return NULL;
    for (int i = 0; i < NUM_HAB; i++) if (strcmp(HABITAT[i].id, id) == 0) return &HABITAT[i];
    return NULL;
}

int NumHabitat(void) { return NUM_HAB; }

const HabitatData *GetHabitatIndex(int i) {
    return (i >= 0 && i < NUM_HAB) ? &HABITAT[i] : NULL;
}

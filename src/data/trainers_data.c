/* FILE GENERATO da tools/genera_database.py - modifica lo script, non questo file. */
#include "trainers_data.h"
#include <stddef.h>
#include <string.h>

static const TrainerData ALLENATORI[] = {
    { "KAEL_INIZIALE_VS_PYRIN", "KAEL", "Rivale", 100, AI_SIMPLE,
      { { 4, 5, { 0, 0, 0, 0 } } }, 1, "", false, false, "Kael sceglie Aquil se il giocatore sceglie Pyrin." },
    { "KAEL_INIZIALE_VS_AQUIL", "KAEL", "Rivale", 100, AI_SIMPLE,
      { { 7, 5, { 0, 0, 0, 0 } } }, 1, "", false, false, "Kael sceglie Mossen se il giocatore sceglie Aquil." },
    { "KAEL_INIZIALE_VS_MOSSEN", "KAEL", "Rivale", 100, AI_SIMPLE,
      { { 1, 5, { 0, 0, 0, 0 } } }, 1, "", false, false, "Kael sceglie Pyrin se il giocatore sceglie Mossen." },
    { "NEXUS_RECLUTA_01", "RECLUTA NEXUS", "Recluta Nexus", 280, AI_SIMPLE,
      { { 10, 8, { 0, 0, 0, 0 } }, { 18, 7, { 0, 0, 0, 0 } } }, 2, "", false, false, "" },
    { "NEXUS_RECLUTA_02", "RECLUTA NEXUS", "Recluta Nexus", 360, AI_SIMPLE,
      { { 26, 10, { 0, 0, 0, 0 } }, { 30, 9, { 0, 0, 0, 0 } } }, 2, "", false, false, "" },
    { "NEXUS_SCIENZIATO", "SCIENZIATO NEXUS", "Scienziato Nexus", 1200, AI_STANDARD,
      { { 32, 15, { 0, 0, 0, 0 } }, { 34, 15, { 0, 0, 0, 0 } }, { 28, 14, { 0, 0, 0, 0 } } }, 3, "", false, false, "" },
    { "NEXUS_COMMANDER", "COMANDANTE NEXUS", "Comandante Nexus", 3000, AI_SMART,
      { { 27, 25, { 0, 0, 0, 0 } }, { 56, 26, { 0, 0, 0, 0 } }, { 71, 27, { 0, 0, 0, 0 } } }, 3, "", false, true, "Boss: può avere specie rare non catturabili dal giocatore a quel punto. Squadra canonica del documento: VENOROK al Lv 26 (VENOMA evolve al 29). Boss con specie rare." },
    { "GUARDIANA_LIRA", "LIRA", "Guardiana", 2100, AI_STANDARD,
      { { 17, 18, { 0, 0, 0, 0 } }, { 8, 19, { 0, 0, 0, 0 } }, { 57, 21, { 0, 0, 0, 0 } } }, 3, "SIGILLO_VERDE", false, true, "Decisione canonica: primo Guardiano Lv 16-21 con MOSSKA/LEAFORA (LEAFORA al Lv 18, evolve al 20)." },
    { "GUARDIANO_KAIN", "KAIN", "Guardiano", 2600, AI_STANDARD,
      { { 19, 23, { 0, 0, 0, 0 } }, { 39, 24, { 0, 0, 0, 0 } }, { 52, 26, { 0, 0, 0, 0 } } }, 3, "SIGILLO_TUONO", false, true, "Squadra canonica del documento: THORVAN al Lv 26 (THUNDERN evolve al 32). Da confermare." },
    { "GUARDIANA_MIRA", "MIRA", "Guardiana", 3000, AI_STANDARD,
      { { 14, 26, { 0, 0, 0, 0 } }, { 51, 27, { 0, 0, 0, 0 } }, { 69, 29, { 0, 0, 0, 0 } } }, 3, "SIGILLO_MAREE", false, false, "" },
    { "GUARDIANO_ROVAN", "ROVAN", "Guardiano", 3500, AI_SMART,
      { { 11, 31, { 0, 0, 0, 0 } }, { 50, 32, { 0, 0, 0, 0 } }, { 3, 34, { 0, 0, 0, 0 } } }, 3, "SIGILLO_FUOCO", false, false, "" },
    { "GUARDIANA_SELENE", "SELENE", "Guardiana", 4000, AI_SMART,
      { { 25, 35, { 0, 0, 0, 0 } }, { 53, 36, { 0, 0, 0, 0 } }, { 77, 38, { 0, 0, 0, 0 } } }, 3, "SIGILLO_GHIACCIO", false, false, "" },
    { "GUARDIANO_DAREN", "DAREN", "Guardiano", 4500, AI_SMART,
      { { 23, 39, { 0, 0, 0, 0 } }, { 61, 40, { 0, 0, 0, 0 } }, { 68, 42, { 0, 0, 0, 0 } } }, 3, "SIGILLO_PIETRA", false, false, "" },
    { "GUARDIANA_ELARA", "ELARA", "Guardiana", 6000, AI_SMART,
      { { 55, 44, { 0, 0, 0, 0 } }, { 29, 45, { 0, 0, 0, 0 } }, { 67, 46, { 0, 0, 0, 0 } }, { 73, 47, { 0, 0, 0, 0 } } }, 4, "SIGILLO_STELLE", false, false, "" },
    { "VAEL_INTERMEZZO", "VAEL", "Ordine Nexus", 0, AI_SMART,
      { { 27, 34, { 0, 0, 0, 0 } }, { 69, 35, { 0, 0, 0, 0 } }, { 66, 36, { 0, 0, 0, 0 } } }, 3, "", true, false, "Interruzione narrativa al 50% HP dell'ultimo Elyon, senza sconfitta forzata." },
    { "LEGA_RHEON", "RHEON", "Lega di Elys", 0, AI_SMART,
      { { 58, 52, { 0, 0, 0, 0 } }, { 81, 53, { 0, 0, 0, 0 } }, { 91, 54, { 0, 0, 0, 0 } }, { 74, 56, { 0, 0, 0, 0 } } }, 4, "", false, false, "" },
    { "LEGA_ILIA", "ILIA", "Lega di Elys", 0, AI_SMART,
      { { 15, 52, { 0, 0, 0, 0 } }, { 59, 53, { 0, 0, 0, 0 } }, { 69, 54, { 0, 0, 0, 0 } }, { 75, 56, { 0, 0, 0, 0 } } }, 4, "", false, false, "" },
    { "LEGA_TOR", "TOR", "Lega di Elys", 0, AI_SMART,
      { { 61, 53, { 0, 0, 0, 0 } }, { 90, 54, { 0, 0, 0, 0 } }, { 68, 55, { 0, 0, 0, 0 } }, { 35, 56, { 0, 0, 0, 0 } } }, 4, "", false, false, "" },
    { "LEGA_NYRA", "NYRA", "Lega di Elys", 0, AI_SMART,
      { { 89, 54, { 0, 0, 0, 0 } }, { 72, 55, { 0, 0, 0, 0 } }, { 55, 56, { 0, 0, 0, 0 } }, { 64, 57, { 0, 0, 0, 0 } } }, 4, "", false, false, "" },
    { "CAMPIONE_KAEL_VS_PYRIN", "KAEL", "Campione", 12000, AI_BOSS,
      { { 6, 58, { 0, 0, 0, 0 } }, { 95, 59, { 0, 0, 0, 0 } }, { 80, 60, { 0, 0, 0, 0 } }, { 87, 60, { 0, 0, 0, 0 } }, { 73, 61, { 0, 0, 0, 0 } }, { 69, 62, { 0, 0, 0, 0 } } }, 6, "TORRE_ACCESSO", false, false, "Se il giocatore ha scelto Pyrin." },
    { "CAMPIONE_KAEL_VS_AQUIL", "KAEL", "Campione", 12000, AI_BOSS,
      { { 3, 58, { 0, 0, 0, 0 } }, { 95, 59, { 0, 0, 0, 0 } }, { 80, 60, { 0, 0, 0, 0 } }, { 87, 60, { 0, 0, 0, 0 } }, { 73, 61, { 0, 0, 0, 0 } }, { 91, 62, { 0, 0, 0, 0 } } }, 6, "TORRE_ACCESSO", false, false, "Se il giocatore ha scelto Aquil." },
    { "CAMPIONE_KAEL_VS_MOSSEN", "KAEL", "Campione", 12000, AI_BOSS,
      { { 3, 58, { 0, 0, 0, 0 } }, { 6, 59, { 0, 0, 0, 0 } }, { 80, 60, { 0, 0, 0, 0 } }, { 87, 60, { 0, 0, 0, 0 } }, { 64, 61, { 0, 0, 0, 0 } }, { 69, 62, { 0, 0, 0, 0 } } }, 6, "TORRE_ACCESSO", false, false, "Se il giocatore ha scelto Mossen." },
    { "VAEL_FINALE", "VAEL", "Ordine Nexus", 0, AI_BOSS,
      { { 71, 60, { 0, 0, 0, 0 } }, { 69, 60, { 0, 0, 0, 0 } }, { 70, 61, { 0, 0, 0, 0 } }, { 65, 62, { 0, 0, 0, 0 } }, { 66, 64, { 0, 0, 0, 0 } } }, 5, "", false, false, "Dopo la Lega." },
    { "AURION_FASE1", "AURION", "Primordiale", 0, AI_BOSS,
      { { 100, 65, { 66, 61, 95, 97 } } }, 1, "", false, false, "Fase 1: RAGGIO STELLARE, ABISSO, ROTTURA LUMEN, EQUILIBRIO. Fase 2 al 50%: forma VOID. Fase 3 al 20%: campo instabile +20%." },
    { "AURION_FASE2_VOID", "AURION", "Primordiale", 0, AI_BOSS,
      { { 100, 65, { 98, 68, 62, 67 } } }, 1, "", false, false, "Forma VOID: FRATTURA DEL VUOTO, SUPERNOVA, ECLISSI, NOVA." },
    { "POSTGAME_VESPERA", "VESPERA", "Leggendario", 0, AI_BOSS,
      { { 99, 75, { 0, 0, 0, 0 } } }, 1, "", false, false, "Incontro post-game." },
    { "POSTGAME_AURION_ORIGINALE", "AURION", "Primordiale", 0, AI_BOSS,
      { { 100, 80, { 0, 0, 0, 0 } } }, 1, "", false, false, "Forma originale, post-game." },
};

#define NUM_ALL ((int)(sizeof(ALLENATORI) / sizeof(ALLENATORI[0])))

const TrainerData *GetTrainer(const char *id) {
    if (!id) return NULL;
    for (int i = 0; i < NUM_ALL; i++) if (strcmp(ALLENATORI[i].id, id) == 0) return &ALLENATORI[i];
    return NULL;
}

int NumAllenatori(void) { return NUM_ALL; }

const TrainerData *GetTrainerIndex(int i) {
    return (i >= 0 && i < NUM_ALL) ? &ALLENATORI[i] : NULL;
}

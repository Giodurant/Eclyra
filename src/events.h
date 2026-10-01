/* events.h - Flag di gioco con nome (eventi completati, oggetti raccolti, progressi). */
#ifndef ECLYRA_EVENTS_H
#define ECLYRA_EVENTS_H

#include <stdbool.h>

#define FLAG_MAX 256
#define FLAG_NOME 32

typedef struct {
    char nome[FLAG_MAX][FLAG_NOME];
    int numero;
} Flags;

void Flags_Azzera(Flags *f);
/* Ritorna false se la tabella e' piena o il nome non e' valido */
bool Flags_Imposta(Flags *f, const char *nome);
void Flags_Rimuovi(Flags *f, const char *nome);
bool Flags_Attivo(const Flags *f, const char *nome);

#endif

/* events.c */
#include "events.h"
#include "utils.h"
#include <string.h>

void Flags_Azzera(Flags *f) { memset(f, 0, sizeof(*f)); }

bool Flags_Attivo(const Flags *f, const char *nome) {
    if (!nome || !nome[0]) return false;
    for (int i = 0; i < f->numero; i++)
        if (strcmp(f->nome[i], nome) == 0) return true;
    return false;
}

bool Flags_Imposta(Flags *f, const char *nome) {
    if (!nome || !nome[0] || strlen(nome) >= FLAG_NOME) return false;
    if (Flags_Attivo(f, nome)) return true;
    if (f->numero >= FLAG_MAX) return false;
    Utils_StrCopy(f->nome[f->numero++], FLAG_NOME, nome);
    return true;
}

void Flags_Rimuovi(Flags *f, const char *nome) {
    for (int i = 0; i < f->numero; i++) {
        if (strcmp(f->nome[i], nome) == 0) {
            memmove(&f->nome[i], &f->nome[i + 1], (size_t)(f->numero - i - 1) * FLAG_NOME);
            f->numero--;
            return;
        }
    }
}

/* FILE GENERATO da tools/genera_database.py - modifica lo script, non questo file. */
#include "abilities_data.h"
#include "moves_data.h"
#include "species_data.h"
#include <stddef.h>

static const AbilityData ABILITA[NUM_ABILITA] = {
    { 1, "FIAMMA_INTERNA", 0, AB_MOLT_TIPO_ATTACCO, COND_HP_SOTTO_30, TYPE_FIAMMA, TYPE_NONE, 1.25f, 0, "Sotto il 30% degli HP, mosse Fiamma x1,25." },
    { 2, "PELLE_UMIDA", 0, AB_MOLT_DANNO_SUBITO, COND_SEMPRE, TYPE_FIAMMA, TYPE_NONE, 0.7f, 0, "Danni Fiamma subiti x0,70." },
    { 3, "RADICE_VIVA", 0, AB_CURA_FINE_TURNO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 2.0f, 0, "A fine turno cura il 2% degli HP." },
    { 4, "CONDUTTORE", 0, AB_IMMUNITA_TIPO, COND_SEMPRE, TYPE_IMPULSO, TYPE_NONE, 1.0f, BOOST_SPA, "Immune a Impulso; SPA +1 quando ne viene colpito." },
    { 5, "CORAZZA", 0, AB_MOLT_DANNO_FISICO_SUBITO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 0.9f, 0, "Danni fisici subiti x0,90." },
    { 6, "GELO_ETERNO", 0, AB_IMMUNITA_STATUS, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, STATUS_GELO, "Immune al congelamento." },
    { 7, "OMBRA_DENSA", 0, AB_MOLT_DANNO_SUBITO, COND_SEMPRE, TYPE_ASTRALE, TYPE_NONE, 0.7f, 0, "Danni Astrale subiti x0,70." },
    { 8, "MENTE_LUCIDA", 0, AB_IMMUNITA_STATUS, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, STATUS_CONFUSIONE, "Immune alla confusione." },
    { 9, "AURA_LUMEN", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_ASTRALE, TYPE_NONE, 1.15f, 0, "Mosse Astrale x1,15." },
    { 10, "CORRENTE_RAPIDA", 0, AB_MOLT_STAT, COND_PIOGGIA, TYPE_NONE, TYPE_NONE, 1.15f, STAT_SPE, "Con la pioggia SPE x1,15." },
    { 11, "FOTOSINTESI", 0, AB_CURA_FINE_TURNO, COND_SOLE, TYPE_NONE, TYPE_NONE, 3.0f, 0, "Al sole cura il 3% degli HP a ogni turno." },
    { 12, "CORAZZA_FERROSA", 0, AB_CRITICO_RIDOTTO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 0.75f, 0, "Bonus dei critici avversari ridotto del 25%." },
    { 13, "SCUDO_LUMEN", 0, AB_RESISTE_UN_COLPO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 0, "Una volta per battaglia resta a 1 HP." },
    { 14, "NUCLEO_ARDENTE", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_FIAMMA, TYPE_NONE, 1.2f, 0, "Mosse Fiamma x1,20." },
    { 15, "VELENO_ATTIVO", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_TOSSICO, TYPE_NONE, 1.15f, 0, "Mosse Tossico x1,15." },
    { 16, "MENTE_OSCURA", 0, AB_MOLT_TIPO_ATTACCO, COND_HP_SOTTO_50, TYPE_PSICHE, TYPE_NONE, 1.15f, 0, "Sotto il 50% degli HP, mosse Psiche x1,15." },
    { 17, "MAGMA_INTERNO", 0, AB_IMMUNITA_STATUS, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, STATUS_BRUCIATURA, "Immune alla bruciatura." },
    { 18, "ARMATURA_MARINA", 0, AB_MOLT_DANNO_SUBITO, COND_SEMPRE, TYPE_ACQUA, TYPE_NONE, 0.8f, 0, "Danni Acqua subiti x0,80." },
    { 19, "CORRENTE_CELESTE", 0, AB_MOLT_STAT, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.2f, STAT_SPE, "SPE x1,20." },
    { 20, "EQUILIBRIO_NOTTURNO", 0, AB_MOLT_TIPO_ATTACCO, COND_NOTTE, TYPE_OMBRA, TYPE_NONE, 1.2f, 0, "Di notte mosse Ombra x1,20." },
    { 21, "SOVRACCARICO", 0, AB_MOLT_TIPO_ATTACCO, COND_HP_SOTTO_25, TYPE_IMPULSO, TYPE_NONE, 1.3f, 0, "Sotto il 25% degli HP, mosse Impulso x1,30." },
    { 22, "BREZZA_VITALE", 0, AB_CURA_FINE_TURNO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 2.0f, 0, "Cura il 2% degli HP a ogni turno." },
    { 23, "CUORE_VEGETALE", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_NATURA, TYPE_NONE, 1.15f, 0, "Mosse Natura x1,15." },
    { 24, "CORAZZA_TITANICA", 0, AB_MOLT_DANNO_FISICO_SUBITO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 0.85f, 0, "Danni fisici subiti x0,85." },
    { 25, "MAREA_LUMEN", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_ACQUA, TYPE_NONE, 1.2f, 0, "Mosse Acqua x1,20." },
    { 26, "OMBRA_ETERNA", 0, AB_IMMUNE_CALO_ATK, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 0, "Immune ai cali di ATK causati da abilità." },
    { 27, "LUCE_ARDENTE", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_ASTRALE, TYPE_FIAMMA, 1.15f, 0, "Mosse Astrale e Fiamma x1,15." },
    { 28, "VELENO_OSCURO", 0, AB_STATUS_SU_ATTACCO, COND_SEMPRE, TYPE_OMBRA, TYPE_NONE, 1.0f, 10, "Le mosse Ombra avvelenano nel 10% dei casi." },
    { 29, "NUCLEO_DEL_VUOTO", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_OMBRA, TYPE_NONE, 1.25f, 0, "Mosse Ombra x1,25." },
    { 30, "EQUILIBRIO_DEL_VUOTO", 0, AB_EFFETTO_CAMPO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 3, "Ogni tre turni modifica il campo. Solo Aurion." },
    { 31, "CORRENTE_LUMEN", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_ACQUA, TYPE_NONE, 1.15f, 0, "Mosse Acqua x1,15." },
    { 32, "VENTO_ARDENTE", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_FIAMMA, TYPE_VENTO, 1.1f, 0, "Mosse Fiamma e Vento x1,10." },
    { 33, "CORRENTE_ASCENDENTE", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_VENTO, TYPE_NONE, 1.15f, 0, "Mosse Vento x1,15." },
    { 34, "VELENO_OSCURO", 28, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 0, "Alias di VELENO_OSCURO (028)." },
    { 35, "OMBRA_MOBILE", 0, AB_MOLT_STAT, COND_NOTTE, TYPE_NONE, TYPE_NONE, 1.1f, STAT_SPE, "Di notte SPE x1,10." },
    { 36, "AURA_ARDENTE", 27, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 0, "Alias di LUCE_ARDENTE (027)." },
    { 37, "CORRENTE_VITALE", 0, AB_CURA_FINE_TURNO, COND_PIOGGIA, TYPE_NONE, TYPE_NONE, 2.0f, 0, "Con la pioggia cura il 2% degli HP a ogni turno." },
    { 38, "TEMPESTA_DI_SABBIA", 0, AB_MOLT_TIPO_ATTACCO, COND_TEMPESTA, TYPE_ROCCIA, TYPE_NONE, 1.15f, 0, "In tempesta mosse Roccia x1,15." },
    { 39, "MENTE_GELIDA", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_GELO, TYPE_NONE, 1.15f, 0, "Mosse Gelo x1,15." },
    { 40, "FIAMMA_OSCURA", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_OMBRA, TYPE_FIAMMA, 1.1f, 0, "Mosse Ombra e Fiamma x1,10." },
    { 41, "ARMATURA_TITANICA", 24, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 0, "Alias di CORAZZA_TITANICA (024)." },
    { 42, "CALORE_PRIMORDIALE", 0, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_FIAMMA, TYPE_NONE, 1.15f, 0, "Mosse Fiamma x1,15." },
    { 43, "CACCIATORE_NOTTURNO", 0, AB_CRITICO_ALTO, COND_NOTTE, TYPE_NONE, TYPE_NONE, 1.0f, 12, "Di notte probabilità di critico 12%." },
    { 44, "VELENO_ACQUATICO", 0, AB_STATUS_SU_ATTACCO, COND_SEMPRE, TYPE_ACQUA, TYPE_NONE, 1.0f, 10, "Le mosse Acqua avvelenano nel 10% dei casi." },
    { 45, "EQUILIBRIO_NOTTURNO", 20, AB_MOLT_TIPO_ATTACCO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 0, "Alias di EQUILIBRIO_NOTTURNO (020)." },
    { 46, "FRATTURA_DEL_VUOTO", 0, AB_EFFETTO_CAMPO, COND_SEMPRE, TYPE_NONE, TYPE_NONE, 1.0f, 3, "Forma boss: effetto di campo per tre turni." },
};

const AbilityData *GetAbility(int id) {
    if (id < 1 || id > NUM_ABILITA) return NULL;
    return &ABILITA[id - 1];
}

const AbilityData *GetAbilityResolved(int id) {
    const AbilityData *a = GetAbility(id);
    for (int passi = 0; a && a->aliasDi != 0 && passi < 4; passi++) a = GetAbility(a->aliasDi);
    return a;
}

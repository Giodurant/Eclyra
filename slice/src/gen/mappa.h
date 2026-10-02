/* GENERATO da tools/design/slice_export.py: regioni della mappa di Asterra. */
#ifndef GEN_MAPPA_H
#define GEN_MAPPA_H

typedef struct { const char *nome, *sottotitolo; int cx, cy; unsigned char chiaro[3]; } RegioneGen;

static const RegioneGen REGIONI_GEN[] = {
    { "NIVAL", "Gelo", 176, 40, { 242, 248, 255 } },
    { "ARKAS", "Roccia", 76, 74, { 232, 200, 144 } },
    { "ELYS", "Astrale", 176, 84, { 184, 176, 255 } },
    { "VULKAR", "Fiamma", 262, 76, { 210, 104, 62 } },
    { "VERDAN", "Natura", 150, 122, { 138, 200, 90 } },
    { "TALASSA", "Acqua", 250, 132, { 124, 192, 232 } },
    { "LUNARIA", "Inizio", 112, 140, { 184, 224, 138 } },
};
#define NUM_REGIONI_GEN 7

#endif

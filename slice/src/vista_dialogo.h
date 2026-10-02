/* vista_dialogo.h - Riquadro dei dialoghi (Design System §8): pergamena, targa del nome,
 * inchiostro, cristallo di avanzamento. Tre righe per pagina. */
#ifndef SLICE_VISTA_DIALOGO_H
#define SLICE_VISTA_DIALOGO_H

#include "dialogue.h"
#include "canvas.h"

#define DIALOGO_RIGHE 3
#define DIALOGO_LARGHEZZA_TESTO 284     /* riquadro 312 px, margini 14 */

void VistaDialogo_Disegna(const Dialogo *d, const Canvas *c, int frame);
/* Riquadro con un testo fisso (avvisi della slice, domande) */
void VistaDialogo_Testo(const char *parlante, const char *testo, const Canvas *c, int frame, bool indicatore);
/* Domanda SI/NO sopra al riquadro */
void VistaDialogo_SiNo(int cursore, const Canvas *c, int frame);

#endif

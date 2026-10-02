/* proto_audio.h - Audio del prototipo: musica sintetizzata in streaming e un effetto sonoro.
 * Nessun file audio: verifica il dispositivo audio, lo streaming continuo e la pausa. */
#ifndef PROTO_AUDIO_H
#define PROTO_AUDIO_H

#include <stdbool.h>

bool Audio_Init(void);
void Audio_Close(void);
void Audio_PlaySfx(void);
void Audio_Suspend(void);
void Audio_Resume(void);
bool Audio_Ready(void);
bool Audio_Suspended(void);
/* Fotogrammi audio generati dall'avvio: se cresce, la musica sta suonando davvero */
unsigned long long Audio_FramesGenerated(void);

#endif

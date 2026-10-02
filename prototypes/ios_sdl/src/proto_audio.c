/* proto_audio.c - Melodia in loop generata nel callback dello stream + bip per i tasti */
#include "proto_audio.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>

#define FREQ 44100

static AudioStream musica;
static Sound bip;
static bool pronto, sospeso;
static volatile unsigned long long generati;

/* Arpeggio in La minore, una nota ogni 1/6 di secondo: abbastanza per sentire tagli e scatti */
static const float NOTE[16] = { 440.00f, 523.25f, 659.25f, 523.25f, 392.00f, 493.88f, 587.33f, 493.88f,
                                349.23f, 440.00f, 523.25f, 440.00f, 329.63f, 415.30f, 493.88f, 415.30f };

static void Genera(void *buffer, unsigned int frames) {
    static double fase = 0.0;
    static unsigned long long t = 0;
    short *out = (short *)buffer;
    for (unsigned int i = 0; i < frames; i++, t++) {
        int nota = (int)((t / (FREQ / 6)) % 16);
        double pos = (double)(t % (FREQ / 6)) / (double)(FREQ / 6);
        double inviluppo = pos < 0.05 ? pos / 0.05 : 1.0 - (pos - 0.05) * 0.8;
        fase += NOTE[nota] / (double)FREQ;
        if (fase >= 1.0) fase -= 1.0;
        double onda = fase < 0.5 ? 4.0 * fase - 1.0 : 3.0 - 4.0 * fase;   /* triangolo, morbido */
        out[i] = (short)(onda * inviluppo * 0.18 * 32767.0);
    }
    generati += frames;
}

bool Audio_Init(void) {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return false;
    SetAudioStreamBufferSizeDefault(2048);
    musica = LoadAudioStream(FREQ, 16, 1);
    SetAudioStreamCallback(musica, Genera);
    PlayAudioStream(musica);

    /* bip di 70 ms a 880 Hz con attacco e rilascio brevi */
    int n = FREQ * 7 / 100;
    short *dati = malloc((size_t)n * sizeof(short));
    for (int i = 0; i < n; i++) {
        double env = i < 200 ? i / 200.0 : (double)(n - i) / (double)n;
        dati[i] = (short)(sin(2.0 * 3.14159265 * 880.0 * i / FREQ) * env * 0.35 * 32767.0);
    }
    Wave w = { (unsigned int)n, FREQ, 16, 1, dati };
    bip = LoadSoundFromWave(w);
    free(dati);
    pronto = true;
    return true;
}

void Audio_Close(void) {
    if (!pronto) return;
    UnloadSound(bip);
    UnloadAudioStream(musica);
    CloseAudioDevice();
    pronto = false;
}

void Audio_PlaySfx(void) { if (pronto && !sospeso) PlaySound(bip); }

void Audio_Suspend(void) {
    if (!pronto || sospeso) return;
    PauseAudioStream(musica);
    sospeso = true;
}

void Audio_Resume(void) {
    if (!pronto || !sospeso) return;
    ResumeAudioStream(musica);
    sospeso = false;
}

bool Audio_Ready(void) { return pronto; }
bool Audio_Suspended(void) { return sospeso; }
unsigned long long Audio_FramesGenerated(void) { return generati; }

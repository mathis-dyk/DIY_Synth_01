#pragma once

struct SynthState {
    // OSC
    int   wave;        // index dans waveforms[]
    int   octave;      // transposition en octaves (-2..+2)
    float detune;      // écart entre les 2 oscs, en demi-tons
    float sub_level;   // volume de l'osc sub (1 octave en dessous)
    bool  osc_on[3];   // osc1, osc2, sub allumés ?
    bool  osc_dirty;   // osc_on a changé → le moteur doit remettre à jour le mixer

    // FILTER
    float cutoff, resonance;

    // RIBBON
    int   ribbon_raw;
    float ribbon_pos;      // position calibrée 0..1 (bas → haut)
    float ribbon_freq;
    bool  ribbon_active;
    int   note;            // demi-ton joué au-dessus de C (0..11)

    // TABLES
    int          num_waveforms;
    const int*   waveforms;
    const char** wave_names;
    const char** note_names;

    // UI
    bool ui_dirty;
};

extern SynthState state;

#pragma once

struct SynthState {
    // OSC
    int   wave;        // index dans waveforms[]
    int   octave;      // transposition en octaves (-2..+2)
    float detune;      // écart entre les 2 oscs, en demi-tons
    float sub_level;   // volume de l'osc sub (1 octave en dessous)

    // FILTER
    float cutoff, resonance;

    // RIBBON
    int   ribbon_raw;
    float ribbon_freq;
    bool  ribbon_active;

    // TABLES
    int          num_waveforms;
    const int*   waveforms;
    const char** wave_names;

    // UI
    bool ui_dirty;
};

extern SynthState state;

#pragma once

struct SynthState {
    // OSC
    int   wave[3];     // forme d'onde de osc1, osc2, sub (index dans waveforms[])
    int   octave;      // transposition en octaves (-2..+2)
    float detune;      // écart entre les 2 oscs, en demi-tons
    float osc_level[3]; // volume de osc1, osc2, sub (0..2 = 0..200 %)
    bool  osc_on[3];   // osc1, osc2, sub allumés ?
    bool  sound_dirty; // un réglage du son a changé → le moteur doit le réappliquer
    int   edit_target; // osc dont la forme d'onde est verrouillée sur l'encodeur (-1 = octave)
    bool  hold;        // HOLD : la dernière note reste tenue (double clic encodeur)

    // MENU RÉGLAGES (clic simple sur l'encodeur)
    float volume;      // volume général (0..1)
    float glide;       // portamento en ms (0 = pas de glissé)
    int   transpose;   // transposition en demi-tons (-12..+12)
    float pitch_env_amt;   // enveloppe de hauteur : départ en demi-tons au-dessus (0 = OFF)
    float pitch_env_time;  // durée de la descente (ms)
    int   menu_index;  // réglage affiché dans le menu (-1 = menu fermé)

    // FILTER
    float cutoff, resonance;

    // ENVELOPPE (ms)
    float attack, decay, release;   // ms
    float sustain;                   // niveau tenu (0..1)

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
    const char*   popup_name;      // dernier réglage touché, affiché en gros un moment
    char          popup_value[16];
    unsigned long popup_time;      // millis() du dernier réglage (0 = aucun)
};

extern SynthState state;


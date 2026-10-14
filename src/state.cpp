#include "state.h"
#include <Audio.h>

namespace {
const int kWaveforms[] = {
    WAVEFORM_SAWTOOTH,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SINE
};

const char* kWaveNames[] = {"SAW", "SQR", "TRI", "SINE"};

const char* kNoteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
}

SynthState state = {
    // OSC
    .wave      = {0, 0, 1},   // SAW, SAW, SQR
    .octave    = 0,
    .detune    = 0.12,
    .osc_level = {1.0, 1.0, 0.5},
    .osc_on    = {true, true, true},
    .sound_dirty = true,
    .edit_target = -1,
    .hold        = false,
    .volume      = 1.0,
    .glide       = 0,
    .transpose   = 0,
    .pitch_env_amt  = 0,
    .pitch_env_time = 40,
    .menu_index  = -1,
    // FILTER
    .cutoff    = 2500.0,
    .resonance = 1.2,
    // ENVELOPPE
    .attack    = 5.0,
    .decay     = 200.0,
    .release   = 150.0,
    .sustain   = 1.0,
    // RIBBON
    .ribbon_raw    = 0,
    .ribbon_pos    = 0,
    .ribbon_freq   = 220.0,
    .ribbon_active = false,
    .note          = 0,
    // TABLES
    .num_waveforms = 4,
    .waveforms     = kWaveforms,
    .wave_names    = kWaveNames,
    .note_names    = kNoteNames,
    // UI
    .ui_dirty    = true,
    .popup_name  = nullptr,
    .popup_value = "",
    .popup_time  = 0
};

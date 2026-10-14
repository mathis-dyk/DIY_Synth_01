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
    .wave      = 0,
    .octave    = 0,
    .detune    = 0.12,
    .sub_level = 0.5,
    .osc_on    = {true, true, true},
    .osc_dirty = true,
    // FILTER
    .cutoff    = 2500.0,
    .resonance = 1.2,
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
    .ui_dirty = true
};

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
}

SynthState state = {
    // OSC
    .wave      = 0,
    .octave    = 0,
    .detune    = 0.12,
    .sub_level = 0.5,
    // FILTER
    .cutoff    = 2500.0,
    .resonance = 1.2,
    // RIBBON
    .ribbon_raw    = 0,
    .ribbon_freq   = 220.0,
    .ribbon_active = false,
    // TABLES
    .num_waveforms = 4,
    .waveforms     = kWaveforms,
    .wave_names    = kWaveNames,
    // UI
    .ui_dirty = true
};

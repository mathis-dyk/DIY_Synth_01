#include <Audio.h>

#include "engine.h"
#include "state.h"

// osc1 + osc2 (désaccordés) + sub → mixer → filtre → enveloppe → PCM5102
AudioSynthWaveform       osc1;
AudioSynthWaveform       osc2;
AudioSynthWaveform       sub;
AudioMixer4              mixer;
AudioFilterStateVariable filter1;
AudioEffectEnvelope      env;
AudioOutputI2S           i2s1;

AudioConnection c1(osc1, 0, mixer, 0);
AudioConnection c2(osc2, 0, mixer, 1);
AudioConnection c3(sub,  0, mixer, 2);
AudioConnection c4(mixer, 0, filter1, 0);
AudioConnection c5(filter1, 0, env, 0);   // sortie 0 = passe-bas
AudioConnection c6(env, 0, i2s1, 0);
AudioConnection c7(env, 0, i2s1, 1);

static int last_wave = -1;

void engine_setup() {
    AudioMemory(32);

    osc1.begin(0.5, 220, state.waveforms[state.wave]);
    osc2.begin(0.5, 220, state.waveforms[state.wave]);
    sub.begin(0.5, 110, WAVEFORM_SQUARE);

    // somme max < 1.0 pour éviter la saturation (0.4 x3 saturait)
    mixer.gain(0, 0.25);
    mixer.gain(1, 0.25);
    mixer.gain(2, 0.25 * state.sub_level);

    filter1.frequency(state.cutoff);
    filter1.resonance(state.resonance);

    env.attack(5);
    env.decay(0);
    env.sustain(1.0);
    env.release(150);
}

void engine_update() {
    if (state.wave != last_wave) {
        osc1.begin(state.waveforms[state.wave]);
        osc2.begin(state.waveforms[state.wave]);
        last_wave = state.wave;
    }

    float f = state.ribbon_freq;
    float detune_ratio = powf(2.0f, state.detune / 12.0f);
    AudioNoInterrupts();
    osc1.frequency(f);
    osc2.frequency(f * detune_ratio);
    sub.frequency(f * 0.5f);
    AudioInterrupts();

    static bool gate = false;
    if (state.ribbon_active && !gate) { env.noteOn();  gate = true;  }
    if (!state.ribbon_active && gate) { env.noteOff(); gate = false; }
}

#include <Audio.h>

#include "engine.h"
#include "state.h"

// osc1 + osc2 (désaccordés) + sub → mixer → filtre → enveloppe → volume → PCM5102
AudioSynthWaveform       osc1;
AudioSynthWaveform       osc2;
AudioSynthWaveform       sub;
AudioMixer4              mixer;
AudioFilterStateVariable filter1;
AudioEffectEnvelope      env;
AudioAmplifier           amp;      // volume général
AudioOutputI2S           i2s1;

AudioConnection c1(osc1, 0, mixer, 0);
AudioConnection c2(osc2, 0, mixer, 1);
AudioConnection c3(sub,  0, mixer, 2);
AudioConnection c4(mixer, 0, filter1, 0);
AudioConnection c5(filter1, 0, env, 0);   // sortie 0 = passe-bas
AudioConnection c6(env, 0, amp, 0);
AudioConnection c7(amp, 0, i2s1, 0);
AudioConnection c8(amp, 0, i2s1, 1);

static AudioSynthWaveform* oscs[3] = {&osc1, &osc2, &sub};
static int last_wave[3] = {-1, -1, -1};

// gain de chaque osc dans le mixer quand il est allumé (osc1, osc2, sub)
static void update_mixer() {
    // 0.25 = niveau à 100 % ; à 200 % partout la somme reste sous la saturation (3 x 0.5 x 0.5 = 0.75)
    for (int i = 0; i < 3; i++) {
        mixer.gain(i, state.osc_on[i] ? 0.25f * state.osc_level[i] : 0.0f);
    }
}

// réapplique tous les réglages du son (appelé quand le menu ou un bouton change quelque chose)
static void apply_sound() {
    update_mixer();
    filter1.frequency(state.cutoff);
    filter1.resonance(state.resonance);
    env.attack(state.attack);
    env.decay(state.decay);
    env.sustain(state.sustain);
    env.release(state.release);
    amp.gain(state.volume);
}

void engine_setup() {
    AudioMemory(32);

    osc1.begin(0.5, 220, state.waveforms[state.wave[0]]);
    osc2.begin(0.5, 220, state.waveforms[state.wave[1]]);
    sub.begin(0.5, 110, state.waveforms[state.wave[2]]);

    apply_sound();
}

void engine_update() {
    for (int i = 0; i < 3; i++) {
        if (state.wave[i] != last_wave[i]) {
            oscs[i]->begin(state.waveforms[state.wave[i]]);
            last_wave[i] = state.wave[i];
        }
    }

    if (state.sound_dirty) {
        apply_sound();
        state.sound_dirty = false;
    }

    static bool gate = false;
    static float f = 0;
    static unsigned long last_us = micros();
    static unsigned long note_start_us = 0;   // départ de l'enveloppe de hauteur
    unsigned long now_us = micros();
    float dt_ms = (now_us - last_us) / 1000.0f;
    last_us = now_us;

    // déclenchement / relâchement de la note
    bool new_note = state.ribbon_active && !gate;
    if (new_note) { env.noteOn(); note_start_us = now_us; }
    if (!state.ribbon_active && gate) env.noteOff();

    // GLIDE : la fréquence glisse vers la note visée au lieu d'y sauter.
    // Seulement en legato (note déjà en cours) : une nouvelle note après un silence part juste.
    float target = state.ribbon_freq;
    if (new_note || !gate || state.glide < 1.0f || f <= 0) {
        f = target;
    } else {
        // approche exponentielle : ~63 % du chemin parcouru en `glide` ms
        f += (target - f) * (1.0f - expf(-dt_ms / state.glide));
    }
    gate = state.ribbon_active;

    // ENVELOPPE DE HAUTEUR : chaque nouvelle note démarre `pitch_env_amt` demi-tons plus haut
    // et redescend vers sa vraie hauteur (~63 % du chemin en `pitch_env_time` ms) → effet kick / "piou"
    float played = f;
    if (state.pitch_env_amt > 0) {
        float elapsed_ms = (now_us - note_start_us) / 1000.0f;
        float offset_st = state.pitch_env_amt * expf(-elapsed_ms / state.pitch_env_time);
        played *= powf(2.0f, offset_st / 12.0f);
    }

    float detune_ratio = powf(2.0f, state.detune / 12.0f);
    AudioNoInterrupts();
    osc1.frequency(played);
    osc2.frequency(played * detune_ratio);
    sub.frequency(played * 0.5f);
    AudioInterrupts();
}

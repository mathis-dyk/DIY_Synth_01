#include <Audio.h>
#include <Encoder.h>

#include "controls.h"
#include "state.h"

// ---- Câblage ----
static const uint8_t ENC_A      = 4;
static const uint8_t ENC_B      = 3;
static const uint8_t ENC_SW     = 2;
static const uint8_t RIBBON_PIN = A0;   // pin 14 (le 2e ribbon est sur A1 / pin 15)

// ---- Ribbon : à ajuster en regardant RIB= dans le moniteur série ----
static const int   RIBBON_MIN     = 30;    // valeur lue tout en bas du ribbon
static const int   RIBBON_MAX     = 800;   // valeur lue tout en haut
static const int   RIBBON_ON      = 80;    // seuil "doigt posé"
static const int   RIBBON_OFF     = 50;    // seuil "doigt levé"
static const float RIBBON_OCTAVES = 3.0;   // étendue du ribbon
static const float RIBBON_LOW_HZ  = 65.41; // C2

static Encoder enc(ENC_A, ENC_B);
static long    last_enc_pos = 0;
static bool    last_sw = HIGH;
static unsigned long last_sw_time = 0;
static unsigned long last_print = 0;
static float   smoothed = 0;

void controls_setup() {
    pinMode(ENC_SW, INPUT_PULLUP);
    pinMode(RIBBON_PIN, INPUT_DISABLE);
    analogReadAveraging(8);
}

void controls_update() {
    // Encodeur → octave
    long pos = enc.read() / 4;
    long delta = pos - last_enc_pos;
    if (delta != 0) {
        state.octave = constrain(state.octave + delta, -2, 2);
        last_enc_pos = pos;
        state.ui_dirty = true;
    }

    // Clic → forme d'onde suivante
    bool sw = digitalRead(ENC_SW);
    if (sw == LOW && last_sw == HIGH && millis() - last_sw_time > 200) {
        state.wave = (state.wave + 1) % state.num_waveforms;
        last_sw_time = millis();
        state.ui_dirty = true;
    }
    last_sw = sw;

    // Ribbon → pitch
    int raw = analogRead(RIBBON_PIN);
    state.ribbon_raw = raw;

    bool was_active = state.ribbon_active;
    if (!state.ribbon_active && raw > RIBBON_ON)  state.ribbon_active = true;
    if ( state.ribbon_active && raw < RIBBON_OFF) state.ribbon_active = false;

    if (state.ribbon_active) {
        // au premier contact on saute direct à la position, ensuite on lisse
        if (!was_active) smoothed = raw;
        else             smoothed += (raw - smoothed) * 0.2f;

        float pos01 = constrain((smoothed - RIBBON_MIN) / float(RIBBON_MAX - RIBBON_MIN), 0.0f, 1.0f);
        state.ribbon_freq = RIBBON_LOW_HZ * powf(2.0f, pos01 * RIBBON_OCTAVES + state.octave);
    }
    if (was_active != state.ribbon_active) state.ui_dirty = true;

    // Debug série : sert à calibrer RIBBON_MIN / MAX
    if (millis() - last_print > 200) {
        Serial.print("RIB="); Serial.print(raw);
        Serial.print(" A1="); Serial.print(analogRead(A1));
        Serial.print(" ON=");  Serial.print(state.ribbon_active);
        Serial.print(" HZ=");  Serial.print(state.ribbon_freq);
        Serial.print(" OCT="); Serial.print(state.octave);
        Serial.print(" WAVE=");Serial.print(state.wave_names[state.wave]);
        // diagnostic audio : MEM doit rester < 32 (AudioMemory), CPU < 100
        Serial.print(" MEM="); Serial.print(AudioMemoryUsageMax());
        Serial.print(" CPU="); Serial.println(AudioProcessorUsageMax());
        last_print = millis();
    }
}

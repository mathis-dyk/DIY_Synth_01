#include <Audio.h>
#include <Encoder.h>

#include "controls.h"
#include "state.h"

// ---- Câblage ----
static const uint8_t ENC_A      = 4;
static const uint8_t ENC_B      = 3;
static const uint8_t ENC_SW     = 2;

// boutons on/off des oscs (osc1, osc2, sub) : bouton entre la pin et GND
static const uint8_t OSC_BTN[3] = {5, 6, 9};
// LED des boutons : pin → résistance 330Ω → LED → GND
static const uint8_t OSC_LED[3] = {10, 11, 12};
static const unsigned long DEBOUNCE_MS = 30;

// ---- Ribbon : à ajuster en regardant RIB1= / RIB2= dans le moniteur série ----
// mesuré sur le ribbon 1 : bas = 62, haut = 780, au repos = 2..9
// (le ribbon 2 utilise les mêmes valeurs tant qu'il n'est pas mesuré)
static const int   RIBBON_MIN     = 70;    // un peu au-dessus du bas mesuré (bord facile à atteindre)
static const int   RIBBON_MAX     = 760;   // un peu sous le haut mesuré
static const int   RIBBON_ON      = 35;    // seuil "doigt posé" : entre le bruit (9) et le bas (62)
static const int   RIBBON_OFF     = 20;    // seuil "doigt levé"
static const float ZONE_HYST      = 0.15f; // marge (en fraction de zone) avant de changer de note
static const float BASE_HZ        = 130.81; // C3, à l'octave 0

// ribbon 1 = touches blanches, ribbon 2 = touches noires (demi-tons au-dessus de C)
static const int WHITE_NOTES[] = {0, 2, 4, 5, 7, 9, 11};   // C D E F G A B
static const int BLACK_NOTES[] = {1, 3, 6, 8, 10};         // C# D# F# G# A#

struct Ribbon {
    uint8_t       pin;
    const int*    notes;
    int           num_notes;
    int           raw;
    bool          active;
    float         smoothed;
    float         pos;        // 0..1
    int           zone;       // index dans notes[]
    unsigned long touch_time; // pour savoir quel ribbon a été touché en dernier
};

static Ribbon ribbons[] = {
    {A0, WHITE_NOTES, 7, 0, false, 0, 0, 0, 0},   // pin 14
    {A1, BLACK_NOTES, 5, 0, false, 0, 0, 0, 0},   // pin 15
};

static Encoder enc(ENC_A, ENC_B);
static long    last_enc_pos = 0;
static bool    last_sw = HIGH;
static unsigned long last_sw_time = 0;
static unsigned long last_print = 0;
static bool    last_btn[3] = {HIGH, HIGH, HIGH};
static unsigned long last_btn_time[3] = {0, 0, 0};

static void ribbon_update(Ribbon& r) {
    r.raw = analogRead(r.pin);

    bool was_active = r.active;
    if (!r.active && r.raw > RIBBON_ON)  r.active = true;
    if ( r.active && r.raw < RIBBON_OFF) r.active = false;
    if (!r.active) return;

    // au premier contact on saute direct à la position, ensuite on lisse
    if (!was_active) r.smoothed = r.raw;
    else             r.smoothed += (r.raw - r.smoothed) * 0.2f;
    r.pos = constrain((r.smoothed - RIBBON_MIN) / float(RIBBON_MAX - RIBBON_MIN), 0.0f, 1.0f);

    // découpage en zones, avec hystérésis pour ne pas sautiller entre deux notes
    float x = r.pos * r.num_notes;
    if (!was_active || x < r.zone - ZONE_HYST || x > r.zone + 1 + ZONE_HYST) {
        r.zone = constrain(int(x), 0, r.num_notes - 1);
    }
    if (!was_active) r.touch_time = millis();
}

void controls_setup() {
    pinMode(ENC_SW, INPUT_PULLUP);
    for (int i = 0; i < 3; i++) {
        pinMode(OSC_BTN[i], INPUT_PULLUP);
        pinMode(OSC_LED[i], OUTPUT);
        digitalWrite(OSC_LED[i], state.osc_on[i]);
    }
    for (Ribbon& r : ribbons) pinMode(r.pin, INPUT_DISABLE);
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

    // Boutons → allume / éteint chaque osc
    for (int i = 0; i < 3; i++) {
        bool b = digitalRead(OSC_BTN[i]);
        if (b == LOW && last_btn[i] == HIGH && millis() - last_btn_time[i] > DEBOUNCE_MS) {
            state.osc_on[i] = !state.osc_on[i];
            digitalWrite(OSC_LED[i], state.osc_on[i]);
            state.osc_dirty = true;
            state.ui_dirty = true;
            last_btn_time[i] = millis();
        }
        last_btn[i] = b;
    }

    // Ribbons → note (monophonique : le dernier ribbon touché gagne)
    for (Ribbon& r : ribbons) ribbon_update(r);
    state.ribbon_raw = ribbons[0].raw;

    Ribbon* played = nullptr;
    for (Ribbon& r : ribbons) {
        if (r.active && (!played || r.touch_time > played->touch_time)) played = &r;
    }

    bool was_active = state.ribbon_active;
    state.ribbon_active = (played != nullptr);
    if (played) {
        int note = played->notes[played->zone];
        if (note != state.note) state.ui_dirty = true;
        state.note        = note;
        state.ribbon_pos  = played->pos;
        state.ribbon_freq = BASE_HZ * powf(2.0f, state.octave + note / 12.0f);
    }
    if (was_active != state.ribbon_active) state.ui_dirty = true;

    // Debug série : sert à calibrer RIBBON_MIN / MAX
    if (millis() - last_print > 200) {
        Serial.print("RIB1="); Serial.print(ribbons[0].raw);
        Serial.print(" RIB2="); Serial.print(ribbons[1].raw);
        Serial.print(" ON=");  Serial.print(state.ribbon_active);
        Serial.print(" NOTE=");Serial.print(state.note_names[state.note]);
        Serial.print(" HZ=");  Serial.print(state.ribbon_freq);
        Serial.print(" OCT="); Serial.print(state.octave);
        Serial.print(" WAVE=");Serial.print(state.wave_names[state.wave]);
        // BTN : 0 = bouton appuyé, 1 = relâché (pull-up) ; OSC : 1 = osc allumé
        Serial.print(" BTN=");
        for (int i = 0; i < 3; i++) Serial.print(digitalRead(OSC_BTN[i]));
        Serial.print(" OSC=");
        for (int i = 0; i < 3; i++) Serial.print(state.osc_on[i]);
        // diagnostic audio : MEM doit rester < 32 (AudioMemory), CPU < 100
        Serial.print(" MEM="); Serial.print(AudioMemoryUsageMax());
        Serial.print(" CPU="); Serial.println(AudioProcessorUsageMax());
        last_print = millis();
    }
}

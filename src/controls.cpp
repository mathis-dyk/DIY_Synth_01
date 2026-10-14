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
// LED sequins des oscs (résistance 100Ω intégrée)
static const uint8_t OSC_LED[3] = {10, 11, 12};
static const unsigned long DEBOUNCE_MS = 30;

// potards : 3.3V / GND / curseur sur la pin
static const uint8_t POT_CUTOFF   = A2;   // pin 16
static const uint8_t POT_RESO     = A3;   // pin 17
static const uint8_t POT_ENV      = A8;   // pin 22
static const int     POT_DEADBAND = 12;   // écart mini (sur 0..1023) pour considérer que le potard a bougé
static const float   POT_SMOOTH   = 0.15f; // lissage : plus petit = plus stable mais plus lent
static const unsigned long POT_INTERVAL_MS = 5;   // lecture des potards toutes les 5 ms

static const char* OSC_NAMES[3]  = {"OSC 1", "OSC 2", "SUB"};
static const char* LEVEL_NAMES[3] = {"LEVEL 1", "LEVEL 2", "LEVEL S"};
static const float MAX_OSC_LEVEL = 2.0f;   // volume d'un osc : jusqu'à 200 %
static const char* WAVE_NAMES[3] = {"WAVE 1", "WAVE 2", "WAVE S"};

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

// bouton avec anti-rebond ; `used` = il a servi de "shift" avec l'encodeur,
// donc son relâchement ne compte pas comme un appui court
struct Button {
    uint8_t       pin;
    bool          down;
    bool          last_raw;
    unsigned long last_change;
    bool          used;
};

struct Pot {
    uint8_t pin;
    int     value;    // dernière valeur retenue (0..1023), -1 = jamais lue
    float   smoothed; // lecture lissée
};

enum PotChange { POT_SAME, POT_FIRST, POT_MOVED };

static Encoder enc(ENC_A, ENC_B);
static long    last_enc_pos = 0;
static unsigned long last_print = 0;

static const unsigned long DOUBLE_CLICK_MS = 300;
static bool          pending_click = false;   // un clic attend de savoir s'il sera double
static unsigned long pending_click_time = 0;
static unsigned long hold_time = 0;          // moment où le HOLD a été activé

static Button enc_btn = {ENC_SW, false, HIGH, 0, false};
static Button osc_btn[3] = {
    {OSC_BTN[0], false, HIGH, 0, false},
    {OSC_BTN[1], false, HIGH, 0, false},
    {OSC_BTN[2], false, HIGH, 0, false},
};
static Pot pot_cutoff = {POT_CUTOFF, -1, 0};
static Pot pot_reso   = {POT_RESO,   -1, 0};
static Pot pot_env    = {POT_ENV,    -1, 0};

// affiche "nom / valeur" en gros à l'écran pendant un moment
static void popup(const char* name, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(state.popup_value, sizeof(state.popup_value), fmt, args);
    va_end(args);
    state.popup_name = name;
    state.popup_time = millis();
    state.ui_dirty = true;
}

// renvoie +1 à l'appui, -1 au relâchement, 0 sinon
static int button_update(Button& b) {
    bool raw = digitalRead(b.pin);
    if (raw != b.last_raw) {
        b.last_raw = raw;
        b.last_change = millis();
    }
    bool down = (raw == LOW);
    if (down != b.down && millis() - b.last_change > DEBOUNCE_MS) {
        b.down = down;
        if (down) { b.used = false; return 1; }
        return -1;
    }
    return 0;
}

static PotChange pot_update(Pot& p) {
    int raw = analogRead(p.pin);
    bool first = (p.value < 0);
    if (first) p.smoothed = raw;
    else       p.smoothed += (raw - p.smoothed) * POT_SMOOTH;

    // les butées sont "aimantées" pour atteindre 0 et 1023 malgré la zone morte
    int v = int(p.smoothed + 0.5f);
    if (v < POT_DEADBAND)        v = 0;
    if (v > 1023 - POT_DEADBAND) v = 1023;

    if (!first && v != 0 && v != 1023 && abs(v - p.value) <= POT_DEADBAND) return POT_SAME;
    if (!first && v == p.value) return POT_SAME;
    p.value = v;
    return first ? POT_FIRST : POT_MOVED;   // la 1re lecture règle le son sans popup
}

// interpolation en proportions : t=0 → a, t=1 → b (pour les fréquences et les temps)
static float exp_lerp(float a, float b, float t) {
    return a * powf(b / a, t);
}

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
    pinMode(POT_CUTOFF, INPUT_DISABLE);
    pinMode(POT_RESO,   INPUT_DISABLE);
    pinMode(POT_ENV,    INPUT_DISABLE);
    analogReadAveraging(8);
}

// ---- Menu réglages : clic = réglage suivant (après le dernier, le menu se ferme), rotation = valeur ----
enum MenuItem { M_VOLUME, M_LEVEL1, M_LEVEL2, M_LEVELS, M_GLIDE, M_TRANSPOSE, M_PENV_AMT, M_PENV_TIME, MENU_COUNT };
static const char* MENU_NAMES[MENU_COUNT] = {"VOLUME", "LEVEL 1", "LEVEL 2", "LEVEL S", "GLIDE", "TRANSPOSE",
                                             "PITCH ENV", "PITCH TIME"};
static const unsigned long MENU_TIMEOUT_MS = 6000;
static unsigned long menu_time = 0;   // dernier geste dans le menu

int menu_count() { return MENU_COUNT; }

// texte de la valeur du réglage affiché (le nom est dans MENU_NAMES)
void menu_value(char* buf, int len) {
    switch (state.menu_index) {
        case M_VOLUME: snprintf(buf, len, "%d %%", int(state.volume * 100 + 0.5f)); break;
        case M_LEVEL1:
        case M_LEVEL2:
        case M_LEVELS: {
            int i = state.menu_index - M_LEVEL1;
            snprintf(buf, len, "%d %%", int(state.osc_level[i] * 100 + 0.5f));
            break;
        }
        case M_GLIDE:
            if (state.glide < 1) snprintf(buf, len, "OFF");
            else                 snprintf(buf, len, "%d ms", int(state.glide + 0.5f));
            break;
        case M_TRANSPOSE: snprintf(buf, len, "%+d", state.transpose); break;
        case M_PENV_AMT:
            if (state.pitch_env_amt < 1) snprintf(buf, len, "OFF");
            else                         snprintf(buf, len, "+%d st", int(state.pitch_env_amt + 0.5f));
            break;
        case M_PENV_TIME: snprintf(buf, len, "%d ms", int(state.pitch_env_time + 0.5f)); break;
        default: buf[0] = 0;
    }
}

const char* menu_name() {
    return state.menu_index >= 0 ? MENU_NAMES[state.menu_index] : "";
}

static void menu_click() {
    state.menu_index++;
    if (state.menu_index >= MENU_COUNT) state.menu_index = -1;   // après le dernier : on sort
    menu_time = millis();
    state.popup_time = 0;   // le menu s'affiche tout de suite, sans attendre la fin d'un popup
    state.ui_dirty = true;
}

static void menu_turn(int delta) {
    switch (state.menu_index) {
        case M_VOLUME: state.volume = constrain(state.volume + delta * 0.05f, 0.0f, 1.0f); break;
        case M_LEVEL1:
        case M_LEVEL2:
        case M_LEVELS: {
            int i = state.menu_index - M_LEVEL1;
            state.osc_level[i] = constrain(state.osc_level[i] + delta * 0.05f, 0.0f, MAX_OSC_LEVEL);
            break;
        }
        case M_GLIDE:
            // pas proportionnel : fin vers 0, rapide vers 1 s
            if      (state.glide < 1 && delta > 0) state.glide = 10;
            else if (state.glide <= 10 && delta < 0) state.glide = 0;
            else    state.glide = constrain(state.glide * powf(1.25f, delta), 0.0f, 1000.0f);
            break;
        case M_TRANSPOSE: state.transpose = constrain(state.transpose + delta, -12, 12); break;
        case M_PENV_AMT:  state.pitch_env_amt = constrain(state.pitch_env_amt + delta, 0.0f, 48.0f); break;
        case M_PENV_TIME: state.pitch_env_time = constrain(state.pitch_env_time * powf(1.2f, delta), 5.0f, 500.0f); break;
    }
    menu_time = millis();
    state.sound_dirty = true;
    state.ui_dirty = true;
}

static void change_wave(int i, int delta) {
    int n = state.num_waveforms;
    state.wave[i] = ((state.wave[i] + delta) % n + n) % n;
    popup(WAVE_NAMES[i], "%s", state.wave_names[state.wave[i]]);
}

// LED de l'osc verrouillé (edit_target) : clignote ; les autres : allumée = osc actif
static void update_leds() {
    bool blink = (millis() / 200) % 2;
    for (int i = 0; i < 3; i++) {
        bool on = state.osc_on[i];
        if (i == state.edit_target && blink) on = !on;
        digitalWrite(OSC_LED[i], on);
    }
}

static void update_buttons_and_encoder() {
    // boutons des oscs : appui court = on/off (au relâchement, sauf s'ils ont servi de shift)
    for (int i = 0; i < 3; i++) {
        if (button_update(osc_btn[i]) == -1 && !osc_btn[i].used) {
            state.osc_on[i] = !state.osc_on[i];
            state.sound_dirty = true;
            popup(OSC_NAMES[i], state.osc_on[i] ? "ON" : "OFF");
        }
    }

    int enc_event = button_update(enc_btn);

    // bouton d'osc maintenu + clic encodeur → verrouille l'encodeur sur la forme d'onde
    // de cet osc : on peut ensuite la changer d'une main en gardant l'autre sur le ribbon
    if (enc_event == 1) {
        for (int i = 0; i < 3; i++) {
            if (osc_btn[i].down) {
                state.edit_target = i;
                osc_btn[i].used = true;
                enc_btn.used = true;
                popup(WAVE_NAMES[i], "%s", state.wave_names[state.wave[i]]);
            }
        }
    }

    // clic seul (sans rotation) : on attend DOUBLE_CLICK_MS pour savoir si c'est un double clic
    //  - double clic → HOLD : la note jouée reste tenue après avoir lâché le ribbon
    //  - simple clic → déverrouille : l'encodeur revient sur l'octave
    if (enc_event == -1 && !enc_btn.used) {
        if (pending_click && millis() - pending_click_time < DOUBLE_CLICK_MS) {
            pending_click = false;
            state.hold = !state.hold;
            hold_time = millis();
            state.ui_dirty = true;
            popup("HOLD", state.hold ? "ON" : "OFF");
        } else {
            pending_click = true;
            pending_click_time = millis();
        }
    }
    // simple clic : déverrouille la forme d'onde si besoin, sinon ouvre le menu / réglage suivant
    if (pending_click && millis() - pending_click_time >= DOUBLE_CLICK_MS) {
        pending_click = false;
        if (state.edit_target >= 0) {
            state.edit_target = -1;
            popup("OCTAVE", "%+d", state.octave);
        } else {
            menu_click();
        }
    }

    // menu fermé tout seul après un moment sans y toucher
    if (state.menu_index >= 0 && millis() - menu_time > MENU_TIMEOUT_MS) {
        state.menu_index = -1;
        state.ui_dirty = true;
    }

    long pos = enc.read() / 4;
    int delta = pos - last_enc_pos;
    if (delta == 0) return;
    last_enc_pos = pos;

    // bouton d'osc maintenu + rotation → forme d'onde de cet osc (en boucle)
    for (int i = 0; i < 3; i++) {
        if (osc_btn[i].down) {
            osc_btn[i].used = true;
            change_wave(i, delta);
            return;
        }
    }

    // clic maintenu + rotation → detune
    if (enc_btn.down) {
        state.detune = constrain(state.detune + delta * 0.02f, 0.0f, 1.0f);
        enc_btn.used = true;
        popup("DETUNE", "%.2f st", state.detune);
        return;
    }

    // rotation seule → réglage du menu s'il est ouvert, sinon forme d'onde verrouillée, sinon octave
    if (state.menu_index >= 0) {
        menu_turn(delta);
        return;
    }
    if (state.edit_target >= 0) {
        change_wave(state.edit_target, delta);
        return;
    }
    state.octave = constrain(state.octave + delta, -2, 2);
    popup("OCTAVE", "%+d", state.octave);
}

// Potards à 2 couches :
//  - normal                     : CUTOFF  / RESO  / RELEASE
//  - clic encodeur maintenu     : ATTACK  / DECAY / SUSTAIN
// Un réglage ne bouge que quand on tourne son potard (la valeur saute alors à la position du potard).
static void update_pots() {
    bool shift = enc_btn.down;
    bool shift_used = false;   // un potard a été tourné pendant que le clic était maintenu

    // bouton d'osc maintenu + n'importe quel potard → volume de cet osc (0..200 %)
    int held = -1;
    for (int i = 0; i < 3; i++) if (osc_btn[i].down) held = i;
    if (held >= 0) {
        Pot* pots[3] = {&pot_cutoff, &pot_reso, &pot_env};
        for (Pot* p : pots) {
            if (pot_update(*p) == POT_MOVED) {
                state.osc_level[held] = p->value / 1023.0f * MAX_OSC_LEVEL;
                osc_btn[held].used = true;   // pas d'on/off au relâchement
                state.sound_dirty = true;
                popup(LEVEL_NAMES[held], "%d %%", int(state.osc_level[held] * 100 + 0.5f));
            }
        }
        return;
    }
    PotChange c;

    if ((c = pot_update(pot_cutoff)) != POT_SAME) {
        float x = pot_cutoff.value / 1023.0f;
        if (shift && c == POT_MOVED) {
            shift_used = true;
            state.attack = exp_lerp(1, 2000, x);
            popup("ATTACK", "%.0f ms", state.attack);
        } else {
            state.cutoff = exp_lerp(50, 12000, x);
            if (c == POT_MOVED) popup("CUTOFF", "%.0f Hz", state.cutoff);
        }
        state.sound_dirty = true;
    }

    if ((c = pot_update(pot_reso)) != POT_SAME) {
        float x = pot_reso.value / 1023.0f;
        if (shift && c == POT_MOVED) {
            shift_used = true;
            state.decay = exp_lerp(10, 2000, x);
            popup("DECAY", "%.0f ms", state.decay);
        } else {
            state.resonance = 0.7f + x * (5.0f - 0.7f);
            if (c == POT_MOVED) popup("RESO", "%.1f", state.resonance);
        }
        state.sound_dirty = true;
    }

    if ((c = pot_update(pot_env)) != POT_SAME) {
        float x = pot_env.value / 1023.0f;
        if (shift && c == POT_MOVED) {
            shift_used = true;
            state.sustain = x;
            popup("SUSTAIN", "%d %%", int(x * 100 + 0.5f));
        } else {
            state.release = exp_lerp(10, 3000, x);
            if (c == POT_MOVED) popup("RELEASE", "%.0f ms", state.release);
        }
        state.sound_dirty = true;
    }

    // un potard tourné pendant que le clic est maintenu : ce n'est plus un clic (pas de menu au relâchement)
    if (shift_used) enc_btn.used = true;
}

void controls_update() {
    update_buttons_and_encoder();
    update_leds();
    // potards lus à intervalle régulier : le lissage ne dépend pas de la vitesse de la boucle
    static unsigned long last_pot_read = 0;
    if (millis() - last_pot_read >= POT_INTERVAL_MS) {
        last_pot_read = millis();
        update_pots();
    }

    // Ribbons → note (monophonique : le dernier ribbon touché gagne)
    for (Ribbon& r : ribbons) ribbon_update(r);
    state.ribbon_raw = ribbons[0].raw;

    Ribbon* played = nullptr;
    for (Ribbon& r : ribbons) {
        if (r.active && (!played || r.touch_time > played->touch_time)) played = &r;
    }

    // une nouvelle touche posée après l'activation du HOLD le désactive
    if (state.hold && played && played->touch_time > hold_time) {
        state.hold = false;
        state.ui_dirty = true;
    }

    bool was_active = state.ribbon_active;
    state.ribbon_active = (played != nullptr) || state.hold;   // HOLD : la dernière note reste jouée
    if (played) {
        int note = played->notes[played->zone];
        if (note != state.note) state.ui_dirty = true;
        state.note        = note;
        state.ribbon_pos  = played->pos;
    }
    // recalculé même en HOLD, pour que l'octave suive l'encodeur
    state.ribbon_freq = BASE_HZ * powf(2.0f, state.octave + (state.note + state.transpose) / 12.0f);
    if (was_active != state.ribbon_active) state.ui_dirty = true;

    // Debug série
    if (millis() - last_print > 200) {
        Serial.print("RIB1="); Serial.print(ribbons[0].raw);
        Serial.print(" RIB2="); Serial.print(ribbons[1].raw);
        Serial.print(" POT="); Serial.print(pot_cutoff.value);
        Serial.print("/");     Serial.print(pot_reso.value);
        Serial.print("/");     Serial.print(pot_env.value);
        Serial.print(" NOTE=");Serial.print(state.note_names[state.note]);
        Serial.print(" OCT="); Serial.print(state.octave);
        Serial.print(" WAVE=");
        for (int i = 0; i < 3; i++) { Serial.print(state.wave_names[state.wave[i]]); Serial.print(i < 2 ? "/" : ""); }
        Serial.print(" OSC=");
        for (int i = 0; i < 3; i++) Serial.print(state.osc_on[i]);
        // diagnostic audio : MEM doit rester < 32 (AudioMemory), CPU < 100
        Serial.print(" MEM="); Serial.print(AudioMemoryUsageMax());
        Serial.print(" CPU="); Serial.println(AudioProcessorUsageMax());
        last_print = millis();
    }
}

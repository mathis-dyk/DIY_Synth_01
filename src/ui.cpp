#include "ui.h"           // toujours en premier
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "controls.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// SSD1309 2.42" : SWITCHCAPVCC + 0x3C (voir memory/project_display.md)
// I2C à 100 kHz (au lieu de 400 kHz par défaut) : plus tolérant aux fils de breadboard
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 100000UL, 100000UL);

static unsigned long last_draw = 0;
static bool display_ok = false;
static const unsigned long POPUP_MS = 1000;   // durée d'affichage d'un réglage touché

void ui_setup() {
    // sans écran on continue quand même (le son doit marcher sans UI)
    display_ok = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    if (!display_ok) {
        Serial.println(F("ECRAN NON TROUVE"));
        return;
    }
    Serial.println(F("ECRAN TROUVE"));
    // contraste réduit (défaut 0xCF) : l'écran tire moins sur le 3.3V de la Teensy
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(0x30);
    display.setTextWrap(false);   // une ligne trop longue est coupée au bord au lieu de déborder dessous
    display.clearDisplay();
    display.display();
}

void ui_update() {
    if (!display_ok) return;

    // un réglage vient d'être touché → on l'affiche en gros pendant POPUP_MS
    bool popup = state.popup_time && millis() - state.popup_time < POPUP_MS;
    static bool was_popup = false;
    if (was_popup && !popup) state.ui_dirty = true;   // retour à la vue d'ensemble
    was_popup = popup;

    // redessiner l'I2C est lent : max ~20 fps si ça bouge, sinon 2 fps.
    // Le rafraîchissement au repos contourne l'écran qui perd son image tout seul.
    bool playing = state.ribbon_active;
    unsigned long interval = (state.ui_dirty || playing) ? 50 : 500;
    if (millis() - last_draw < interval) return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    if (popup) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(state.popup_name);

        // taille 3 = 7 caractères max sur 128 px ; au-delà ("12000 Hz") on passe en taille 2
        display.setTextSize(strlen(state.popup_value) > 7 ? 2 : 3);
        display.setCursor(0, 18);
        display.print(state.popup_value);
    } else if (state.menu_index >= 0) {
        // menu réglages : nom + position, valeur en gros, aide en bas
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("MENU ");
        display.print(state.menu_index + 1);
        display.print("/");
        display.print(menu_count());
        display.print("  ");
        display.print(menu_name());

        char value[16];
        menu_value(value, sizeof(value));
        display.setTextSize(3);
        display.setCursor(0, 16);
        display.print(value);

        display.setTextSize(1);
        display.setCursor(0, 46);
        display.print("rot:regle  clic:suite");   // 21 car. = 126 px
    } else {
        // ligne du haut : forme d'onde de chaque osc, "---" s'il est éteint
        const char* labels[3] = {"1", "2", "S"};
        display.setTextSize(1);
        // l'osc verrouillé sur l'encodeur est affiché en inversé (blanc sur noir → noir sur blanc)
        for (int i = 0; i < 3; i++) {
            bool locked = (i == state.edit_target);
            display.setTextColor(locked ? SSD1306_BLACK : SSD1306_WHITE,
                                 locked ? SSD1306_WHITE : SSD1306_BLACK);
            display.setCursor(i * 44, 0);
            display.print(labels[i]);
            display.print(":");
            display.print(state.osc_on[i] ? state.wave_names[state.wave[i]] : "---");
        }
        display.setTextColor(SSD1306_WHITE);

        // note jouée en gros (ex. "C#3"), octave et fréquence à côté
        display.setTextSize(2);
        display.setCursor(0, 14);
        if (playing) {
            // note réellement entendue, transposition comprise (peut changer d'octave)
            int n = state.note + state.transpose;
            int oct_shift = (n < 0) ? -1 : n / 12;
            display.print(state.note_names[(n % 12 + 12) % 12]);
            display.print(3 + state.octave + oct_shift);
        } else {
            display.print("--");
        }
        display.setTextSize(1);
        display.setCursor(64, 14);
        display.print("OCT ");
        if (state.octave > 0) display.print("+");
        display.print(state.octave);
        if (state.hold) {
            display.setCursor(104, 14);
            display.print("HOLD");
        }
        if (playing) {
            display.setCursor(64, 24);
            display.print(state.ribbon_freq, 1);
            display.print(" Hz");
        }

        // réglages des potards
        display.setCursor(0, 36);
        display.print("CUT ");
        display.print(state.cutoff, 0);
        display.setCursor(64, 36);
        display.print("RES ");
        display.print(state.resonance, 1);
        // enveloppe : A/D/R en ms, S en %
        char adsr[32];
        snprintf(adsr, sizeof(adsr), "A%d D%d S%d R%d",
                 int(state.attack + 0.5f), int(state.decay + 0.5f),
                 int(state.sustain * 100 + 0.5f), int(state.release + 0.5f));
        display.setCursor(0, 46);
        display.print(adsr);
    }

    // barre de position du ribbon
    int w = state.ribbon_pos * SCREEN_WIDTH;   // même calibration que le pitch
    display.drawRect(0, 57, SCREEN_WIDTH, 7, SSD1306_WHITE);
    if (playing) display.fillRect(0, 57, w, 7, SSD1306_WHITE);

    display.display();
    last_draw = millis();
    state.ui_dirty = false;
}

#include "ui.h"           // toujours en premier
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// SSD1309 2.42" : SWITCHCAPVCC + 0x3C (voir memory/project_display.md)
// I2C à 100 kHz (au lieu de 400 kHz par défaut) : plus tolérant aux fils de breadboard
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 100000UL, 100000UL);

static unsigned long last_draw = 0;
static bool display_ok = false;

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
    display.clearDisplay();
    display.display();
}

void ui_update() {
    if (!display_ok) return;

    // redessiner l'I2C est lent : max ~20 fps si ça bouge, sinon 2 fps.
    // Le rafraîchissement au repos contourne l'écran qui perd son image tout seul.
    bool playing = state.ribbon_active;
    unsigned long interval = (state.ui_dirty || playing) ? 50 : 500;
    if (millis() - last_draw < interval) return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(state.wave_names[state.wave]);

    display.setTextSize(1);
    display.setCursor(80, 4);
    display.print("OCT ");
    if (state.octave > 0) display.print("+");
    display.print(state.octave);

    display.setCursor(0, 28);
    if (playing) {
        display.print(state.ribbon_freq, 1);
        display.print(" Hz");
    } else {
        display.print("touche le ribbon");
    }

    // barre de position du ribbon
    int w = map(constrain(state.ribbon_raw, 0, 1023), 0, 1023, 0, SCREEN_WIDTH);
    display.drawRect(0, 50, SCREEN_WIDTH, 10, SSD1306_WHITE);
    if (playing) display.fillRect(0, 50, w, 10, SSD1306_WHITE);

    display.display();
    last_draw = millis();
    state.ui_dirty = false;
}

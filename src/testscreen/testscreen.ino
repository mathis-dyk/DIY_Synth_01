#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(9600);
  Serial.println("Demarrage...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Ecran non trouve !");
    return;
  }

  delay(500);
  Serial.println("Ecran OK");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 24);
  display.println("Hello World");
  display.display();
  Serial.println("Affichage OK");
}

void loop() {}

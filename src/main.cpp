#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Audio chain
AudioSynthWaveform       osc1;
AudioFilterStateVariable filter1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(osc1, 0, filter1, 0);
AudioConnection          patchCord2(filter1, 0, i2s1, 0);
AudioConnection          patchCord3(filter1, 0, i2s1, 1);

// Define the OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create an SSD1306 display object (I2C address 0x3C)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Waveforms
const int NUM_WAVEFORMS = 4;
const int waveforms[NUM_WAVEFORMS] = {WAVEFORM_SINE, WAVEFORM_SAWTOOTH, WAVEFORM_SQUARE, WAVEFORM_TRIANGLE};
const char* waveNames[NUM_WAVEFORMS] = {"SINE", "SAW", "SQR", "TRI"};
int currentWaveform = 0;
float currentFreq = 440;

// Encodeur
Encoder myEnc(2, 3);
long lastEncPos = 0;

// Bouton
bool lastBtn = HIGH;
unsigned long lastDebounce = 0;

// Octave
int currentOctave = 4;
const float BASE_FREQ = 32.70; // C1

void setup() {
  Serial.begin(9600);
  pinMode(4, INPUT_PULLUP);
  AudioMemory(64);
  osc1.begin(0.9, 440, waveforms[currentWaveform]);
  filter1.frequency(1000);
  filter1.resonance(0.7);

  // Initialize the OLED display
  if (!display.begin(SSD1306_PAGEADDR, 0x3C)) {
    Serial.println(F("SSD1309 initialization failed!"));
    while (true); // Halt execution if initialization fails
  }

  // Clear the display buffer
  display.clearDisplay();

  // Set text size, color, and cursor position
  display.setTextSize(1); // Text size multiplier
  display.setTextColor(SSD1306_WHITE); // White text
  display.setCursor(0, 0); // Top-left corner

  // Display a message
  display.println(F("Hello, OLED!"));
  display.display(); // Render the text on the screen
}

void loop() {
  // --- Encodeur → octave ---
  long encPos = myEnc.read() / 4;
  long delta = encPos - lastEncPos;
  if (delta != 0) {
    currentOctave = constrain(currentOctave + delta, 1, 8);
    lastEncPos = encPos;
    Serial.print("Octave: ");
    Serial.println(currentOctave);
  }

  // --- Bouton → forme d'onde avec debounce ---
  bool btn = digitalRead(4);
  if (btn == LOW && lastBtn == HIGH && millis() - lastDebounce > 200) {
    currentWaveform = (currentWaveform + 1) % NUM_WAVEFORMS;
    osc1.begin(0.9, currentFreq, waveforms[currentWaveform]);
    Serial.print("Wave: ");
    Serial.println(waveNames[currentWaveform]);
    lastDebounce = millis();
  }
  lastBtn = btn;

  // --- Fréquence via encodeur uniquement ---
  float freqMin = BASE_FREQ * pow(2, currentOctave - 1);
  osc1.amplitude(0.9);
  osc1.frequency(freqMin);
  currentFreq = freqMin;

  // --- Print potard A2 ---
  int raw = analogRead(A2);
  Serial.print("Potard A2=");
  Serial.println(raw);

  delay(200); // plus lent pour lire confortablement
}

// map() pour les floats
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

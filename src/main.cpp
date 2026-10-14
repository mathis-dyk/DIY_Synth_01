
#include <Audio.h>
#include "ui.h"
#include "controls.h"
#include "engine.h"

void setup() {
  Serial.begin(96000);

  ui_setup();
  controls_setup();
  engine_setup();
}

void loop() {
  controls_update();
  ui_update();
  engine_update();
}

---
name: project-display
description: Display hardware info — SSD1309 2.42" OLED, working Adafruit SSD1306 config
metadata:
  type: project
---

The project uses a **2.42 inch SSD1309 OLED display** (128x64), module marked "2.42 OLED IIC 1.1", 4 pins. It has an onboard chip marked "662K" (likely an XC6206 3.3V LDO), so VCC is probably meant for 5V. Before using 5V, check that SDA/SCL idle at ~3.3V (not 5V) with VCC at 5V and the module disconnected from the Teensy.

It works with the Adafruit SSD1306 library using:
- Address: `0x3C`
- Mode: `SSD1306_SWITCHCAPVCC`
- Wiring: VCC→**Teensy VIN (5V)**, through the onboard 662K regulator (works, 2026-10-14; on 3.3V the LDO dropped out and the screen kept losing its image), GND→GND, SDA→pin 18, SCL→pin 19 (Teensy 4.1 Wire)
- Code: I2C at 100 kHz, plus a redraw every 500 ms when idle (`src/ui.cpp`)

On the shared + rail, the screen lost its image after a few seconds (2026-10-14). A dedicated 3.3V pin delayed the problem but did not fix it. Suspected cause: total draw near the limit of the Teensy 3.3V regulator (~250 mA); contrast reduced to 0x30 in code. If that is not enough, the next step is an external 3.3V regulator (AMS1117) fed from VIN for the OLED + DAC.

**Why:** SSD1309 is partially compatible with Adafruit SSD1306 library. `SSD1306_EXTERNALVCC` and address `0x3D` were tested and did NOT work.

**How to apply:** Always use `SSD1306_SWITCHCAPVCC` and `0x3C` when initializing the display. Do not use `SSD1306_PAGEADDR` (invalid vcc mode used in early code).

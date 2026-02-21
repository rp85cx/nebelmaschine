/*
v2.5
Board library ESP32 by Espressif System <= 2.0.17 
ich hoffe du verstehst denglisch

"kleines" Projekt um eine Nebelmaschine fast smart zu machen
*/
#include <U8g2lib.h>
#include <RotaryEncoder.h>
#include <OneButton.h>
#include <Preferences.h>
#include <max6675.h>
#include <esp_dmx.h>

#include "config.h"
#include "functions.h"

void setup() {
  if (debugMode) Serial.begin(115200);

  initPins();
  setInitalState();
  initLibs();

  xTaskCreatePinnedToCore(code0, "core0", 10000, NULL, 2, NULL, 0);
  delay(200);

  xTaskCreatePinnedToCore(code1, "core1", 10000, NULL, 1, NULL, 1);
  delay(200);
}


void code0(void* pvParameters) {
  while (true) {
    encoder.tick();        //update encoder
    encoderButton.tick();  //update encoder button

    encoderMenuScroll();

    checkTemp(true);  //temperatur check im interval  -> true = action call nach temp read falls menu damit

    checkInactivity();

    systemControl();

    timerControl();

    dmxInPrefs();

    if (action) {
      action = false;

      calcShownItems();
      drawScreen();
    }

    pageFunctions();
  }
}


void code1(void* pvParameters) {
  while (true) {
    ledControl();

    dmxControl(true);
  }
}


void loop() {}
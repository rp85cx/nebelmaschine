//Board library ESP32 by Espressif System <= 2.0.17 
//foreshadowing: hier ist alles Denglisch, tut mir leid

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

  initLibs();
  initPins();

  xTaskCreatePinnedToCore(code0, "core0", 10000, NULL, 1, &core0, 0);
  delay(100);

  xTaskCreatePinnedToCore(code1, "core1", 10000, NULL, 1, &core1, 1);
  delay(100);

  setInitalState();
}


void code0(void* pvParameters) {
  for (;;) {
    encoder.tick();        //update encoder
    encoderButton.tick();  //update encoder button

    pageFunctions();

    encoderMenuScroll();

    if (action) {
      action = false;

      calcShownItems();
      drawScreen();
    }

    checkTemp(true);  //temperatur check im interval  -> true = action call nach temp read falls menu damit

    checkInactivity();

    systemControl();

    timerControl();
  }
}


void code1(void* pvParameters) {
  for (;;) {
    ledControl();
    dmxControl(true);
  }
}


void loop() {}
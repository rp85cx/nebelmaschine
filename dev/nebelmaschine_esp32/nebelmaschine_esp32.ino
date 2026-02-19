/*
v3.0 dev build

ISSUES: max6675 &/& wifi :(

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
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>

#include "config.h"
#include "functions.h"

void setup() {
  if (debugMode) Serial.begin(115200);

  initPins();
  setInitalState();
  initLibs();

  xTaskCreatePinnedToCore(code0, "core0", 10000, NULL, 2, NULL, 0);
  delay(500);

  xTaskCreatePinnedToCore(code1, "core1", 10000, NULL, 1, NULL, 1);
  delay(500);

  timer = timerBegin(0, 80, true); // 80 prescaler = 1MHz
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 500000, true); // 500ms
  timerAlarmEnable(timer);
}


void code0(void* pvParameters) {  //system critical
  while (true) {
    encoder.tick();        //update encoder
    encoderButton.tick();  //update encoder button

    encoderMenuScroll();

    checkTemp(true);  //temperatur check im interval  -> true = action call nach temp read falls menu damit

    checkInactivity();

    systemControl();

    timerControl();

    dmxInPrefs();
  }
}


void code1(void* pvParameters) { //not so critical
  while (true) {
    ledControl();
    dmxControl(true);

    if (preferences.getBool("wifiActive", false)) {
      server.handleClient();
      webpageCheckActivity();
    } else {
      foggingActiveWeb = false;
    }

    if (action) {
      action = false;

      calcShownItems();
      drawScreen();
    }

    pageFunctions();
  }
}


void loop() {}
#include "ui.h"

//-------helpers----------
void heat(bool active) {
  digitalWrite(relay_heatPin, !active ? HIGH : LOW);  //inverted relay
  relay_heat = active;
}

void pump(bool active) {
  digitalWrite(relay_pumpPin, !active ? HIGH : LOW);  //inverted relay
  relay_pump = active;
}

void webpageHandleIndex() {
  server.send_P(200, "text/html", webpage);
}

void webpageHandleButton() {
  if (!server.hasArg("state")) {
    server.send(400, "text/plain", "missing state");
    return;
  }
  if (server.arg("state") == "1") {
    foggingActiveWeb = true;
    lastPing = millis();
  } else {
    foggingActiveWeb = false;
  }
  server.send(200, "text/plain", "ok");
}

//-------------setups--------------
void initLibs() {
  u8g2.setI2CAddress(0x3C * 2);  //u8g2
  u8g2.begin();
  u8g2.setColorIndex(1);
  u8g2.setBitmapMode(1);
  u8g2.setFontMode(1);

  encoderButton.setLongPressIntervalMs(300);  //encoder

  preferences.begin("machine", false);  //preferences (save after reboot)

  dmx_config_t config = DMX_CONFIG_DEFAULT;  //dmx
  dmx_personality_t personalities[] = { { 1, "Default Personality" } };
  int personality_count = 1;
  dmx_driver_install(1, &config, personalities, personality_count);
  dmx_set_pin(1, dmxTX, dmxRX, dmxRTS);

  if (preferences.getBool("wifiActive", false)) {
    WiFi.softAP(AP_SSID, AP_PWD);
    if (debugMode) Serial.print("IP:  ");
    if (debugMode) Serial.println(WiFi.softAPIP());
    server.on("/", webpageHandleIndex);
    server.on("/button", webpageHandleButton);
    server.begin();
  }
}

void initPins() {
  pinMode(fluidSensorPin, INPUT);
  pinMode(triggerButton, INPUT);
  pinMode(relay_heatPin, OUTPUT);
  pinMode(relay_pumpPin, OUTPUT);
  pinMode(led_ready, OUTPUT);
  pinMode(led_rdyOut, OUTPUT);
  pinMode(led_fluidRed, OUTPUT);
  pinMode(led_fluidBlue, OUTPUT);

  ledcSetup(0, 5000, 12);
  ledcAttachPin(led_pwrHeat, 0);
}

void setInitalState() {
  heat(false);
  pump(false);
  digitalWrite(led_ready, LOW);
  digitalWrite(led_pwrHeat, LOW);
  digitalWrite(led_rdyOut, LOW);
  digitalWrite(led_fluidRed, LOW);
  digitalWrite(led_fluidBlue, LOW);
}

//-------system-------------
void encoderMenuScroll() {
  encoderButton.attachLongPressStart([]() {  //page back to menu press anywhere
    current_screen = 0;
    action = true;
  });
}

void calcShownItems() {
  item_sel_previous = menu_selected - 1;  //vorher nachher items tracking
  if (item_sel_previous < 0) {
    item_sel_previous = menu_itemsLength - 1;
  }
  item_sel_next = menu_selected + 1;
  if (item_sel_next >= menu_itemsLength) {
    item_sel_next = 0;
  }
}

void checkTemp(bool wantsAction) {
  unsigned long now = millis();
  if (((now - lastTempUpdate) >= tempCheckInterval)) {
    lastTempUpdate = now;
    temperature = thermocouple.readCelsius();
    Serial.println(temperature);
    if (wantsAction) {
      if (current_screen == 1 && (menu_selected == 7 || menu_selected == 3)) {
        action = true;
      }
    }
  }
}

void ledControl() {
  unsigned long now = millis();
  if (now - lastLedUpdate >= 500) {
    lastLedUpdate = now;
    ledState = !ledState;

    //heat/power led
    if (relay_heat) {
      ledcWrite(0, (4095 / 255) * min(255, 255));
    } else {
      ledcWrite(0, (4095 / 255) * min(led_dunkelPwm, 255));
    }

    //ready/output led
    if (relay_pump) {
      digitalWrite(led_rdyOut, ledState ? HIGH : LOW);
    } else if (ready) {
      digitalWrite(led_rdyOut, HIGH);
    } else {
      digitalWrite(led_rdyOut, LOW);
    }

    digitalWrite(led_ready, ready ? HIGH : LOW);  //only ready led

    if (digitalRead(fluidSensorPin)) {  //ik geht schöner aber da kommt iwann noch logic
      digitalWrite(led_fluidRed, LOW);
      digitalWrite(led_fluidBlue, HIGH);
    } else if (!digitalRead(fluidSensorPin)) {
      digitalWrite(led_fluidRed, HIGH);
      digitalWrite(led_fluidBlue, LOW);
    } else {
      digitalWrite(led_fluidRed, LOW);
      digitalWrite(led_fluidBlue, LOW);
    }
  }
}

void systemControl() {
  unsigned long now = millis();

  ready = ((temperature >= temp_ready) && (temperature < 1000)) ? true : false;

  if (((temperature <= temp_heatMax) || (relay_pump && temperature <= temp_pumpMax)) && preferences.getBool("heatActive", true) && temperature != 0) {
    if (now - lastHeatStateChange >= 2000) {
      lastHeatStateChange = now;
      heat(true);
    }
  } else {
    heat(false);
  }

  if (ready && foggingActive) pump(true);
  else pump(false);

  if (now - lastTriggerButtonCheck >= 50) {
    lastTriggerButtonCheck = now;
    if (!digitalRead(triggerButton)) {  //button is inverted
      foggingActiveButton = true;
      if (debugMode) Serial.println("button activated");
    } else {
      foggingActiveButton = false;
    }
  }

  /*Serial.printf("dmx:%d | button:%d | display:%d | timer:%d | web:%d |allowed:%d  |==> fogs:%d\n",  //troubleshooting help for trigger inputs
                foggingActiveDMX,
                foggingActiveButton,
                foggingActiveDisplay,
                foggingActiveTimer,
                foggingActiveWeb,
                foggingAllowed,
                foggingActive
  );*/

  if ((foggingActiveDMX || foggingActiveButton || foggingActiveDisplay || foggingActiveTimer || foggingActiveWeb) && foggingAllowed) {
    foggingActive = true;
  } else {
    foggingActive = false;
  }

  if (lastFoggingState != foggingActive) {
    lastFoggingState = foggingActive;
    foggingTime = now;
  }

  if (!foggingAllowed) {  //allow fogging again after quarter timeout time has passed
    if ((now - foggingTime) >= (maxFoggingTime * 1000 / 4)) {
      foggingAllowed = true;
    }
  }

  if (foggingActive) {  //disallow fogging if on for more than timeout time
    if ((now - foggingTime) >= (maxFoggingTime * 1000)) {
      foggingAllowed = false;
    }
  }
}

void dmxControl(bool wantsAction) {
  if (dmxActive) {
    dmx_packet_t packet;
    if (dmx_receive(1, &packet, DMX_TIMEOUT_TICK)) {
      if (!packet.err) {
        //dmx verfügbar:
        if (!dmxIsConnected) {
          if (debugMode) Serial.println("dmx connected");
          dmxIsConnected = true;
        }

        unsigned long now1 = millis();
        unsigned long now = millis();

        if (wantsAction && current_screen == 1 && (menu_selected == 6 || menu_selected == 4)) {
          calcDmxPPS++;
          if (now1 - lastDmxPPSUpdate >= 1000) {
            dmxPPS = calcDmxPPS;
            calcDmxPPS = 0;
            lastDmxPPSUpdate = now1;
            action = true;
          }
        }
        if (now - lastDmxUpdate > dmxUpdateInterval) {
          lastDmxUpdate = now;
          dmx_read(1, dmxData, packet.size);
          dmxValue = dmxData[preferences.getUInt("dmxAdress", 1)];
        }

        if ((dmxIsConnected && (preferences.getUInt("dmxMode", 1) == 1)) && dmxValue >= 127) {
          foggingActiveDMX = true;
          action = true;
        } else if ((dmxIsConnected && (preferences.getUInt("dmxMode", 1)) == 2) && dmxValue >= 127) {
          preferences.putBool("timerActive", true);
          action = true;
        } else if ((dmxIsConnected && (preferences.getUInt("dmxMode", 1)) == 2) && dmxValue < 127) {
          preferences.putBool("timerActive", false);
          action = true;
        } else {
          foggingActiveDMX = false;
        }
      } else {
        //dmx error
        if (debugMode) Serial.println("dmx error occurred");
      }
    }
  } else {
    foggingActiveDMX = false;
  }
}

void timerControl() {
  if (preferences.getBool("timerActive", false)) {
    unsigned long now = millis();
    if (((now - lastTimerUpdate) >= (preferences.getUInt("timerOff", 30) * 1000 * 60)) && !timerState) {
      lastTimerUpdate = now;
      timerState = true;
      foggingActiveTimer = true;
    } else if (((now - lastTimerUpdate) >= (preferences.getUInt("timerOn", 10) * 1000)) && timerState) {
      lastTimerUpdate = now;
      timerState = false;
      foggingActiveTimer = false;
    }
  } else {
    foggingActiveTimer = false;
  }
}

void checkInactivity() {
  unsigned long now = millis();

  if (current_screen != lastCurrent_screen) {
    lastCurrent_screen = current_screen;
    lastAction = millis();
  }
  if (menu_selected != lastMenu_selected) {
    lastMenu_selected = menu_selected;
    lastAction = millis();
  }

  if ((now - lastAction) >= (inactivityTime * 1000)) {  //ich saß hier bestimmt nd eine stunde dran inverted und home in einen timer loop zu packen um zu merken das das totdumm ist hahaha
    if (!(current_screen == 1 && menu_selected == 7)) {
      current_screen = 1;
      menu_selected = 7;
      action = true;
      if (debugMode) Serial.println("now on home");
    }
  }

  if (current_screen == 1 && menu_selected == 7 && (now - lastInvert) >= (inactivityTime * 1000 * 5)) {
    lastInvert = now;
    if (debugMode) Serial.println("now inverted");
    displayInverted = !displayInverted;
    action = true;
  }
}

void dmxInPrefs() {
  dmxActive = preferences.getBool("dmxActive", true);
}

void webpageCheckActivity() {
  if (foggingActiveWeb && millis() - lastPing > pingTimeout) {
    foggingActiveWeb = false;
    Serial.println("ping lost, foggingActiveWeb false");
  }
}
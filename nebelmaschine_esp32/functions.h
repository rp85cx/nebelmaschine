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
}

void initPins() {
  pinMode(fluidSensorPin, INPUT_PULLDOWN);  // 5V----Z---GPIO
  pinMode(triggerButton, INPUT_PULLUP);     // GND----Z---GPIO
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

//-------------------ui pages-------------------------------------
void draw_switch(int x, int y, bool toggledOn) {
  if (toggledOn) {
    u8g2.drawRFrame(x + 0, y + 0, 32, 16, 4);
    u8g2.drawRFrame(x + 17, y + 1, 14, 14, 5);
    u8g2.drawBox(x + 1, y + 2, 17, 12);
    u8g2.drawLine(x + 2, y + 1, x + 29, y + 1);
    u8g2.drawLine(x + 2, y + 14, x + 28, y + 14);
    u8g2.drawLine(x + 30, y + 2, x + 30, y + 13);
    u8g2.drawLine(x + 27, y + 1, x + 29, y + 1);
    u8g2.drawLine(x + 20, y + 2, x + 18, y + 2);
    u8g2.drawLine(x + 20, y + 13, x + 17, y + 13);
    u8g2.drawLine(x + 27, y + 2, x + 30, y + 2);
    u8g2.drawLine(x + 27, y + 13, x + 29, y + 13);
    u8g2.drawLine(x + 29, y + 2, x + 29, y + 4);
    u8g2.drawLine(x + 29, y + 11, x + 29, y + 13);
    u8g2.drawLine(x + 18, y + 2, x + 18, y + 4);
    u8g2.drawLine(x + 18, y + 11, x + 18, y + 13);
  } else {
    u8g2.drawRFrame(x, y, 32, 16, 4);
    u8g2.drawRBox(x + 2, y + 2, 12, 12, 4);
  }
}

void page_manuell() {
  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(33, 14, "Manuell");

  u8g2.setFont(u8g2_font_t0_18_tr);
  u8g2.drawStr(6, 39, "Trigger");

  if (ready) {
    u8g2.drawRFrame(78, 25, 18, 18, 3);
    if (foggingActive) {
      u8g2.drawRBox(80, 27, 14, 14, 3);
    }
  }
}

void page_wifi() {
  u8g2.setFont(u8g2_font_t0_16_tr);
  u8g2.drawStr(6, 39, "kommt noch...");
  //nach wifi chagen ESP.restart(); callen
}

void page_timer() {
  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(42, 14, "Timer");

  u8g2.setFont(u8g2_font_t0_14_tr);
  u8g2.drawStr(6, 48, "Off Time:");

  u8g2.drawStr(6, 62, "On Time:");

  u8g2.setFont(u8g2_font_t0_16_tr);
  u8g2.drawStr(6, 32, "Active");

  u8g2.setFont(u8g2_font_t0_14_tr);
  u8g2.drawStr(103, 47, "min");

  u8g2.drawStr(103, 62, "s");

  u8g2.setFont(u8g2_font_t0_14b_tr);
  u8g2.drawStr(77, 47, itoa(preferences.getUInt("timerOff", 60), stringBuffer, 10));

  u8g2.drawStr(85, 62, itoa(preferences.getUInt("timerOn", 30), stringBuffer, 10));

  draw_switch(79, 17, preferences.getBool("timerActive", false));

  switch (edit_selected) {
    case 0:
      u8g2.drawFrame(76, 15, 38, 20);
      break;
    case 1:
      u8g2.drawFrame(74, 34, 28, 17);
      if (editMode) {
        u8g2.setDrawColor(2);
        u8g2.drawBox(74, 34, 28, 17);
        u8g2.setDrawColor(1);
      }
      break;
    case 2:
      u8g2.drawFrame(83, 51, 19, 13);
      if (editMode) {
        u8g2.setDrawColor(2);
        u8g2.drawBox(83, 51, 19, 13);
        u8g2.setDrawColor(1);
      }
      break;
  }
}

void page_heatOnOff() {
  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(15, 14, "Heat an/aus");

  u8g2.setFont(u8g2_font_t0_16_tf);
  u8g2.drawStr(5, 53, "Temp:");

  u8g2.drawStr(5, 36, "Heizen");

  u8g2.drawUTF8(95, 53, "°C");

  u8g2.drawStr(69, 53, itoa(temperature, stringBuffer, 10));

  draw_switch(79, 21, preferences.getBool("heatActive", true));
}

void page_dmxAdress() {
  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(15, 14, "DMX Adresse");

  u8g2.setFont(u8g2_font_t0_18_tr);
  u8g2.drawStr(16, 43, "Adresse:");

  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(95, 43, itoa(preferences.getUInt("dmxAdress", 30), stringBuffer, 10));

  if (preferences.getBool("dmxActive", true)) {
    u8g2.setFont(u8g2_font_t0_12_tr);
    u8g2.drawStr(3, 62, "Captured DMX:");
    u8g2.drawStr(90, 62, itoa(dmxValue, stringBuffer, 10));
  }

  /*if (edit_selected == 1) {
    u8g2.setDrawColor(2);
    u8g2.drawBox(93, 29, 32, 16);
    u8g2.setDrawColor(1);
    }*/
}

void page_dmxMode() {
  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(28, 14, "DMX Mode");

  u8g2.setFont(u8g2_font_t0_16_tr);
  u8g2.drawStr(7, 32, "Mode 1");

  u8g2.drawStr(73, 32, "Mode 2");

  u8g2.setFont(u8g2_font_t0_13_tr);
  u8g2.drawStr(6, 50, "Trigger");

  u8g2.drawStr(80, 50, "Timer");

  if (preferences.getUInt("dmxMode", 1) == 1) {
    u8g2.drawRFrame(3, 17, 55, 40, 3);
  } else if (preferences.getUInt("dmxMode", 1) == 2) {
    u8g2.drawRFrame(70, 17, 55, 40, 3);  //selected Mode 2
  }
}

void page_dmxDebug() {
  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(15, 14, "DMX Debug");

  u8g2.setFont(u8g2_font_t0_16_tr);
  u8g2.drawStr(25, 34, "DMX");

  draw_switch(79, 19, preferences.getBool("dmxActive", true));

  if (preferences.getBool("dmxActive", true)) {
    u8g2.setFont(u8g2_font_t0_12_tr);
    u8g2.drawStr(3, 50, "Recieved DMX:");

    u8g2.setFont(u8g2_font_t0_12_tr);
    u8g2.drawStr(88, 50, itoa(dmxPPS, stringBuffer, 10));

    u8g2.drawStr(104, 50, "P/s");

    u8g2.drawStr(3, 62, "Adresse:");

    u8g2.drawStr(54, 62, itoa(preferences.getUInt("dmxAdress", 1), stringBuffer, 10));

    u8g2.drawStr(84, 62, "Mode:");

    u8g2.drawStr(117, 62, itoa(preferences.getUInt("dmxMode", 1), stringBuffer, 10));
  }
}

void page_home() {
  u8g2.drawXBMP(6, 6, 16, 16, image_temp);  //temp icon zeichen

  u8g2.setFont(u8g2_font_t0_18_tf);
  u8g2.drawUTF8(60, 21, " °C");

  if (relay_heat && !ready) {
    u8g2.drawStr(6, 40, "Heating");
  } else if (ready && !relay_pump) {
    u8g2.drawStr(6, 40, "Ready");
  } else if (relay_pump && ready) {
    u8g2.drawStr(6, 40, "Fogging");
  } else if (!relay_heat && !relay_pump && !ready) {
    u8g2.drawStr(6, 40, "Standby");
  } else {
    u8g2.drawStr(6, 40, "Error");
  }

  u8g2.drawStr(33, 21, itoa(temperature, stringBuffer, 10));

  if (preferences.getBool("dmxActive", true)) {
    u8g2.drawStr(6, 60, "DMX Adr:");
    u8g2.drawStr(82, 60, itoa(preferences.getUInt("dmxAdress", 1), stringBuffer, 10));
  } else {
    u8g2.setFont(u8g2_font_t0_16_tr);
    u8g2.drawStr(6, 60, "DMX deactivated");
  }

  if (displayInverted) {
    u8g2.setDrawColor(2);
    u8g2.drawBox(0, 0, 128, 64);
    u8g2.setDrawColor(1);
  }
}

void page_help() {
  u8g2.drawXBMP(3, 3, 58, 58, image_qr_code);

  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.drawStr(65, 10, "github.com");

  u8g2.drawStr(72, 53, "press to");

  u8g2.drawStr(75, 63, "restart");

  u8g2.setFont(u8g2_font_t0_12b_tr);
  u8g2.drawStr(75, 27, "made by");

  u8g2.drawStr(84, 38, "Joni");
}

void drawScreen() {
  u8g2.clearBuffer();

  if (current_screen == 0) {  // menu malen

    // selected item background
    u8g2.drawXBMP(0, 22, 128, 21, bitmap_item_sel_outline);

    // draw previous item as label
    u8g2.setFont(u8g2_font_t0_16_tr);
    u8g2.drawStr(9, 15, menu_items[item_sel_previous]);

    // draw selected item as label in bold font
    u8g2.setFont(u8g2_font_t0_16b_tr);
    u8g2.drawStr(9, 15 + 20 + 2, menu_items[menu_selected]);

    // draw next item as label
    u8g2.setFont(u8g2_font_t0_16_tr);
    u8g2.drawStr(9, 15 + 20 + 20 + 2 + 2, menu_items[item_sel_next]);

    // draw scrollbar background
    u8g2.drawXBMP(128 - 8, 0, 8, 64, bitmap_scrollbar_background);

    // draw scrollbar handle
    u8g2.drawBox(125, 64 / menu_itemsLength * menu_selected, 3, 64 / menu_itemsLength);

  } else if (current_screen == 1) {  //pages malen
    switch (menu_selected) {
      case 0:
        page_manuell();
        break;

      case 1:
        page_wifi();
        break;

      case 2:
        page_timer();
        break;

      case 3:
        page_heatOnOff();
        break;

      case 4:
        page_dmxAdress();
        break;

      case 5:
        page_dmxMode();
        break;

      case 6:
        page_dmxDebug();
        break;

      case 7:
        page_home();
        break;

      case 8:
        page_help();
        break;
    }
  }
  u8g2.sendBuffer();
}

int getEncoderScroll(int changingValue, int maxNum, bool wantsAction) {
  int encoderDirection = (int)(encoder.getDirection());
  if (encoderDirection != 0) {
    changingValue += encoderDirection;
    if (changingValue < 0) {
      changingValue = maxNum - 1;
    }
    if (changingValue >= maxNum) {
      changingValue = 0;
    }
    if (wantsAction) action = true;
    return changingValue;
  }
  return changingValue;
}

void pageFunctions() {
  //--------------menu page functions-------
  if (current_screen == 0) {
    menu_selected = getEncoderScroll(menu_selected, menu_itemsLength, true);  //through menu scroll

    encoderButton.attachClick([]() {  //menu into page press
      current_screen = 1;
      action = true;
    });
  }
  //-----------page page functions-------------
  if (current_screen == 1 && menu_selected == 0) {  //manuell page
    unsigned long timeTrigger = millis();

    encoderButton.attachClick([]() {  //button function
      foggingActiveDisplay = true;
      action = true;
      buttonPressed = true;
      lastTrigger = millis();
    });

    if (buttonPressed && ((timeTrigger - lastTrigger) >= foggingTimeOnDisplayPress)) {
      //lastTrigger = timeTrigger;
      buttonPressed = false;
      foggingActiveDisplay = false;
      action = true;
    }
  } else if (current_screen == 1 && menu_selected == 1) {  //wifi page
  } else if (current_screen == 1 && menu_selected == 2) {  //timer page

    encoderButton.attachClick([]() {  //button function
      editMode = !editMode;
      action = true;
      if (edit_selected == 0) {
        preferences.putBool("timerActive", !preferences.getBool("timerActive", false));
        editMode = !editMode;
        action = true;
      }
    });
    if (!editMode) {
      edit_selected = getEncoderScroll(edit_selected, 3, true);
    } else {
      switch (edit_selected) {
        case 1:
          preferences.putUInt("timerOff", getEncoderScroll(preferences.getUInt("timerOff", 1), 121, true));
          break;
        case 2:
          preferences.putUInt("timerOn", getEncoderScroll(preferences.getUInt("timerOn", 1), 91, true));
          break;
      }
    }
  } else if (current_screen == 1 && menu_selected == 3) {  //heat page
    encoderButton.attachClick([]() {
      preferences.putBool("heatActive", !preferences.getBool("heatActive", true));
      action = true;
    });
  } else if (current_screen == 1 && menu_selected == 4) {  //dmx adress page
    edit_selected = 1;
    preferences.putUInt("dmxAdress", getEncoderScroll(preferences.getUInt("dmxAdress", 1), 255, true));
  } else if (current_screen == 1 && menu_selected == 5) {  //dmx mode page
    encoderButton.attachClick([]() {                       //button function
      if (preferences.getUInt("dmxMode", 1) == 1) preferences.putUInt("dmxMode", 2);
      else preferences.putUInt("dmxMode", 1);
      action = true;
    });
  } else if (current_screen == 1 && menu_selected == 6) {  //debug dmx page
    encoderButton.attachClick([]() {                       //button function
      preferences.putBool("dmxActive", !preferences.getBool("dmxActive", true));
      action = true;
    });
  } else if (current_screen == 1 && menu_selected == 7) {  //home page
    encoderButton.attachClick([]() {                       //button function
      current_screen = 0;
      action = true;
    });
  } else if (current_screen == 1 && menu_selected == 8) {  //help page
    encoderButton.attachClick([]() {                       //button function
      ESP.restart();
    });
  }
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
    if (!digitalRead(triggerButton)) {
      foggingActiveButton = true;
    } else if (digitalRead(triggerButton)) {
      foggingActiveButton = false;
    }
  }

  if (foggingActiveDMX || foggingActiveButton || foggingActiveDisplay || foggingActiveTimer) {
    foggingActive = true;
  } else {
    foggingActive = false;
  }

  if ((lastFoggingState != foggingActive) && foggingActive) {  //sometimes simpler isnt better; hätte man safe schöner machen können aber so gehts auch
    foggingTime = now;
    lastFoggingState = foggingActive;
  }

  if ((now - foggingTime) >= maxFoggingTime) {
    foggingActive = false;
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

void dmxInPrefs(){
  dmxActive  = preferences.getBool("dmxActive", true);
}

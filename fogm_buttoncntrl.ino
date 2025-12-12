#include <TM1637Display.h>
#include <max6675.h>

#define CLK 4
#define DIO 2
TM1637Display display(CLK, DIO);

#define thermoDO 12   // SO
#define thermoCS 10   // CS
#define thermoCLK 13  // SCK
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);


const int heaterPin = 8;   // heating relay (inverted)
const int pumpPin = 7;     // pump relay (inverted)
const int ledReady = 9;    // ready LED
const int buttonPin = A1;  // trigger button (on=low)
const int sled1 = 5;       // smart LED 1(pwm)
const int sled2 = 3;       // smart LED 2(pwm)


const float TEMP_READY = 210.0;
const float TEMP_HEAT_STOP = 230.0;
const float TEMP_CUTOFF = 240.0;


const unsigned long TEMP_INTERVAL = 250;
unsigned long lastTempRead = 0;
float tempC = 0;


const unsigned long RELAY_MIN_INTERVAL = 3000;
unsigned long lastHeaterSwitch = 0;
unsigned long lastPumpSwitch = 0;


const int SLED1_DIM = 15;      // dim brightness when idle
const int SLED1_BRIGHT = 255;  // bright when heating


const unsigned long PUMP_MIN_ON = 100;
unsigned long pumpTurnOnTime = 0;


const unsigned long BLINK_INTERVAL = 250;
unsigned long lastBlinkTime = 0;
bool blinkState = false;


bool heaterState = true;  //inverted
bool pumpState = true;    //inverted




void setup() {
  Serial.begin(9600);

  pinMode(heaterPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(ledReady, OUTPUT);
  pinMode(sled1, OUTPUT);
  pinMode(sled2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(heaterPin, HIGH);
  digitalWrite(pumpPin, HIGH);

  analogWrite(sled1, SLED1_DIM);
  analogWrite(sled2, LOW);

  display.setBrightness(7);
  display.clear();

  delay(500);
}




void loop() {

  if (millis() - lastTempRead >= TEMP_INTERVAL) {
    lastTempRead = millis();
    tempC = thermocouple.readCelsius();

    Serial.print("Temp: ");
    Serial.println(tempC);

    display.showNumberDec((int)tempC, false);
  }

  bool isReady = (tempC > TEMP_READY);
  digitalWrite(ledReady, isReady ? HIGH : LOW);

  bool buttonPressed = !digitalRead(buttonPin);  //  inverted

  bool desiredPumpState = true;  // default OFF
  bool isSmoking = false;

  if (buttonPressed && isReady) {
    desiredPumpState = false;  // ON
    isSmoking = true;
  }

  setPump(desiredPumpState);


  bool desiredHeaterState = true;  // OFF
  bool heaterActive = false;

  if (tempC >= TEMP_CUTOFF) {
    desiredHeaterState = true;
  } else {
    if (buttonPressed && isReady) {
      desiredHeaterState = false;
      heaterActive = true;
    } else if (tempC < TEMP_HEAT_STOP) {
      desiredHeaterState = false;
      heaterActive = true;
    } else {
      desiredHeaterState = true;
    }
  }

  setHeater(desiredHeaterState);



  if (heaterActive) {
    analogWrite(sled1, SLED1_BRIGHT);
  } else {
    analogWrite(sled1, SLED1_DIM);
  }


  if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = millis();
    blinkState = !blinkState;
  }

  if (isSmoking) {

    analogWrite(sled2, blinkState ? 255 : 0);
  } else if (isReady) {

    analogWrite(sled2, 255);
  } else {

    analogWrite(sled2, 0);
  }
}




void setHeater(bool newState) {
  if (newState != heaterState && millis() - lastHeaterSwitch >= RELAY_MIN_INTERVAL) {

    heaterState = newState;
    digitalWrite(heaterPin, heaterState);
    lastHeaterSwitch = millis();
  }
}




void setPump(bool newState) {
  unsigned long now = millis();

  if (newState == false) {
    if (pumpState == true) {
      pumpState = false;
      digitalWrite(pumpPin, LOW);
      pumpTurnOnTime = now;
      lastPumpSwitch = now;
    }
    return;
  }

  if (pumpState == false) {

    if (now - pumpTurnOnTime < PUMP_MIN_ON)
      return;
  }

  if (now - lastPumpSwitch >= PUMP_MIN_ON) {
    pumpState = true;
    digitalWrite(pumpPin, HIGH);
    lastPumpSwitch = now;
  }
}

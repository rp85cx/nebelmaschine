//--------config-------------
char menu_items[9][15] = {
  { "Manuell" },
  { "Help" },
  { "Timer" },
  { "Heat an/aus" },
  { "DMX Adresse" },
  { "DMX Mode" },
  { "DMX Debug" },
  { "Home" }
};

const int inactivityTime = 120;     //in s; -120-       invert time = inactivity time * 5
const int tempCheckInterval = 500;  //in ms >200; -500-
const int dmxUpdateInterval = 100;  //in ms; -100-
const int maxFoggingTime = 90;      //in s; -60-

const int led_dunkelPwm = 8;       //helligkeit von dunkler status led, 0-255; -8-

const int foggingTimeOnDisplayPress = 500;  //wie lange (ms) foggen soll wenn in manuell page geklickt wurde -500-

const bool debugMode = true;            //serial feedback active oder nicht

//-------pin declarations-------- 
const int relay_heatPin = 19;
const int relay_pumpPin = 18;
const int led_ready = 32;
const int led_pwrHeat = 12;
const int led_rdyOut = 13;
const int triggerButton = 14;
const int led_fluidRed = 5;
const int led_fluidBlue = 4;
const int encoderA = 0;
const int encoderB = 2;
const int encoderButtonPin = 15;
const int thermocoupleDO = 26;
const int thermocoupleCS = 25;
const int thermocoupleCLK = 33;
const int fluidSensorPin = 23;
const int dmxRX = 16;   //RO
const int dmxTX = 17;   //DI
const int dmxRTS = 27;  //connect to RE und DE - enable to both

const float temp_ready = 200.0;    //prod: -200-
const float temp_heatMax = 225.0;  //prod: -225-
const float temp_pumpMax = 235.0;  //prod: -235-


//----------program variables and class calls-----------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
RotaryEncoder encoder(encoderA, encoderB, RotaryEncoder::LatchMode::FOUR3);
OneButton encoderButton(encoderButtonPin, true, false);
Preferences preferences;
MAX6675 thermocouple(thermocoupleCLK, thermocoupleCS, thermocoupleDO);
TaskHandle_t core0;
TaskHandle_t core1;

volatile int menu_selected = 7;
volatile int current_screen = 1;
int lastMenu_selected;
int lastCurrent_screen;
int edit_selected = 0;
bool editMode = false;
volatile bool action = true;
unsigned long lastAction = millis();
unsigned long lastInvert = millis();
unsigned long lastTempUpdate;
unsigned long lastTrigger;
unsigned long lastLedUpdate;
unsigned long lastDmxPPSUpdate;
unsigned long lastDmxUpdate;
unsigned long lastTimerUpdate;
unsigned long lastTriggerButtonCheck;
unsigned long lastHeatStateChange;
unsigned long foggingTime;
int lastLedControlState=6;;
int ledControlState;
bool buttonPressed = false;
char stringBuffer[4];
const int menu_itemsLength = (sizeof(menu_items) / sizeof(menu_items[0])) - 1;
byte dmxData[513];
int dmxPPS = 0;
int calcDmxPPS = 0;
bool timerState = false;
int item_sel_next = 0;
int item_sel_previous = 0;
volatile bool dmxIsConnected = false;
volatile int dmxValue = 0;
volatile bool ledState=false;
volatile bool ready = false;
volatile bool relay_heat = false;
volatile bool relay_pump = false;
volatile bool foggingActive = false;
volatile bool foggingActiveDisplay = false;
volatile bool foggingActiveButton = false;
volatile bool foggingActiveDMX = false;
volatile bool foggingActiveTimer = false;
volatile bool foggingAllowed = true;
bool displayInverted=false;
bool lastFoggingState;
//bool timerActive = false;   // -> preferences
//int timerOff = 60;   // -> preferences
//int timerOn = 30;   // -> preferences
//bool heatActive = true;   // -> preferences
volatile int temperature = 0;  //dev: = 200 prod: = 0
//int dmxAdress = 1;   // -> preferences
//int dmxMode = 1;  // -> preferences
volatile bool dmxActive = true;  // -> preferences für core0, local var für core1

/*
default core setup:
xTaskCreatePinnedToCore(  
    task1code, // Task function. 
    "task1",   // name of task. 
    10000,     // Stack size of task 
    NULL,      // parameter of the task 
    1,         // priority of the task 
    &task1,    // Task handle to keep track of created task 
    0);        // pin task to core 0 
  delay(500);
  ps.: du brauchst diesen absoluten hässlichen code auch nicht lesen gell
*/
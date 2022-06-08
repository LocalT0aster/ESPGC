// Debugging & Features///////////////////////////////////////////////
//#define PERFMEASUREMENT // Enables performance measurement & output|

// Libaries
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Screen
TFT_eSPI tft = TFT_eSPI(); // Screen library object
#define BLK_PIN 32         // Backlight pin
#define SCRSIZEX 240       // Screen size x
#define SCRSIZEY 240       // Screen size y
#define BKGCLR TFT_BLACK   // Background color

// Bluetooth
#include <BleGamepad.h>
BleGamepad bleGamepad("ESPGC by NeRaG0n9", "NeRaG0n9", 100);
BleGamepadConfiguration bleGamepadConfig;
// TODO battery level indication. Min working voltage 3.0V

// Button pins (NEW LAYOUT NUM)
#define BA 17      // 0
#define BB 15      // 1
#define BX 16      // 2
#define BY 2       // 3
#define BSTART 5   // 4
#define BSELECT 19 // 5
#define BLEFT 27   // 6
#define BRIGHT 21  // 7
#define BUP 14     // 8
#define BDOWN 22   // 9

// Button layout & state variables
const uint8_t _btnpin[10] = {BA, BB, BX, BY, BSTART, BSELECT, BLEFT, BRIGHT, BUP, BDOWN}; // Button pins
const uint8_t _btnnum[4] = {1, 2, 4, 5};                                                  // Button numbers (for bluetooth compatibility)

boolean btn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};     // Current buttons state
boolean prevbtn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Previous buttons state
int8_t dPadX = 0;
int8_t dPadY = 0;
boolean stateChanged = 0;
uint8_t btncnt; // counter for button arrays
boolean dpadStateChanged = 0;

// Button pins initialization procedure
void _initButtons()
{
  for (btncnt = 0; btncnt < 10; btncnt++)
  {
    pinMode(_btnpin[btncnt], INPUT);
  }
  return;
}

// Dpad state update procedure
void updDPad()
{
  dPadX = 0;
  dPadY = 0;
  if (btn[6])
    dPadX--;
  if (btn[7])
    dPadX++;
  if (btn[8])
    dPadY++;
  if (btn[9])
    dPadY--;
}
void bleDPadSet()
{
  switch (dPadY)
  {
  case 0:
    switch (dPadX)
    {
    case 0:
      bleGamepad.setHat(DPAD_CENTERED);
      break;
    case 1:
      bleGamepad.setHat(DPAD_RIGHT);
      break;
    case -1:
      bleGamepad.setHat(DPAD_LEFT);
      break;
    }
    break;
  case 1:
    switch (dPadX)
    {
    case 0:
      bleGamepad.setHat(DPAD_UP);
      break;
    case 1:
      bleGamepad.setHat(DPAD_UP_RIGHT);
      break;
    case -1:
      bleGamepad.setHat(DPAD_UP_LEFT);
      break;
    }
    break;
  case -1:
    switch (dPadX)
    {
    case 0:
      bleGamepad.setHat(DPAD_DOWN);
      break;
    case 1:
      bleGamepad.setHat(DPAD_DOWN_RIGHT);
      break;
    case -1:
      bleGamepad.setHat(DPAD_DOWN_LEFT);
      break;
    }
    break;
  }
}

// Buttons state update procedure
void readbtn()
{
  for (btncnt = 0; btncnt < 10; btncnt++)
  {
    prevbtn[btncnt] = btn[btncnt];
    btn[btncnt] = digitalRead(_btnpin[btncnt]);
    if (btn[btncnt] != prevbtn[btncnt])
    {
      stateChanged = true;
      if (btncnt > 6)
        dpadStateChanged = 1;
    }
  }
  if (dpadStateChanged)
    updDPad();
  return;
}

// Button test GUI layout
const uint8_t _btnTestBox[10][4] = {
    {160, 140, 40, 40}, // A
    {200, 100, 40, 40}, // B
    {120, 100, 40, 40}, // X
    {160, 60, 40, 40},  // Y
    {121, 215, 50, 25}, // START
    {69, 215, 50, 25},  // SELECT
    {0, 100, 40, 40},   // LEFT
    {80, 100, 40, 40},  // RIGHT
    {40, 60, 40, 40},   // UP
    {40, 140, 40, 40}   // DOWN
};

// Random byte function
uint8_t rnd() { return (esp_random() >> 24); } // returns 0 - 255

// HSV to 16 bit RGB conversion function (thing that I use for cool gradients)
uint16_t HSVtoRGB565(float H, float S, float V)
{
  float s = S / 100;
  float v = V / 100;
  float C = s * v;
  float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
  float m = v - C;
  float r, g, b;
  if (H >= 0 && H < 60)
  {
    r = C, g = X, b = 0;
  }
  else if (H >= 60 && H < 120)
  {
    r = X, g = C, b = 0;
  }
  else if (H >= 120 && H < 180)
  {
    r = 0, g = C, b = X;
  }
  else if (H >= 180 && H < 240)
  {
    r = 0, g = X, b = C;
  }
  else if (H >= 240 && H < 300)
  {
    r = X, g = 0, b = C;
  }
  else
  {
    r = C, g = 0, b = X;
  }
  return tft.color565((r + m) * 255, (g + m) * 255, (b + m) * 255);
}

// Blink procedure for beautiful transitions
void blink(uint16_t toColor)
{
  tft.fillScreen(TFT_WHITE);
  delay(10);
  tft.fillScreen(toColor);
  delay(10);
}

// Default Arduino setup procedure. Executes on startup
void setup()
{
  // Debug
  Serial.begin(115200);
#ifdef PERFMEASUREMENT
  // Time measure
  uint32_t time = 0;
  time += micros();
#endif

  // Screen initialization
  Serial.println("blk pin init");
  pinMode(BLK_PIN, OUTPUT);
  digitalWrite(BLK_PIN, 0);
  tft.init();
  tft.setRotation(1);
  tft.setCursor(40, 80);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(80);
  tft.fillScreen(TFT_BLACK);
  digitalWrite(BLK_PIN, 1);
  blink(TFT_BLACK);

  // Buttons initialization
  _initButtons();

  // Bluetooth gamepad initialization
  bleGamepadConfig.setAutoReport(0);
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepadConfig.setButtonCount(6);
  bleGamepadConfig.setWhichSpecialButtons(1,1,0,0,0,0,0,0);
  bleGamepadConfig.setHatSwitchCount(1);
  bleGamepad.begin(&bleGamepadConfig);

#ifdef PERFMEASUREMENT
  time = micros() - time;
  Serial.println(time);
  Serial.println(micros());
#endif

  return; // initialization end
}

// Bluetooth gamepad implementation using new layout
void loop()
{
  readbtn();
  // Action & special buttons
  for (btncnt = 0; btncnt < 6; btncnt++)
  {
    if (btn[btncnt] != prevbtn[btncnt])
    {
      if (btncnt < 4)
      {
        if (btn[btncnt])
          bleGamepad.press(_btnnum[btncnt]);
        else
          bleGamepad.release(_btnnum[btncnt]);
      }
      else
      {
        if (btncnt == 4)
        {
          if (btn[4])
            bleGamepad.pressStart();
          else
            bleGamepad.releaseStart();
        }
        else
        {
          if (btn[5])
            bleGamepad.pressSelect();
          else
            bleGamepad.releaseSelect();
        }
      }
    }
    tft.drawRect(_btnTestBox[btncnt][0], _btnTestBox[btncnt][1], _btnTestBox[btncnt][2], _btnTestBox[btncnt][3], btn[btncnt] ? TFT_RED : TFT_DARKGREY);
  }
  for (; btncnt < 10; btncnt++)
    tft.drawRect(_btnTestBox[btncnt][0], _btnTestBox[btncnt][1], _btnTestBox[btncnt][2], _btnTestBox[btncnt][3], btn[btncnt] ? TFT_RED : TFT_DARKGREY);
  // DPad
  bleDPadSet();

  bleGamepad.sendReport();
  delay(4); // 250hz
}

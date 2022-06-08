// Debugging & Features
//#define BTTNTEST        // Switch to buttons test programm
//#define PERFMEASUREMENT // Enables performance measurement & output

// Libaries
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Global Constants
// Screen
TFT_eSPI tft = TFT_eSPI(); // Screen library object
#define BLK_PIN 32         // Backlight pin
#define SCRSIZEX 240       // Screen size x
#define SCRSIZEY 240       // Screen size y
#define BKGCLR 0x0020      // Background color

// Button pins (NEW LAYOUT NUM)
#define BA 17                                                                             // 0
#define BB 15                                                                             // 1
#define BX 16                                                                             // 2
#define BY 2                                                                              // 3
#define BSTART 5                                                                          // 4
#define BSELECT 19                                                                        // 5
#define BLEFT 27                                                                          // 6
#define BRIGHT 21                                                                         // 7
#define BUP 14                                                                            // 8
#define BDOWN 22                                                                          // 9
const uint8_t _btnpin[10] = {BA, BB, BX, BY, BSTART, BSELECT, BLEFT, BRIGHT, BUP, BDOWN}; // Buttons layout\

// Button state variables
boolean btn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};     // Current buttons state
boolean prevbtn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Previous buttons state
int8_t dpadY = 0;
int8_t dpadX = 0;
boolean stateChanged = 0;
uint8_t btncnt; // Counter for button arrays
boolean dpadStateChanged = 0;

// Button pins initialization procedure
void _initButtons()
{
  for (btncnt = 0; btncnt < 10; btncnt++)
    pinMode(_btnpin[btncnt], INPUT);
  return;
}

// Dpad state update procedure
void updDPAD()
{
  dpadX = 0;
  dpadY = 0;
  if (btn[6])
    dpadX--;
  if (btn[7])
    dpadX++;
  if (btn[8])
    dpadY++;
  if (btn[9])
    dpadY--;
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
    updDPAD();
  return;
}

// Button test GUI layout
#ifdef BTTNTEST
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
#endif

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

#ifndef BTTNTEST

// Global Game Variables
// uint8_t level = 0; //TODO not implemented, for menus and multiple games

// Snake Game
// Color Pallete
#define SNAKECLR1 0x3353 // Snake color 1
#define SNAKECLR2 0xfea7 // Snake color 2
#define BERRYCLR 0xf800  // Berry color

const uint8_t fieldSize = 30;           // square edge size
const uint16_t maxSnakeLength = fieldSize * fieldSize; // maximum length of snake
uint16_t snakeLenght = 2;               // current snake length
int8_t snake[fieldSize * fieldSize][2]; // snake body coordinates array
int8_t direction = 2;                   // 0 - up, 1 - left, 2 - down, 3 - right
int8_t prevDirection = 2;               // previous direction state
uint16_t period = 500;                  // game tick in ms
int8_t berry[2] = {0, 0};               // berry coordinates {x, y}
int8_t collider[2] = {0, 0};            // collider coordinates {x, y} (used for collision detection)
bool collision = 0;                     //(1 if collided)
bool isBerryPlaced = 0;
bool addLength = 0; // increments the length of the snake
bool isRunning = 1;
// FREE RTOS Handles & Semaphores
TaskHandle_t renderTaskHandle;
TaskHandle_t directionsTaskHandle;
SemaphoreHandle_t renderSem;

// Game Functions
// Erase tail procedure. Fills the last snake tile on the screen with background color.
void eraseTail() { tft.fillRect(snake[snakeLenght - 1][0] * 8, snake[snakeLenght - 1][1] * 8, 8, 8, BKGCLR); }

// Render task (FREE RTOS)
void tRenderSnake(void *p)
{
  for (;;)
  {
    for (uint16_t i = 0; i < snakeLenght; i++)
    {
      tft.fillRect(snake[i][0] * 8, snake[i][1] * 8, 8, 8, i % 2 == 0 ? SNAKECLR1 : SNAKECLR2);
    }
    tft.fillRoundRect(berry[0] * SCRSIZEX / fieldSize, berry[1] * SCRSIZEY / fieldSize, SCRSIZEX / fieldSize, SCRSIZEY / fieldSize, 1, BERRYCLR);
    xSemaphoreTake(renderSem, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}

// Direction change task (FREE RTOS)
void tDirChange(void *p)
{
  for (;;)
  {
    if (isRunning)
    {
      readbtn();
      if (dpadY == 1)
      {
        if (prevDirection != 2)
          direction = 0;
      }
      else if (dpadX == -1)
      {
        if (prevDirection != 3)
          direction = 1;
      }
      else if (dpadY == -1)
      {
        if (prevDirection != 0)
          direction = 2;
      }
      else if (dpadX == 1)
      {
        if (prevDirection != 1)
          direction = 3;
      }
      delay(4); // 250hz
    }
    delay(100);
  }
  vTaskDelete(NULL);
}

// Berry Placement procedure
void placeBerry()
{
  do
  {
    berry[0] = rnd() % fieldSize;
    berry[1] = rnd() % fieldSize;
    isBerryPlaced = 1;
    for (uint16_t i = 0; i < snakeLenght; i++)
      if (snake[i][0] == berry[0] && snake[i][1] == berry[1])
      {
        isBerryPlaced = 0;
        break;
      }
  } while (!isBerryPlaced);
}

// Game initialization procedure
void initSnake()
{
  snakeLenght = 2;
  snake[0][0] = fieldSize >> 1;
  snake[0][1] = fieldSize >> 1;
  snake[1][0] = fieldSize >> 1;
  snake[1][1] = (fieldSize >> 1) + 1;
  direction = 1;
  collision = 0;
  placeBerry();
  blink(BKGCLR);
  xTaskCreatePinnedToCore(tRenderSnake, "render", 4000, NULL, 0, &renderTaskHandle, 1);
  vSemaphoreCreateBinary(renderSem);
  xTaskCreate(tDirChange, "dir", 1000, NULL, 0, &directionsTaskHandle);
  delay(1000);
  isRunning = 1;
}

// Snake movement procedure
void moveSnake()
{
  // Border collision detection
  collider[0] = snake[0][0];
  collider[1] = snake[0][1];
  if (direction == 0)
  {
    collider[1]--;
    if (collider[1] == -1)
      collision = 1;
  }
  else if (direction == 1)
  {
    collider[0]--;
    if (collider[0] == -1)
      collision = 1;
  }
  else if (direction == 2)
  {
    collider[1]++;
    if (collider[1] == fieldSize)
      collision = 1;
  }
  else if (direction == 3)
  {
    collider[0]++;
    if (collider[0] == fieldSize)
      collision = 1;
  }
  if (collision)
  {
    isRunning = 0;
    return;
  }
  // Body collision detection
  for (uint16_t i = snakeLenght - 2; i > 2; i--)
  {
    if (collider[0] == snake[i][0] && collider[1] == snake[i][1])
    {
      collision = 1;
      isRunning = 0;
      return;
    }
  }
  // Berry collision detection
  if (collider[0] == berry[0] && collider[1] == berry[1])
  {
    addLength = 1;
    isBerryPlaced = 0;
  }
  prevDirection = direction;
  // Movement
  for (uint16_t i = snakeLenght - 1 + addLength; i > 0; i--)
  {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }
  snake[0][0] = collider[0];
  snake[0][1] = collider[1];
  if (addLength)
  {
    snakeLenght++;
    addLength = 0;
    if (snakeLenght >= maxSnakeLength) isRunning = 0;
  }
}
#endif

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
  pinMode(BLK_PIN, OUTPUT);
  digitalWrite(BLK_PIN, 0);
  tft.init();
  tft.setRotation(1);
  tft.setCursor(40, 80);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  digitalWrite(BLK_PIN, 1);
  blink(TFT_BLACK);

  // Buttons initialization
  _initButtons();

#ifdef PERFMEASUREMENT
  time = micros() - time;
  Serial.println(time);
  Serial.println(micros());
#endif

  // Game initialization
#ifndef BTTNTEST
  initSnake();
#endif
  return; // initialization end
}

#ifdef BTTNTEST
// buttons test
void loop()
{
  readbtn();
  for (int32_t i = 0; i < 10; i++)
    tft.drawRect(_btnTestBox[i][0], _btnTestBox[i][1], _btnTestBox[i][2], _btnTestBox[i][3], btn[i] == 1 ? TFT_RED : TFT_DARKGREY);
  delay(4); // 250hz
}
#else

// Game loop
void loop()
{
  if (isRunning)
  {
    eraseTail();
    moveSnake();
    if (!isBerryPlaced)
      placeBerry();
    xSemaphoreGive(renderSem);
    delay(period);
  }
  else
  {
    vTaskDelete(renderTaskHandle);
    vTaskDelete(directionsTaskHandle);
    vSemaphoreDelete(renderSem);
    delay(2000);
    blink(TFT_BLACK);
    tft.setCursor(40, 80);
    tft.setTextSize(7);
    tft.print(snakeLenght);
    delay(3000);
    tft.setTextSize(2);
    tft.setCursor(0, 224);
    tft.print("PRESS START");
    for (;;)
    {
      delay(10);
      readbtn();
      if (btn[4])
        break;
    }
    initSnake();
  }
}
#endif

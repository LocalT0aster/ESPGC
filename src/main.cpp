//Debugging/////////////////////////////////////////////////////
//#define BTTNTEST          //Enables button test              |
//#define TIMEMEASUREMENT   //Enables time measurement & output|
//#define PRINTFUNC         //Outputs functions name           |
//#define ENABLE_BLE        //Enables Bluetooth                |
#define LAYOUT_OLD        //Enables old button layout        |
//Libaries
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#ifdef ENABLE_BLE

#include <BleGamepad.h>
BleGamepad gamepad;
//TODO battery level indication, min voltage 3.0V
#endif
//Screen//////////////////////////
TFT_eSPI tft = TFT_eSPI(); ///////
#define BLK_PIN 32         //Backlight pin
//Buttons/////////////(NEW LAYOUT)
#define BA 17      //0
#define BB 15      //1
#define BX 16      //2
#define BY 2       //3
#define BSTART 5   //4
#define BSELECT 19 //5
#define BLEFT 27   //6
#define BRIGHT 21  //7
#define BUP 14     //8
#define BDOWN 22   //9
#ifdef LAYOUT_OLD
const uint8_t _btnpin[10] = {BSTART, BSELECT, BDOWN, BRIGHT, BUP, BLEFT, BA, BB, BY, BX};
#else
const uint8_t _btnpin[10] = {
  BA,
  BB,
  BX,
  BY,
  BSTART,
  BSELECT,
  BLEFT,
  BRIGHT,
  BUP,
  BDOWN
  };
#endif
boolean btn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Get buttons state here
boolean prevbtn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Get previous buttons state here
int8_t dpadY = 0;
int8_t dpadX = 0;
boolean stateChanged = 0;
uint8_t _btncounter;
int8_t sendNum = 1;
boolean dpadStateChanged = 0;
SemaphoreHandle_t tSem;
void tRenderSendNum(void *p)
{
  for (;;)
  {
    tft.fillRect(40,0,60,20,TFT_BLACK);
    tft.setCursor(40,0);
    tft.print(sendNum);
    xSemaphoreTake(tSem, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}
void _initButtons()
{
#if PRINTFUNC
  Serial.println("f _initButtons");
#endif
  pinMode(BSTART, INPUT);
  pinMode(BSELECT, INPUT);
  pinMode(BDOWN, INPUT);
  pinMode(BRIGHT, INPUT);
  pinMode(BUP, INPUT);
  pinMode(BLEFT, INPUT);
  pinMode(BA, INPUT);
  pinMode(BB, INPUT);
  pinMode(BY, INPUT);
  pinMode(BX, INPUT);
  return;
}
void updDPAD()
{
  dpadX = 0;
  dpadY = 0;
  if(btn[6])dpadX--;
  if(btn[7])dpadX++;
  if(btn[8])dpadY++;
  if(btn[9])dpadY--;
/*
  switch(dpadY)
  {
    case 0:
      switch(dpadX)
      {
        case 0:
          gamepad.setHat(DPAD_CENTERED);
          break;
        case 1:
          gamepad.setHat(DPAD_RIGHT);
          break;
        case -1:
          gamepad.setHat(DPAD_LEFT);
          break;
      }
      break;
    case 1:
      switch(dpadX)
      {
        case 0:
          gamepad.setHat(DPAD_UP);
          break;
        case 1:
          gamepad.setHat(DPAD_UP_RIGHT);
          break;
        case -1:
          gamepad.setHat(DPAD_UP_LEFT);
          break;
      }
      break;
    case -1:
      switch(dpadX)
      {
        case 0:
          gamepad.setHat(DPAD_DOWN);
          break;
        case 1:
          gamepad.setHat(DPAD_DOWN_RIGHT);
          break;
        case -1:
          gamepad.setHat(DPAD_DOWN_LEFT);
          break;
      }
      break;
  }
*/
}
void readbtn() // Update buttons state
{
  for (_btncounter = 0; _btncounter < 10; _btncounter++){
    prevbtn[_btncounter] = btn[_btncounter];
    btn[_btncounter] = digitalRead(_btnpin[_btncounter]);
    if(btn[_btncounter] != prevbtn[_btncounter]){
      stateChanged = true;
      if(_btncounter>6)dpadStateChanged = 1;
      /*
      if(btn[_btncounter]>prevbtn[_btncounter])
        gamepad.press(_btncounter+1);
      else gamepad.release(_btncounter+1);*/
    }
  }
  if(dpadStateChanged) updDPAD();
  return;
}
#ifdef BTTNTEST
#ifdef LAYOUT_OLD
const uint8_t _btnTestBox[10][4] = {
    {121, 215, 50, 25}, //START
    {69, 215, 50, 25},  //SELECT
    {40, 140, 40, 40},  //DOWN
    {80, 100, 40, 40},  //RIGHT
    {40, 60, 40, 40},   //UP
    {0, 100, 40, 40},   //LEFT
    {160, 140, 40, 40}, //A
    {200, 100, 40, 40}, //B
    {160, 60, 40, 40},  //Y
    {120, 100, 40, 40}};//X
#else
const uint8_t _btnTestBox[10][4] = {
    {160, 140, 40, 40}, //A
    {200, 100, 40, 40}, //B
    {120, 100, 40, 40}, //X
    {160, 60, 40, 40},  //Y
    {121, 215, 50, 25}, //START
    {69, 215, 50, 25},  //SELECT
    {0, 100, 40, 40},   //LEFT
    {80, 100, 40, 40},  //RIGHT
    {40, 60, 40, 40},   //UP
    {40, 140, 40, 40}   //DOWN
};
#endif
#endif
uint16_t HSVtoRGB565(float H, float S, float V) //Thing that I use for cool gradients
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
uint8_t rnd() { return (esp_random() >> 24); } // returns 0 - 255
#ifndef BTTNTEST
//Global Game Variables (put your variables here)///////////////
//uint8_t level = 0; //TODO not implemented, for menus
#define SNAKECLR1 0x3353
#define SNAKECLR2 0xfea7
const uint8_t fieldSize = 30; //edge of square
uint8_t snake[fieldSize * fieldSize][2];
uint16_t snakeLenght = 2;
uint8_t direction = 2; //0 - down, 1 - right, 2 - up, 3 - left
uint16_t period = 500; //in ms
uint8_t berry[2] = {0, 0};
uint32_t placeAttempts = 0;
uint8_t collider[2] = {0, 0};
bool collision = 0;
bool isBerryPlaced = 0;
bool addLenght = 0; //for render purposes
bool isRunning = 1;
TaskHandle_t rHandle;
TaskHandle_t dHandle;
SemaphoreHandle_t rSem;
#endif
//Game Functions
void blink()
{
#if PRINTFUNC
  Serial.println("f blink");
#endif
  tft.fillScreen(TFT_WHITE);
  delay(10);
  tft.fillScreen(TFT_BLACK);
  delay(10);
}
#ifndef BTTNTEST
void tRenderSnake(void *p)
{
  for (;;)
  {
    //tft.fillScreen(TFT_BLACK);
    for (uint16_t i = 0; i < snakeLenght; i++)
    {
      tft.fillRect(snake[i][0] * 8, snake[i][1] * 8, 8, 8, i % 2 == 0 ? SNAKECLR1 : SNAKECLR2);
    }
    tft.fillRoundRect(berry[0] * floor(240 / fieldSize), berry[1] * floor(240 / fieldSize), floor(240 / fieldSize), floor(240 / fieldSize), 1, TFT_RED);
    xSemaphoreTake(rSem, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}
void eraseTail() { tft.fillRect(snake[snakeLenght - 1][0] * 8, snake[snakeLenght - 1][1] * 8, 8, 8, TFT_BLACK); };
void tDirChange(void *p)
{
  for (;;)
  {
    if(isRunning){
    readbtn();
    for (uint8_t i = 2; i < 6; i++)
      if (btn[i] && (direction + 2) % 4 != i - 2)
        direction = i - 2;
    delay(4);//250hz
    }
    delay(100);
  }
  vTaskDelete(NULL);
}
void placeBerry()
{
  placeAttempts = 0;
  do
  {
    berry[0] = floor(rnd() % fieldSize);
    berry[1] = floor(rnd() % fieldSize);
    placeAttempts++;
    isBerryPlaced = 1;
    for (uint16_t i = 0; i < snakeLenght; i++)
      if (snake[i][0] == berry[0] && snake[i][1] == berry[1])
      {
        isBerryPlaced = 0;
        break;
      }
  } while (!isBerryPlaced);
  Serial.print("placeAttempts ");
  Serial.println(placeAttempts);
}
void initSnake()
{
  snakeLenght = 2;
  snake[0][0] = fieldSize >> 1;
  snake[0][1] = fieldSize >> 1;
  snake[1][0] = fieldSize >> 1;
  snake[1][1] = (fieldSize >> 1) + 1;
  direction = 2;
  collision = 0;
  placeBerry();
  blink();
  xTaskCreatePinnedToCore(tRenderSnake, "render", 4000, NULL, 0, &rHandle, 1);
  vSemaphoreCreateBinary(rSem);
  xTaskCreate(tDirChange, "dir", 1000, NULL, 0, &dHandle);
  delay(1000);
  isRunning = 1;
}
void moveSnake()
{
  collider[0] = snake[0][0];
  collider[1] = snake[0][1];
  switch (direction)
  {
  case 0:
    collider[1]++;
    if (collider[1] == fieldSize)
      collision = 1;
    break;
  case 1:
    collider[0]++;
    if (collider[0] == fieldSize)
      collision = 1;
    break;
  case 2:
    collider[1]--;
    if (collider[1] == 255)
      collision = 1;
    break;
  case 3:
    collider[0]--;
    if (collider[0] == 255)
      collision = 1;
    break;
  default:
    break;
  }
  if (collision)
  {
    Serial.println("border collision!");
    isRunning = 0;
    return;
  }
  for (uint16_t i = 3; i < snakeLenght; i++)
    if (collider[0] == snake[i][0] && collider[1] == snake[i][1])
    {
      collision = 1;
      Serial.println("body collision!");
      isRunning = 0;
      return;
    }
  if (collider[0] == berry[0] && collider[1] == berry[1])
  {
    addLenght = 1;
    isBerryPlaced = 0;
  }
  for (uint16_t i = snakeLenght - 1; i > 0; i--)
  {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }
  snake[0][0] = collider[0];
  snake[0][1] = collider[1];
  if (addLenght)
  {
    snake[snakeLenght][0] = snake[snakeLenght - 1][0];
    snake[snakeLenght][1] = snake[snakeLenght - 1][1];
    snakeLenght++;
    addLenght = 0;
  }
}
/*void printSnakeArray()
{
  Serial.println("Snake:");
  for (uint16_t i = 0; i < fieldSize * fieldSize; i++)
  {
    Serial.print(i);
    Serial.print(": ");
    Serial.print(snake[i][0]);
    Serial.print(' ');
    Serial.println(snake[i][1]);
  }
}*/
#endif
void setup() ////////////////////////////////////////////////////////////////
{
  //Debug
  Serial.begin(115200);
#if PRINTFUNC
  Serial.println("f setup");
#endif
#ifdef TIMEMEASUREMENT
  //Time measure
  uint32_t time = 0;
  time += micros();
#endif
  _initButtons();
  //Screen initialization
  pinMode(BLK_PIN, OUTPUT);
  digitalWrite(BLK_PIN, 0);
  tft.init();
  tft.setRotation(1);
  tft.setCursor(40, 80);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(80);
  tft.fillScreen(TFT_BLACK);
  digitalWrite(BLK_PIN, 1);
  blink();
  //Bluetooth gamepad initialization
  #ifdef ENABLE_BLE
  gamepad.begin(128,0,0,0,0,0,0,0,0,0,0,0,0,0,0);//buttons only implementation
  gamepad.setAutoReport(0);
  tft.setCursor(40, 0);
  tft.setTextSize(20);
  vSemaphoreCreateBinary(tSem);
  #endif
  //delay(1980);
#ifdef TIMEMEASUREMENT
  time = micros() - time;
  Serial.println(time);
  Serial.println(micros());
#endif
  //////////////////////
  //Game initialization
  ///(init things here)/
  #ifndef BTTNTEST
  initSnake();
  #endif
  return; //initialization end
} ////////////////////////////////////////////////////////////////

#ifdef BTTNTEST
//buttons test
void loop()
{
  readbtn();

  if(gamepad.isConnected()&&stateChanged)
  {
    if(prevbtn[0]!=btn[0])
    {
      if(btn[0]) gamepad.press(sendNum);
      else gamepad.release(sendNum);
    }
    gamepad.sendReport();
    stateChanged = false;
    }
  if(dpadStateChanged){
    gamepad.release(sendNum);
    if(dpadY == 1) sendNum++;
    else if(dpadY == -1) sendNum--;
    xSemaphoreGive(tSem);
  }

  for (int32_t i = 0; i < 10; i++)
    tft.drawRect(_btnTestBox[i][0], _btnTestBox[i][1], _btnTestBox[i][2], _btnTestBox[i][3], btn[i] == 1 ? TFT_RED : TFT_DARKGREY);
  
  delay(4);//250hz
}
#else
///////////////////
//Game loop ///////
///(put game here)/

void loop()
{

#if PRINTFUNC
  Serial.println("f loop");
#endif
  if (isRunning)
  {
    eraseTail();
    moveSnake();
    if (!isBerryPlaced)
      placeBerry();
    xSemaphoreGive(rSem);
    delay(period);
  }
  else
  {
    vTaskDelete(rHandle);
    vTaskDelete(dHandle);
    vSemaphoreDelete(rSem);
    delay(2000);
    blink();
    tft.setCursor(40, 80);
    tft.setTextSize(7);
    tft.print(snakeLenght);
    //printSnakeArray();
    /*Serial.print("Berry: ");
    Serial.print(berry[0]);
    Serial.print(' ');
    Serial.println(berry[1]);*/
    delay(3000);
    tft.setTextSize(1);
    tft.setCursor(0, 231);
    tft.print("PRESS START");
    for (;;)
    {
      delay(10);
      readbtn();
      if (btn[0])
        break;
    }
    initSnake();
  }
}
#endif

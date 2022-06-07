//Debugging & Features///////////////////////////////////////////////
//#define BTTNTEST        //Switch to buttons test programm         |
//#define PERFMEASUREMENT //Enables performance measurement & output|
//#define LAYOUT_NEW      //Enables old button layout               |

//Libaries
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

//Screen
TFT_eSPI tft = TFT_eSPI(); //Screen library object
#define BLK_PIN 32         //Backlight pin
#define SCRSIZEX 240       //Screen size x
#define SCRSIZEY 240       //Screen size y
#define BKGCLR TFT_BLACK //Background color

//Button pins (NEW LAYOUT NUM)
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

//Button layout & state variables
#ifndef LAYOUT_NEW
const uint8_t _btnpin[10] = {BSTART, BSELECT, BDOWN, BRIGHT, BUP, BLEFT, BA, BB, BY, BX};
#else
const uint8_t _btnpin[10] = {BA, BB, BX, BY, BSTART, BSELECT, BLEFT, BRIGHT, BUP, BDOWN};
#endif
boolean btn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};     //Current buttons state
boolean prevbtn[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Previous buttons state
int8_t dpadY = 0;
int8_t dpadX = 0;
boolean stateChanged = 0;
uint8_t _btncounter; //counter for button array
boolean dpadStateChanged = 0;

//Button pins initialization procedure
void _initButtons()
{
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

//Dpad state update procedure
void updDPAD()
{
  dpadX = 0;
  dpadY = 0;
  if(btn[6])dpadX--;
  if(btn[7])dpadX++;
  if(btn[8])dpadY++;
  if(btn[9])dpadY--;
}

//Buttons state update procedure
void readbtn()
{
  for (_btncounter = 0; _btncounter < 10; _btncounter++){
    prevbtn[_btncounter] = btn[_btncounter];
    btn[_btncounter] = digitalRead(_btnpin[_btncounter]);
    if(btn[_btncounter] != prevbtn[_btncounter]){
      stateChanged = true;
      if(_btncounter>6)dpadStateChanged = 1;
    }
  }
  if(dpadStateChanged) updDPAD();
  return;
}

//Button test GUI layout
#ifdef BTTNTEST
#ifndef LAYOUT_NEW
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

//Random byte function
uint8_t rnd() { return (esp_random() >> 24); } // returns 0 - 255

//HSV to 16 bit RGB conversion function (thing that I use for cool gradients)
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

//Blink procedure for beautiful transitions
void blink(uint16_t toColor)
{
  tft.fillScreen(TFT_WHITE);
  delay(10);
  tft.fillScreen(toColor);
  delay(10);
}

#ifndef BTTNTEST
//Global Game Variables
//uint8_t level = 0; //TODO not implemented, for menus and multiple games

//Snake Game
//Color Pallete
#define SNAKECLR1 0x3353 //Snake color 1
#define SNAKECLR2 0xfea7 //Snake color 2
#define BERRYCLR TFT_RED //Berry color

const uint8_t fieldSize = 30; //square edge size
uint8_t snake[fieldSize * fieldSize][2]; //snake body coordinates array
uint16_t snakeLenght = 2; //current snake length
uint8_t direction = 2; //0 - down, 1 - right, 2 - up, 3 - left
uint8_t prevDirection = 2; //previous direction state
uint16_t period = 500; //game tick in ms
uint8_t berry[2] = {0, 0}; //berry coordinates {x, y}
uint8_t collider[2] = {0, 0}; //collider coordinates {x, y} (used for collision detection)
bool collision = 0; //(1 if collided)
bool isBerryPlaced = 0;
bool addLength = 0; //increments the length of the snake
bool isRunning = 1;
//FREE RTOS Handles & Semaphores
TaskHandle_t renderTaskHandle;
TaskHandle_t directionsTaskHandle;
SemaphoreHandle_t renderSem;

//Game Functions
//Erase tail procedure. Fills the last snake tile on the screen with background color.
void eraseTail() { tft.fillRect(snake[snakeLenght - 1][0] * 8, snake[snakeLenght - 1][1] * 8, 8, 8, BKGCLR); };

//Render task (FREE RTOS)
void tRenderSnake(void *p)
{
  for (;;)
  {
    eraseTail();
    for (uint16_t i = 0; i < snakeLenght; i++)
    {
      tft.fillRect(snake[i][0] * 8, snake[i][1] * 8, 8, 8, i % 2 == 0 ? SNAKECLR1 : SNAKECLR2);
    }
    tft.fillRoundRect(berry[0] * floor(SCRSIZEX / fieldSize), berry[1] * floor(SCRSIZEY / fieldSize), floor(SCRSIZEX / fieldSize), floor(SCRSIZEY / fieldSize), 1, BERRYCLR);
    xSemaphoreTake(renderSem, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}

//Direction change task (FREE RTOS)
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

//Berry Placement procedure
void placeBerry()
{
  do
  {
    berry[0] = floor(rnd() % fieldSize);
    berry[1] = floor(rnd() % fieldSize);
    isBerryPlaced = 1; 
    for (uint16_t i = 0; i < snakeLenght; i++)
      if (snake[i][0] == berry[0] && snake[i][1] == berry[1])
      {
        isBerryPlaced = 0;
        break;
      }
  } while (!isBerryPlaced);
}

//Game initialization procedure
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
  blink(BKGCLR);
  xTaskCreatePinnedToCore(tRenderSnake, "render", 4000, NULL, 0, &renderTaskHandle, 1);
  vSemaphoreCreateBinary(renderSem);
  xTaskCreate(tDirChange, "dir", 1000, NULL, 0, &directionsTaskHandle);
  delay(1000);
  isRunning = 1;
}

//Snake movement procedure
void moveSnake()
{
  //Border collision detection
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
    isRunning = 0;
    return;
  }
  //Body collision detection
  for (uint16_t i = 3; i < snakeLenght; i++)
  {
    if (collider[0] == snake[i][0] && collider[1] == snake[i][1])
    {
      collision = 1;
      isRunning = 0;
      return;
    }
  }
  //Berry collision detection 
  if (collider[0] == berry[0] && collider[1] == berry[1])
  {
    addLength = 1;
    isBerryPlaced = 0;
  }
  //Movement
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
  }
}
#endif

//Default Arduino setup procedure. Executes on startup
void setup()
{
  //Debug
  Serial.begin(115200);
#ifdef PERFMEASUREMENT
  //Time measure
  uint32_t time = 0;
  time += micros();
#endif

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
  blink(TFT_BLACK);

  //Buttons initialization
  _initButtons();

#ifdef PERFMEASUREMENT
  time = micros() - time;
  Serial.println(time);
  Serial.println(micros());
#endif


  //Game initialization
#ifndef BTTNTEST
  initSnake();
#endif
  return; //initialization end
}

#ifdef BTTNTEST
//buttons test
void loop()
{
  readbtn();
  for (int32_t i = 0; i < 10; i++)
    tft.drawRect(_btnTestBox[i][0], _btnTestBox[i][1], _btnTestBox[i][2], _btnTestBox[i][3], btn[i] == 1 ? TFT_RED : TFT_DARKGREY);
  
  delay(4);//250hz
}
#else

//Game loop
void loop()
{
  if (isRunning)
  {
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

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
//uint8_t t = 0;
//uint8_t relx = 0;
//uint8_t rely = 0;
boolean bSel = 0;
uint8_t box[2]={239 - 16, 230 - 16};
uint8_t pos[2]={0,0};
uint8_t estPos[2]={0,0};
uint8_t spd[2]={1,1};
uint8_t curCol = 0;
uint8_t curDel = 10;
uint8_t curBrightness = 0;
uint16_t clib[12]=
{ tft.color565(255,0,0),
  tft.color565(255,128,0),
  tft.color565(255,255,0),
  tft.color565(128,255,0),
  tft.color565(0,255,0),
  tft.color565(0,255,128),
  tft.color565(0,255,255),
  tft.color565(0,128,255),
  tft.color565(0,0,255),
  tft.color565(128,0,255),
  tft.color565(255,0,255),
  tft.color565(255,0,128)};

void IRAM_ATTR bSelI(){ bSel = !bSel; }

void setup()   {
  pinMode(14,OUTPUT);
  digitalWrite(14,HIGH);
  //pinMode(32,INPUT_PULLDOWN);
  //attachInterrupt(32,bSelI,RISING);
  //Set up the display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
}

void loop()
{
  tft.fillScreen(TFT_BLACK);
  delay(1000);
  tft.fillScreen(TFT_WHITE);
  delay(1000);
  /*
  tft.fillRect(pos[0],pos[1],16,16,TFT_BLACK);
  estPos[0] = pos[0] + spd[0];
  estPos[1] = pos[1] + spd[1];
  if(estPos[0]>box[0]||estPos[0]<0){
    spd[0] = -spd[0];
    estPos[0] = pos[0] + spd[0];
    curCol++;
    if(curCol>11)curCol = 0;
    }
  if(estPos[1]>box[1]||estPos[1]<0){
    spd[1] = -spd[1];
    estPos[1] = pos[1] + spd[1];
    curCol++;
    if(curCol>11)curCol = 0;
    }
  pos[0] = estPos[0];
  pos[1] = estPos[1];
  tft.fillRect(pos[0],pos[1],16,16,clib[curCol]);
  delay(10);*/
  //analogWrite(14,curBrightness);
}
/*
void colorsTest() {
  for(uint8_t y = 0; y<240;y++){
    rely = (uint8_t)((double_t)y/240*255);
    for (uint8_t x = y%2; x < 240; x++){
    relx = (uint8_t)((double_t)x/240*255);
    t=(uint8_t)relx*((double_t)(255 - rely)/255);
    tft.drawPixel(x,y,tft.color565(255 - rely, t, t));
  }
  }
  delay(7500);
  for(uint8_t y = 0; y<240;y++){
    rely = (uint8_t)((double_t)y/240*255);
    for (uint8_t x = y%2; x < 240; x++){
    relx = (uint8_t)((double_t)x/240*255);
    t=(uint8_t)relx*((double_t)(255 - rely)/255);
    tft.drawPixel(x,y,tft.color565(t, 255 - rely, t));
  }
  }
  delay(7500);
  for(uint8_t y = 0; y<240;y++){
    rely = (uint8_t)((double_t)y/240*255);
    for (uint8_t x = y%2; x < 240; x++){
    relx = (uint8_t)((double_t)x/240*255);
    t=(uint8_t)relx*((double_t)(255 - rely)/255);
    tft.drawPixel(x,y,tft.color565(t, t, 255 - rely));
  }
  }
  delay(7500);
}*/

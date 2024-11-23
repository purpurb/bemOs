#include <ESP32Lib.h>
#include "logoMini.h"
#include "avaMini.h"
#include <Update.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Ressources/CodePage437_8x16.h>
#include <PS2Kbd.h>

const int redPin = 15;
const int greenPin = 2;
const int bluePin = 4;
const int hsyncPin = 32;
const int vsyncPin = 33;

const int keyboardDataPin = 17;
const int keyboardClockPin = 16;

int logoX = random(0,477);
int logoY = random(0,359);
int xSpeed = 1;
int ySpeed = 1;

int avaX = random(0,592);
int avaY = random(0,352);
int axSpeed = -1;
int aySpeed = -1;

VGA3BitI vga;
PS2Kbd keyboard(keyboardDataPin,keyboardClockPin);

void flashOs(){
  if(keyboard.available()){
    char c = keyboard.read();
    File file = SD.open("/bemOs.bin");
    if (c == '\r'){
      if(!file || file.isDirectory()){
          vga.println("error, cant flash firmware");
      }
      while(file.available()){
          Update.begin(file.size(),U_FLASH);
          Update.writeStream(file);
          if (Update.end()){
            vga.println("succesfuly flashed , rebooting");
            file.close();
            abort();
        }
      }
    }
  }
}

void setup() {
  vga.init(vga.MODE640x400, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
  keyboard.begin();
  vga.setFont(CodePage437_8x16);
  vga.setCursor(0,0);
  pinMode(22,OUTPUT);
  if (!SD.begin(SS)){
    vga.println("failed to init sd");
    //while(1);
  }else{
    vga.println("inited sd");
    digitalWrite(22,HIGH);
  }
}

void loop() {
  flashOs();
  logoMini.draw(vga,0,logoX,logoY);
  //avaMini.draw(vga,0,avaX,avaY);
  
  logoX += xSpeed;
  logoY += ySpeed;
  if (logoX > 477){
    xSpeed = -1;
  }
  if (logoY > 359){
    ySpeed = -1;
  }
  if (logoX < 1){
    xSpeed = 1;
  }
  if (logoY < 1){
    ySpeed = 1;
  }
  
  avaX += axSpeed;
  avaY += aySpeed;
  if (avaX > 592){
    axSpeed = -1;
  }
  if (avaY > 352){
    aySpeed = -1;
  }
  if (avaX < 1){
    axSpeed = 1;
  }
  if (avaY < 1){
    aySpeed = 1;
  }
  vga.show();
}

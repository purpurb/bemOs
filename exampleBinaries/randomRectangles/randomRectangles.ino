#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Update.h>
#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x16.h>
#include <PS2Kbd.h>

const int redPin = 15;
const int greenPin = 2;
const int bluePin = 4;
const int hsyncPin = 32;
const int vsyncPin = 33;
const int keyboardDataPin = 17;
const int keyboardClockPin = 16;

VGA3BitI vga;
PS2Kbd keyboard(keyboardDataPin,keyboardClockPin);

#include <Update.h>
#include "SD.h"

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

void setup()
{
	vga.init(vga.MODE640x400, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
  vga.setFont(CodePage437_8x16);
  keyboard.begin();
  
  if (SD.begin(SS)){
    vga.println("inited sd");
  }
  else{
    vga.println("failed to init sd");
    while(1);
  }
}

void loop()
{
  vga.rect(random(0,640),random(0,400),random(0,640),random(0,400),vga.RGB(random(0,255),random(0,255),random(0,255)));
  vga.show();
  flashOs();
  delay(50);
}

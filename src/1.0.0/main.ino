#include <Update.h>
#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x16.h>
#include <PS2Kbd.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "images.h"

bool shellMenu = true;
bool debounce = true;

int dat;
String workDir = "/";
String kbdInput = "";
bool onc = true;
bool parsed = false;

// pin config
const int redPin = 15;
const int greenPin = 2;
const int bluePin = 4;
const int hsyncPin = 32;
const int vsyncPin = 33;
//vga pins

//buzzer pin
const int buzzerPin = 13;
//buzzer pin

//ps/2 keyboard pins
const int keyboardDataPin = 17;
const int keyboardClockPin = 16;
//pin config

//declaring classes
PS2Kbd keyboard(keyboardDataPin,keyboardClockPin);
VGA3BitI vga;
//declaring classes

void beep(int freq, int count, int del = 100,int del2 = 100){ //fu
  while (count > 0){
    delay(del);
    ledcWriteTone(1,freq);
    delay(del2);
    ledcWriteTone(1,0);
    count -= 1;
  }
}

void setup(){  
  ledcSetup(1,8000,10);
  ledcAttachPin(buzzerPin,1);

	vga.init(vga.MODE640x400, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
  keyboard.begin();
  vga.setFont(CodePage437_8x16);

  images.draw(vga,0,316,205);
  vga.println("booting bemOS v1.0");

  beep(1000,1);
  delay(500);
  
  if (!SD.begin(SS)){
    vga.println("failed to mount SD card");
    beep(1500,3);
    while(1);
  }
  else{
    vga.println("mounted SD card");
    beep(800,1);
    beep(1000,1,0,100);
  }
  if (!SD.exists("/appdata")){
    SD.mkdir("/appdata");
    vga.println("created /appdata dir");
  }
  Serial.begin(115200);
  delay(3000);
}

String strFromFile (File fil){
  String currentLine = "";
  String str = "";
  bool iterating = true;
  while (iterating == true){
    bool availabl = fil.available();
    char c = fil.read();
    if (c and availabl and c != '\n'){
      currentLine += c;
    }
    availabl = fil.available();
    if (c == '\n' or availabl == false){
      str += "\n" + currentLine;
      currentLine = "";
    }
    if (availabl == false){
     iterating = false;
    }
  }
  return str;
}

void printFromFile(File file){
  vga.print(file.name());
  vga.println(" contents:");
  vga.println(strFromFile(file).c_str());
}

//void fileManager(){
//  bool runnning = true;
//  bool debounce = true;
//  bool waitForInput = false;
//  String filecontent = "";
//  int buf = 0;
//  shellMenu = false;
//  while (runnning){
//    if (debounce == true){
//      vga.clear(vga.RGB(0,0,0));
//      vga.setCursor(198,0);
//      vga.show();
//      vga.fillRect(0,0,640,16,vga.RGB(255,255,255));
//      vga.setTextColor(vga.RGB(0,0,0),vga.RGB(255,255,255));
//      vga.println("kubik file editor v1.0");
//      vga.setCursor(0,16);
//      vga.print("f1 to save file   f2 to open file   esc to exit");
//      vga.setCursor(0,32);
//      vga.setTextColor(vga.RGB(255,255,255),vga.RGB(0,0,0));
//      vga.print(filecontent.c_str());
//      buf = 0;
//      debounce = false;
//    }
//    if (keyboard.available()){
//      Serial.write(keyboard.read());
//      char c = keyboard.read();
//      if (c == 8 or c == 127 and waitForInput == false){
//        filecontent.remove(filecontent.length()-1,filecontent.length());
//        debounce = true;
//      }
//      
//      else if (c == '\n' and waitForInput == false){
//        filecontent += "\n";
//        if (buf > 0){
//          int ost = 80-buf;
//          buf = 0;
//          while (ost > 0){
//            vga.print(" ");
//            ost -= 1;
//          }
//        }
//      }
//      else{
//        filecontent += c;
//        vga.print(c);
//        buf += 1;
//      }
//    }
//  }
//}

void sysinfo(){
  vga.clear(vga.RGB(0,0,0));
  images.draw(vga,1,170/2,56);
  images.draw(vga,2,170+24,56);

  vga.fillRect(0,0,32,32,vga.RGB(255,0,0));
  vga.fillRect(32*1,0,32,32,vga.RGB(255,255,0));
  vga.fillRect(32*2,0,32,32,vga.RGB(255,255,255));
  vga.fillRect(32*3,0,32,32,vga.RGB(0,255,255));
  vga.fillRect(32*4,0,32,32,vga.RGB(0,0,255));
  vga.fillRect(32*5,0,32,32,vga.RGB(255,0,255));
  vga.fillRect(32*6,0,32,32,vga.RGB(0,255,0));
 
  vga.setCursor(0,48+32);
  vga.println("running bemOs v1.0-pre");
  vga.println("purpurb | 2024");
  vga.println("");
  vga.println("3bit color pallete");
  vga.print("free memory heap: ");
  vga.print(ESP.getFreeHeap());
  vga.println(" bytes");

  vga.print("mcu revision: ");
  vga.println(ESP.getChipRevision());

  vga.print("current resolution: ");
  vga.print(vga.xres);
  vga.print("x");
  vga.println(vga.yres);
}

void parsei(String input){
  kbdInput = "";
  if (input == "list"){
    vga.println("list of files:");
    File root = SD.open(workDir);
    File file = root.openNextFile();
    while(file){
      vga.print("file: ");
      vga.println(file.name());
      vga.print("  size: ");
      vga.print(file.size());
      vga.println(" bytes");
      file = root.openNextFile();
    }
  }
  else if (input.startsWith("printFile")){
    input.remove(0,10);
    File inpFile = SD.open(input);
    printFromFile(inpFile);
  }
  else if (input.startsWith("delete")){
    input.remove(0,7);
    if (SD.remove(workDir+input)){
      vga.println(String("deleted "+input).c_str());
    }else{
      vga.println("can't delete: not found");
    }
  }
  else if (input.startsWith("cd")){
    input.remove(0,3);
    if (SD.exists(input)){
      workDir = input;
      vga.print("changed directory to ");
      vga.println(workDir.c_str());
    }else{
      vga.println("can't change directory: not found");
    }
  }
  else if (input.startsWith("mkdir")){
    input.remove(0,6);
    if (SD.mkdir(workDir+input)){
      vga.println(String("created directory "+input).c_str());
    }else{
      vga.println("can't create directory");
    }
  }
  else if (input.startsWith("rmdir")){
    input.remove(0,6);
    if (SD.rmdir(workDir+input)){
      vga.println(String("deleted directory "+input).c_str());
    }else{
      vga.println("can't delete directory");
    }
  }
 else if (input.startsWith("rename")){
    input.remove(0,7);
    File file = SD.open(workDir+input);
    if (file){
      vga.println("new name: ");
      bool waiting = true;
      while (waiting){
        if (keyboard.available()){
          char c = keyboard.read();
          if (c == '\n'){
            SD.rename(workDir+input,workDir+kbdInput.c_str());
            vga.print("renamed ");
            vga.print(input.c_str());
            vga.print(" to ");
            vga.println(kbdInput.c_str());
            file.close();
            waiting = false;
          }
          else{
            kbdInput += c;
            vga.print(c);
          }
        }
      }
    }else{
      vga.println("can't rename: not found");
    }
  }
  else if (input.startsWith("copy")){
    input.remove(0,5);
    File origFile = SD.open(workDir+input);
    if (origFile){
      vga.print("file copy name: ");
      bool waiting = true;
      kbdInput = "";
      while (waiting){
        if (keyboard.available()){
          char c = keyboard.read();
          if (c == '\n'){
            File fileCopy = SD.open(workDir+kbdInput,FILE_WRITE);
            vga.println("");
            vga.println("please wait, writing data");
            while ((dat = origFile.read()) >= 0){
              fileCopy.write(dat);
            }
            vga.print("created copy of ");
            vga.println(input.c_str());
            origFile.close();
            fileCopy.close();
            waiting = false;
          }
          else{
            kbdInput += c;
            Serial.println(kbdInput);
            vga.print(c);
          }
        }
      }
    }else{
      vga.println("can't copy: not found");
    }
  }
  else if (input == "sysinfo"){
    sysinfo();
  }
  else if (input == "reset" or input == "restart"){
    abort();
  }
  else if (input == "help" or input == "?"){
    vga.println("reset/restart - reboot");
    vga.println("list - displays list of files");
    vga.println("delete </file> - deletes file");
    vga.println("sysinfo - displays system info");
    vga.println("rename (1st input:) </file> (2nd input:) </newname> - renames file");
    vga.println("copy (1st input:) </file> (2nd input:) </copyname> - duplicates file");
    vga.println("mkdir <name> - creates directory");
    vga.println("rmdir </dir> - deletes directory");
    vga.println("help - displays all commands");
    vga.println("printFile </file> - prints file contents");
  }
  
  else{
    vga.println("please wait...");
    File file = SD.open(workDir+input);
    if(!file || file.isDirectory()){
        vga.println("can't open: not a file or command");
    }
    while(file.available()){
      vga.println("writing dat , please wait...");
      Update.begin(file.size(),U_FLASH);
      Update.writeStream(file);
      if (Update.end()){
        vga.println("rebooting...");
        file.close();
        abort();
      }
    }
  }
  parsed = true;
}

void shellM(){
  if (shellMenu == true){
    if (onc == true){
      onc = false;
      vga.clear(vga.RGB(0,0,0));
      vga.setCursor(0,0);
      vga.println("welcome to bemOs!");
      vga.println("type filename to execute it , or command");
      vga.println("or type ''help'' to display all commands");
      vga.println();
      vga.print(workDir.c_str());
      vga.print("> ");
      vga.print(kbdInput.c_str());
    }
    if (keyboard.available()){
      char cRead = keyboard.read();
      if (parsed == true){
        onc = true;
        parsed = false;
      }
      if (cRead == 8 or cRead == 127){
        kbdInput.remove(kbdInput.length()-1,kbdInput.length());
        onc = true;
      }
      else if (cRead == '\n'){
        vga.println("");
        parsei(kbdInput);
        kbdInput = "";
      }
      else{
        kbdInput += cRead;
        vga.print(cRead);
      }
    }
  }
}

void loop()
{
  shellM();
}

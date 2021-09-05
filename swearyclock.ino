//
// UNPLUG SPEAKER CONNECTOR TO MAKE THE PROGRAMMER WORK.
//
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <LittleFS.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include "AudioFileSourceLittleFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"


AudioFileSourceLittleFS *file;
AudioOutputI2SNoDAC *out;
AudioGeneratorMP3 *mp3;


const byte interruptPin = D7;

int doCuckoo = 0;

File swrf;

char fstr[14];
char sstr[4];
uint32_t fsize;
String fnr;
long fnn;

 
void setup() {

  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  LittleFS.begin();
    
  pinMode(5, OUTPUT);  // Motor A
  pinMode(0, OUTPUT);  // Direction
  
  digitalWrite(5, 0);
  digitalWrite(0, 1);
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();

  Serial.println("\nDir listing:");
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName());
    if(dir.fileSize()) {
        Serial.print(" - ");
        File f = dir.openFile("r");
        Serial.println(f.size());
    }
  }

  swrf = LittleFS.open("/swearno.txt", "r");
  if (swrf) {
    Serial.println("\nFile Content:");
    while (swrf.available()) {
      fnr = swrf.readStringUntil('\n');
      fnn = fnr.toInt();
      Serial.println(fnn);
    }
   swrf.close();
  }
  else {
    Serial.println("Failed to open file for reading");
  }
 
  pinMode(interruptPin, INPUT);
  Serial.println("Bing!");
}


void cMove(int dir) {
  digitalWrite(0, dir); // Direction - 1 out, 0 in
  analogWrite(5, 1023); // Cuckoo out...
  delay(200);
  analogWrite(5, 0);    // Stop cuckoo motor.
}

void cuckooGubbins() {
  sprintf(fstr,"/swearf-%02d.mp3",fnn);
  Serial.println(fstr);
  file = new AudioFileSourceLittleFS(fstr);

  cMove(1);
  fsize = file->getSize();
  Serial.println(fsize);
    
  mp3->begin(file, out);
  while (mp3->isRunning()) {
    // Serial.println("Mid bong..");
    if (!mp3->loop()) mp3->stop(); 
  } 
  Serial.println("End Bong!");
  cMove(0);
   
  fnn++;
  if (fnn > 13) {
    fnn = 1;
  }
  swrf = LittleFS.open("/swearno.txt","w");
  if (swrf) {
    sprintf(sstr,"%02d",fnn);
    swrf.println(sstr);
    swrf.close();
  }
  else {
    Serial.println("Failed to open file for writing");
  }
  
}

void loop() {

  // Thrash-test uncomment this. 
  // doCuckoo = 1;
  doCuckoo = digitalRead(interruptPin);
  
  if (doCuckoo == 1) {
    Serial.println("More Bong!");

    cuckooGubbins();
    
    doCuckoo = 0;
    Serial.println("Deep sleep..");
    ESP.deepSleep(300e6);
    Serial.println("AWAKE!");
  }
}

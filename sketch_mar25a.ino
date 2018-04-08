// Interrupts
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include "SPIFFS.h"
#else
  #include <ESP8266WiFi.h>
#endif
#include "AudioFileSourceSPIFFS.h"
//#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"


AudioFileSourceSPIFFS *file;
AudioOutputI2SNoDAC *out;
AudioGeneratorMP3 *mp3;
//AudioFileSourceID3 *id3;

const byte interruptPin = 13;
volatile byte interruptCounter = 0;
int doCuckoo = 0;
int fn = 1;
long debouncing_time = 15; //Debouncing Time in Milliseconds
volatile unsigned long last_micros;
char fstr[14];
uint32_t fsize;

 
void setup() {

  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  // SPIFFS.begin();
    
  pinMode(5, OUTPUT);  // Motor A
  pinMode(0, OUTPUT);  // Direction
  
  digitalWrite(5, 0);
  digitalWrite(0, 1);

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  Serial.println("Bing!");
}


void handleInterrupt() {
  if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    Interrupt();
    last_micros = micros();
  }
}

void Interrupt() {
  doCuckoo = 1;
  Serial.println("Bong!");
}

void cMove(int dir) {
  digitalWrite(0, dir); // Direction - 1 out, 0 in
  analogWrite(5, 1023); // Cuckoo out...
  delay(200);
  analogWrite(5, 0);    // Stop cuckoo motor.
}

void loop() {

  if (doCuckoo == 1) {
    detachInterrupt(digitalPinToInterrupt(interruptPin));
    Serial.println("More Bong!");
    
    sprintf(fstr,"/swear-%02d.mp3",fn);
    Serial.println(fstr);
    file = new AudioFileSourceSPIFFS(fstr);
    out = new AudioOutputI2SNoDAC();
    mp3 = new AudioGeneratorMP3();

    cMove(1);

    fsize = file->getSize();
    Serial.println(fsize);
    
    mp3->begin(file, out);
    while (mp3->isRunning()) {
      // Serial.println("Mid bong..");
      if (!mp3->loop()) mp3->stop(); 
    } 
    Serial.println("End Bong!");
    
    fn++;
    if (fn > 12) {
      fn = 1;
    }

    cMove(0);
    
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
    doCuckoo = 0;
  }
  
}





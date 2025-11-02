#include <Arduino.h>         
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>

RTC_DS1307 rtc;
Adafruit_BME280 bme;


#define pinCS 10 // CS pin
#define BME_SDA A4
#define BME_SCL A5

const unsigned long INTERVAL = 600000;// 10 mins in milliseconds
unsigned long lastLog = 0; 
const char* filename = "datalog.csv"; 

void errorLoop() {
  while (true) {
    delay(1000);
    Serial.println(F("SYSTEM HALTED - Fix issue and reset!"));
  }
}

void setup() {

Serial.begin(9600);
  // Serial Start delay
  unsigned long start = millis();
  while (!Serial && (millis() - start < 2000)) delay(10);

  if (Serial) {
    Serial.println(F("=== NANO LOGGER ==="));
  }

  


//I2C Bus start
Wire.begin();

  if (!bme.begin(0x76)) {  
    Serial.println(F("BME280 not found! Check wiring/address."));
    errorLoop();
    }
  
  Serial.println(F("BME280 initialized"));

  // RTC INIT 
  if (!rtc.begin()) {
    Serial.println(F("RTC not found! Check wiring."));
    errorLoop();
  }

  DateTime now = rtc.now();
  Serial.print(F("RTC Now: "));
  Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));

  Serial.println(F("RTC initialized"));

  //  SD CARD INIT 
  if (!SD.begin(pinCS)) {
    Serial.println(F("SD Card failed! Check wiring, card, or format."));
    errorLoop();
  }
  Serial.println(F("SD Card initialized"));

  // SLOW SPI TO 4MHz
SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
Serial.println(F("SPI clock set to 4MHz"));

if (!SD.exists(filename)) {
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
      file.println(F("Date,Time,Temp_C,Hum_%"));
      file.close();
      Serial.println(F("Created new log file: datalog.csv"));
    } 
      else {
      Serial.println(F("Failed to create file!"));
      errorLoop();
    
  }

 

}
}


void loop() {
  unsigned long now = millis();

  // Every 10 minutes
  if (now - lastLog >= INTERVAL) {
    logReading();
    lastLog = now;
  }

 
  delay(100);
}

void logReading() {
  // Read sensors
  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  DateTime now = rtc.now();

  
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    // Format: 2025-10-27,20:30:45,23.5,65.2
    file.print(now.year()); file.print("-");
    printTwoDigits(file, now.month()); file.print("-");
    printTwoDigits(file, now.day()); file.print(",");

    printTwoDigits(file, now.hour()); file.print(":");
    printTwoDigits(file, now.minute()); file.print(":");
    printTwoDigits(file, now.second()); file.print(",");

    file.print(temp, 1);
    file.print(",");
    file.println(hum, 1);

    file.close();

    // Serial feedback
    Serial.print(F("LOGGED: "));
    Serial.print(now.timestamp(DateTime::TIMESTAMP_FULL));
    Serial.print(F(" | "));
    Serial.print(temp, 1); Serial.print(F("°C | "));
    Serial.print(hum, 1); Serial.println(F("%"));
  } else {
    Serial.println(F("ERROR: Could not open file for writing!"));
  }
}


void printTwoDigits(File &file, int value) {
  if (value < 10) file.print("0");
  file.print(value);
}












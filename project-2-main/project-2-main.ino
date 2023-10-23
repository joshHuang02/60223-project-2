/*
Project LowKey
by Josh Huang

This is the controller code for the LowKey lock-out prevention device.
Its main function is to read inputs from a VL53L0X laser distance reader, door sensor, and MFRC522 RFID reader to determine when
the user is attempting to leave their residence without their key. Thus preventing locking the user out.
The output is a micro servo that raises a physical blocker to stop the user from leaving the residence without their key.

This file includes example code from Adafruit_VL53L0X and MFRC522 libraries

Pin map starting line 28

Copyright 2023 Junzhe Huang

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Adafruit_VL53L0X.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// pin map (MFRC522 RFID reader requires using I2C pins not listed here)
#define DOOR_PIN        2
#define SERVO           3
#define RST_PIN         5
#define SS_PIN          10

Servo servo;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// limiter values
int handleDistLimit = 130;
int rfidTimeoutMS = 2000;
int servoDefault = 150;
int servoTimeoutMS = 5000;

// vars
unsigned long rfidScanTime = 0;
unsigned long servoTime = 0;

// status booleans
bool rfid;
bool handleTurned;
bool doorOpen;
bool doorBlocked;
bool servoTriggered;

// most lines in setup are taken from example code
void setup() {
  Serial.begin(115200);

  pinMode(DOOR_PIN, INPUT);

  //Servo
  servo.attach(SERVO);
  servo.write(servoDefault);

  // laser 
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
  
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 

  // RFID Reader
  while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}


void loop() {
  // read inputs and set conditonal booleans
  readLaser(); // reads laser distance measurement and sets handleTurned bool
  readRFID(); // scans for rfid tag and sets rfid bool
  doorOpen = !digitalRead(DOOR_PIN);
  bool servoTimedOut = millis() > servoTime + servoTimeoutMS;

  // printStatus();

  // control blocker
  if (!doorBlocked) { // blocked down
    if (handleTurned && !doorOpen && !rfid) {
      servoTime = millis();
      servo.write(servoDefault - 65); // lift blocker
      doorBlocked = true;
    }
  } else { //blocker up
    if ((servoTimedOut && !handleTurned) || rfid || doorOpen) {
      servo.write(servoDefault); // drop blocker
      doorBlocked = false;
    }
  }
}

// reads the laser distance to flag on door handle, update the handleTurned boolean 
void readLaser() {
  VL53L0X_RangingMeasurementData_t measure;
    
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
    handleTurned = measure.RangeMilliMeter > handleDistLimit;
  }
}

void readRFID() {
  // RFID Reader
  // set rfid bool
  rfid = millis() < rfidTimeoutMS + rfidScanTime;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) return;

  // At this point there is a RFID device present
  rfidScanTime = millis();
	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}
}

// debugging status of each input
void printStatus() {
  Serial.print("Handle Turned: ");
  Serial.print(handleTurned);
  Serial.print(" | RFID: ");
  Serial.print(rfid);
  Serial.print(" | Door Open: ");
  Serial.println(doorOpen);
}

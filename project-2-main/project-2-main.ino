#include "Adafruit_VL53L0X.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define DOOR_PIN        2
#define SERVO           3
#define RST_PIN         5          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

Servo servo;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// vars
int handleDistLimit = 110;
int rfidTimeoutMS = 2000;
int servoDefault = 150;
int servoTimeoutMS = 5000;

unsigned long rfidScanTime = 0;
unsigned long servoTime = 0;

bool rfid = false;
bool handleTurned = false;
bool doorOpen = false;
bool doorBlocked = false;

bool servoTriggered;

void setup() {
  Serial.begin(115200);

  pinMode(DOOR_PIN, INPUT);

  //Servo
  servo.attach(SERVO);
  servo.write(servoDefault);

  // Lazer 
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
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}


void loop() {
  // delay(10);

  // read inputs and set conditonal booleans
  readLazer(); // reads lazer distance measurement and sets handleTurned bool
  readRFID(); // scans for rfid tag and sets rfid bool
  doorOpen = !digitalRead(DOOR_PIN);
  bool servoTimedOut = millis() > servoTime + servoTimeoutMS;

  // print status
  printStatus();

  if (!doorBlocked) { // blocked down
    if (handleTurned && !doorOpen && !rfid) {
      servoTime = millis();
      servo.write(servoDefault - 100);
      doorBlocked = true;
    }
  } else { //blocker up
    if ((servoTimedOut && !handleTurned) || rfid || doorOpen) {
      servo.write(servoDefault);
      doorBlocked = false;
    }
  }

  // if (handleTurned && !doorOpen && !rfidPresent && !servoTriggered && !servoTimedOut) {
  //   // servoTriggered = true;
  //   servoTime = millis();

  //   servo.write(servoDefault + 90);
  // } else {
  //   servo.write(servoDefault);
  // }
}

void readLazer() {
  // lazer
  VL53L0X_RangingMeasurementData_t measure;
    
  // Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    // Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
    handleTurned = measure.RangeMilliMeter > handleDistLimit;
  } else {
    // Serial.println(" out of range ");
  }
    
  // delay(100);
}

void readRFID() {
  // RFID Reader
  // set rfid bool
  rfid = millis() < rfidTimeoutMS + rfidScanTime;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
    // Serial.println("Not Present");
		return;
	}

  // Serial.println("Present");
  rfidScanTime = millis();
	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

	// Dump debug info about the card; PICC_HaltA() is automatically called
	// mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

void printStatus() {
  Serial.println("Handle Turned: ");
  Serial.print(handleTurned);
  Serial.print(" | RFID: ");
  Serial.print(rfid);
  Serial.print(" | Door Open: ");
  Serial.println(doorOpen);
}

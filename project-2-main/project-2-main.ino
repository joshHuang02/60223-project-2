#include "Adafruit_VL53L0X.h"
#include <SPI.h>
#include <MFRC522.h>

#define DOOR_PIN        2
#define RST_PIN         5          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// vars
int doorLimit = 80;
bool rfid = false;

void setup() {
  Serial.begin(115200);

  pinMode(DOOR_PIN, INPUT);

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
  readLazer();
  readRFID();
  if (digitalRead(DOOR_PIN)) Serial.println("Door closed");
}

void readLazer() {
  // lazer
  VL53L0X_RangingMeasurementData_t measure;
    
  // Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    // Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println(" out of range ");
  }
    
  // delay(100);
}

void readRFID() {
  // RFID Reader
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

  Serial.println("Present");
	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
    
		return;
	}

	// Dump debug info about the card; PICC_HaltA() is automatically called
	// mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

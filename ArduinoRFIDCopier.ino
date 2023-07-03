/*
* -----------------------------------------------------------------------------------------
*             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
*             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
* Signal      Pin          Pin           Pin       Pin        Pin              Pin
* -----------------------------------------------------------------------------------------
* RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
* SPI SS      SDA(SS)      10            53        D10        10               10
* SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
* SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
* SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h> // Include Libs
#include <MFRC522.h>
#include "GyverButton.h"

#define SS_PIN 10 // Configure Pins
#define RST_PIN 9
#define BTN_PIN 8
#define LED_MODE_PIN 7
#define LED_READ_PIN 6
#define LED_WRITE_PIN 5

GButton butt(BTN_PIN);
 
MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key; 

byte uidPICC[4];
bool mode = 0;

void setup() { // Setup part
  pinMode(LED_MODE_PIN, OUTPUT);
  pinMode(LED_READ_PIN, OUTPUT);
  pinMode(LED_WRITE_PIN, OUTPUT);
  
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code work with the MIFARE UID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop() { // Main Loop
  butt.tick(); // Update button state and change mode
  if (butt.isSingle()) { mode = !mode; }
  digitalWrite(LED_MODE_PIN, mode);
  
  if ( ! rfid.PICC_IsNewCardPresent()) { return; } // If no RFID detected, skip

  if ( ! rfid.PICC_ReadCardSerial()) { return; }

  digitalWrite(LED_READ_PIN, 1);
  Serial.println("------------------------------------");

  Serial.print(F("PICC type: ")); // Print RFID about and data
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE."));
    return;
  }

  if (mode == 0) { // Read RFID
    if (rfid.uid.uidByte[0] != uidPICC[0] || 
      rfid.uid.uidByte[1] != uidPICC[1] || 
      rfid.uid.uidByte[2] != uidPICC[2] || 
      rfid.uid.uidByte[3] != uidPICC[3]) {
      Serial.println(F("A new card has been detected."));
      
      for (byte i = 0; i < 4; i++) {
        uidPICC[i] = rfid.uid.uidByte[i];
      }
     
      Serial.println(F("The UID tag is:"));
      Serial.print(F("In hex: "));
      printHex(rfid.uid.uidByte, rfid.uid.size);
      Serial.println();
      
    }
    else { 
      Serial.println(F("Card read previously."));
      Serial.println(F("The UID tag is:"));
      Serial.print(F("In hex: "));
      printHex(rfid.uid.uidByte, rfid.uid.size);
      Serial.println();
    }
  } else if (mode == 1) {  // Write to RFID
      digitalWrite(LED_WRITE_PIN, 1);
      if ( rfid.MIFARE_SetUid(uidPICC, (byte)4, true) ) {
        Serial.println(F("Wrote new UID to card."));
      } 
      mode = 0;
      delay(500);
      digitalWrite(LED_WRITE_PIN, 0);
  }
  
  rfid.PICC_HaltA(); // Update
  rfid.PCD_StopCrypto1();
  digitalWrite(LED_READ_PIN, 0);
}

void printHex(byte *buffer, byte bufferSize) { // Function to print HEX in serial
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

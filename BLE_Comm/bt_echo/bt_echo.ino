#include <SoftwareSerial.h>

#define BT_RST 4
#define LED_PIN_1 8
#define LED_PIN_2 9
#define RX_PIN 12
#define TX_PIN 11
#define MOD_BAUD 19200
#define BT_BAUD 19200

SoftwareSerial moduleSerial(RX_PIN, TX_PIN);

void setup() {
  // Initialize pins
  pinMode(BT_RST, OUTPUT);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  digitalWrite(BT_RST, HIGH);

  // Initialize serial ports
  Serial.begin(BT_BAUD);
  moduleSerial.begin(MOD_BAUD);
  delay(20);
}

// Two-way echo
//void loop() {
//  // Laptop to module
//  if (moduleSerial.available()) {
//    Serial.write(moduleSerial.read());
//    Serial.flush(); // Waits for the transmission of outgoing serial data to complete
//  }
//  
//  // Module to laptop
//  if (Serial.available()) {
//    moduleSerial.write(Serial.read());
//  }
//}

// Echo's BT messages back over BT and to Arduino
void loop() {
  // Echo
  if (Serial.available()) {
    byte c = Serial.read();
    Serial.write(c);
    moduleSerial.write(c);
  }
}

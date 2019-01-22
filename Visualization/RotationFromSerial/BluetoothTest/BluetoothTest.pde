  
// Example by Tom Igoe

import processing.serial.*;

// The serial port
Serial btPort;
String portName = "/dev/cu.Bluetooth-Incoming-Port";
int lastSendTime;

void setup() {

  // List all the available serial ports
  printArray(Serial.list());
  lastSendTime = millis();

  size(900, 540, P3D);
  btPort = new Serial(this, Serial.list()[1], 19200);
  
  // Wait for shits
  delay(2000);
}

void draw() {
  background(0);

  while (btPort.available() > 0) {
    int inByte = btPort.read();
    println(inByte);
  }

  if (millis() - lastSendTime > 1000) {
    println("Sending: 'A'");
    btPort.write(65);
    lastSendTime = millis();
  }
}


#include <Wire.h>
#include<SoftwareSerial.h>
#include "DHT.h"

#define DHTPIN 9
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

void receiveEvent(int howMany) {
  while (Wire.available() > 1) {
    char c = Wire.read();
    Serial.print(c);
  }
  int x = Wire.read();
  Serial.println(x);
}
void requestEvent() {
  Wire.write("1234567890");
  Wire.write("abcdefghlijklmnopqrstuvwxyz");
}

void setup() {
  // Start the I2C interface
  Wire.begin(4);
  Serial.begin(9600);
  dht.begin();
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop() {
  delay(1000);
}


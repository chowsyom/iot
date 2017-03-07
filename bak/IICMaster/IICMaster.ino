#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
}
byte x = 0;
void loop() {
  Wire.beginTransmission(4);
  Wire.write("x is ");
  Wire.write(x);
  Wire.endTransmission();
  x++;
  delay(1000);


  Wire.requestFrom(4,24);
  while (Wire.available() > 0) {
    char c = Wire.read();
    Serial.print(c);
  }
  //Serial.println("\r\n------------------");
}

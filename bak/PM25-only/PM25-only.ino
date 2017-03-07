#include<SoftwareSerial.h>

#define RXPIN 7
#define TXPIN 6

SoftwareSerial Serial1(RXPIN, TXPIN);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial1.listen();

}

void loop() {

  while (Serial1.available()) {
    delay(100);
    size_t len = Serial1.available();
    byte buf[len];
    Serial1.readBytes(buf, len);
    if(buf[0]==0x42&&buf[1]==0x4d)
    {
//      long pm10 = buf[10]*256+buf[11];
//      Serial.print("atmosphere, PM1.0=");
//      Serial.print(pm10);
      //Serial.println(" ug/m3");
      long pm25 = buf[12]*256+buf[13];
      Serial.print("atmosphere, PM2.5=");
      Serial.print(pm25);
      Serial.println(" ug/m3");
//      long pm100 = buf[14]*256+buf[15];
//      Serial.print("atmosphere, PM10=");
//      Serial.print(pm100);
//      Serial.println(" ug/m3");
    }
  }
  //while(Serial1.available()) Serial1.read();
  Serial.println();
  delay(5000);

}

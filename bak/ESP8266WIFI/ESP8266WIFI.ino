#include<SoftwareSerial.h>
#include "DHT.h"

#define SSId       "ES_001"                //type your own SSID name
#define PASSWORD   "go2map123"             //type your own WIFI password

#define SERVER      "192.168.0.108"
#define PORT       80

#define RXPIN 2
#define TXPIN 3

#define DebugBaudRate 115200
#define ESP8266BaudRate 115200

SoftwareSerial softSerial(RXPIN, TXPIN);

bool reset(void)
{
  softSerial.println("AT+GMR\r\n");
  softSerial.flush();
  unsigned long start;
  start = millis();
  String str = "";
  while (millis() - start < 10000 && softSerial.available() > 0) {
    Serial.write(softSerial.read());
    softSerial.flush();
    /*if (softSerial.find("OK") == true)
    {
      Serial.print("Module is ready\r\n");
      return true;
    }*/
  }
  //Serial.print("Module have no response\r\n");
  return false;
}


void setup()
{
  Serial.begin(DebugBaudRate);
  softSerial.begin(ESP8266BaudRate);
  softSerial.listen();

}


unsigned long time1 = -5000;
unsigned long time2 = -5000;
unsigned long time3 = -5000;
// The loop function is called in an endless loop
void loop()
{
  while (softSerial.available() > 0) {
    Serial.write(softSerial.read());
    softSerial.flush();
  }
  //reset();

  if (millis() - time1 > 5000) {
    softSerial.println("AT+CIPSTART=\"TCP\",\"192.168.0.108\",80\r\n");
    time1 = millis();
  }
  if (millis() - time2 > 7000) {
    softSerial.println("AT+CIPSEND=24\r\n");
    time2 = millis();
  }
  if (millis() - time3 > 9000) {
    softSerial.println("abcdefghijklmnopqrstuvwxyz\r\n");
    time3 = millis();
  }
 
  //softSerial.flush();

}




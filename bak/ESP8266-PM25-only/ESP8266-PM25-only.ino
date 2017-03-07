#include <ZESP8266.h>

#ifndef VW_MAX_MESSAGE_LEN
#define VW_MAX_MESSAGE_LEN 128
#endif
byte receiveBuf[VW_MAX_MESSAGE_LEN];
unsigned long send_StartTime = 0;


void wifis2serial()
{
  uint8_t len = VW_MAX_MESSAGE_LEN;
  ZWifi.receive(receiveBuf, &len);
}

void setup()
{
  Serial.begin(9600);
  ZWifi.begin();
}

void loop()
{
  ZWifi.loop();
  wifis2serial();
  if (Serial.available()) {
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    Serial.readBytes(buf, len);
    if(millis() - send_StartTime > 3000)
    {
      send_StartTime = millis();
      if(buf[0]==0x42&&buf[1]==0x4d)
      {
        long pm10 = buf[10]*256+buf[11];
        long pm25 = buf[12]*256+buf[13];
        long pm100 = buf[14]*256+buf[15];
        char info[50];
        sprintf(info,"PM1.0 = %d ug/m3, PM2.5 = %d ug/m3, PM10 = %d ug/m3\n",pm10,pm25,pm100);
        uint8_t sz = strlen(info);
        info[sz] = 0;
        ZWifi.send((byte*) info, sz, true);
        Serial.write(info, sz);
      }
    }
  }
}

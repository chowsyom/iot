#include <ZESP8266_UDP.h>
#include <ZUdpClock.h>

#define _BURN_FLASH_MODE_

#ifndef VW_MAX_MESSAGE_LEN
  #define VW_MAX_MESSAGE_LEN 128
#endif
byte receiveBuf[VW_MAX_MESSAGE_LEN];

#ifdef _BURN_FLASH_MODE_
    #define resetPIN 2
    WiFiServer server(8888);
    WiFiClient wifiClient;
    bool resetFlag = false;
    bool debugFlag = true;
#else
    unsigned long clock_update_inteval_1h = 3600000;
    unsigned long clock_update_inteval_5s = 5000;
    unsigned long update_clock_start_time = 0;
    unsigned long* clock_update_inteval;
    //FE 7C FF FF 01 03 FF 35 11 01 1A 0C 10 
    byte TIMING_CMD[15] = {0xFE,0x7C,0xFF,0xDB,0x01,0x03,0,0,0,0,0,0,0,0,0};
#endif

void serial2wifies()
{
  if (Serial.available())
  {
    delay(100);
    size_t len = Serial.available();
    byte sbuf[len+1];
    Serial.readBytes(sbuf, len);//serial缓冲区128字节，超出会丢失
    sbuf[len] = 0;
    ZWifi.send(sbuf, len+1, true);
  }
}
void wifis2serial()
{
  uint8_t len = VW_MAX_MESSAGE_LEN;
  ZWifi.receive(receiveBuf, &len);
  delay(100);
  if (len > 0)
  {
    #ifdef _BURN_FLASH_MODE_
      debugFlag = true;
    #endif
    Serial.write(receiveBuf, len);
    delay(10);
  }
}

void setup()
{
    Serial.begin(115200);
    ZWifi.begin();
    #ifdef _BURN_FLASH_MODE_
        pinMode(resetPIN,OUTPUT);
        digitalWrite(resetPIN, HIGH);
        server.begin();
        server.setNoDelay(true);
    #else
        clock_update_inteval = &clock_update_inteval_1h;
    #endif
}

void loop()
{
  ZWifi.loop();
  #ifdef _BURN_FLASH_MODE_
      
      if(!resetFlag) wifis2serial();
      if(debugFlag) serial2wifies();
      if (server.hasClient())
      {
          wifiClient = server.available();
          resetFlag = true;
          debugFlag = false;
      }
      while (wifiClient.available())//无线读取到的数据转发到到串口
      {
          uint8_t c = wifiClient.read();
          if(resetFlag)
          {
              resetFlag = false;
              digitalWrite(resetPIN, LOW);    // 将reset管脚的电平拉低50ms，起到复位的作用
              delay(50);
              digitalWrite(resetPIN, HIGH);
          }
          Serial.write(c);
      }
      while(Serial.available())
      {
          uint8_t b = (uint8_t) Serial.read();
          wifiClient.write(b);
      }
  #else
      if(update_clock_start_time == 0 || millis() - update_clock_start_time > *clock_update_inteval)
      {
        update_clock_start_time = millis();
        uint8_t date[7];
        if (ZClock.updateDateTime(date,true))
        {
          clock_update_inteval = &clock_update_inteval_1h;
          memcpy(TIMING_CMD+6,date,7);
          Serial.write(TIMING_CMD,15);
          delay(10);
        }
        else
        {
          clock_update_inteval = &clock_update_inteval_5s;
        }
      }
      wifis2serial();
      serial2wifies();
  #endif
  
}

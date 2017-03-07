#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ZESP8266.h>


#ifndef VW_MAX_MESSAGE_LEN
#define VW_MAX_MESSAGE_LEN 254
#endif

byte sendBuf[VW_MAX_MESSAGE_LEN];
byte receiveBuf[VW_MAX_MESSAGE_LEN];

void serial2wifies()
{
  if (Serial.available())
  {
    //size_t len = Serial.available();
    //byte sbuf[len];
    //Serial.readBytes(sbuf, len);//serial缓冲区128字节，超出会丢失
    uint8_t len = 0;
    while (Serial.available() && len < VW_MAX_MESSAGE_LEN)
    {
      sendBuf[len++] = Serial.read();
    }
    ZWifi.send(sendBuf, len, true);
    //delay(100);
    //ZWifi.print("Got ");
    //ZWifi.print(len,DEC);
    //ZWifi.println(" byte data from Serail and sended.");
    //delay(100);
  }
}
void wifis2serial()
{
  uint8_t len = VW_MAX_MESSAGE_LEN;
  ZWifi.receive(receiveBuf, &len);
  delay(100);
  if (len > 0)
  {
    Serial.write(receiveBuf, len);
  }
  delay(100);
}

void setup() {

  Serial.begin(115200);
  ZWifi.begin();
  
  // Port defaults to 8266
  ArduinoOTA.setPort(7777);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("Miko-5");
  // No authentication by default
  //ArduinoOTA.setPassword((const char *)"712");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  ZWifi.loop();
  wifis2serial();
  serial2wifies();
}

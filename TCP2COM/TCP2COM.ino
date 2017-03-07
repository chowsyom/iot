#include <ESP8266WiFi.h>

#define resetPIN 2

const char *ssid     = "ES_001";
const char *password = "go2map123";

WiFiServer server(8266);
WiFiClient wifiClient;

void setup()
{
    Serial.begin(115200);
    pinMode(resetPIN,OUTPUT);
    digitalWrite(resetPIN, HIGH);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.println("Connection Failed! Rebooting...");
      delay(3000);
      ESP.restart();
    }
    server.begin();
    server.setNoDelay(true);
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

bool resetFlag = false;

void loop()
{
   if (server.hasClient())
   {
        wifiClient = server.available();
        resetFlag = true;
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
//    if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
//    {
//        size_t len = Serial.available();
//        uint8_t sbuf[len];
//        Serial.readBytes(sbuf, len);
//        wifiClient.write(sbuf, len);
//    }
      while(Serial.available())
      {
        uint8_t b = (uint8_t) Serial.read();
        wifiClient.write(b);
      }
}

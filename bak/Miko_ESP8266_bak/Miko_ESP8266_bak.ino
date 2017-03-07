#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define _DEBUG_MODE_
#define _SERVER_MODE_
#define _HOME_MODE_
//#define _D1_MODE_

#ifdef _SERVER_MODE_
//#include <WiFiUdp.h>
#define reserve 0
#define MAX_WIFI_CLIENTS 5
#else
#define reserve 1
#define MAX_WIFI_CLIENTS 4
#endif



#ifdef _D1_MODE_
#define D0_PIN 0
#define D1_PIN 1
#define D2_PIN 2
#define D3_PIN 3
#define D4_PIN 4
#define D5_PIN 5
#define D12_PIN 12
#define D13_PIN 13
#define D14_PIN 14
#define D15_PIN 15
#define D16_PIN 16
#endif
uint8_t CLIENT_ID = 0xFF;
#ifndef _HOME_MODE_
const char *ssid     = "ES_001";
const char *password = "go2map123";
const char *host = "192.168.0.101";//Server IP
#else
const char *ssid     = "ZZY-2.4G";
const char *password = "Auglx19780712";
const char *host = "192.168.1.50";//Server IP
#endif

const int tcpPort = 8888;//端口号，随意修改，范围0-65535
//const int udpPort = 7777;//端口号，随意修改，范围0-65535
const unsigned long ALIVETIMEOUT = 300000;
const unsigned int HEARTBEATTIMEOUT = 60000;//int
WiFiServer server(tcpPort);
WiFiClient wifiClients[MAX_WIFI_CLIENTS + reserve];
WiFiClient debugClient;
//WiFiUDP udp;
unsigned long wifiClientStartConnectTime[MAX_WIFI_CLIENTS + reserve];
unsigned long heartBeatStartTime = 0;//no -
unsigned long serverStartConnectTime = 0;//no -

void configWifi(uint8_t id)
{
#ifndef _HOME_MODE_
  IPAddress IP(192, 168, 0, id);
  IPAddress GATEWAY(192, 168, 0, 1);
#else
  IPAddress IP(192, 168, 1, id);
  IPAddress GATEWAY(192, 168, 1, 1);
#endif
  IPAddress SUBNET(255, 255, 255, 0);
  WiFi.config(IP, GATEWAY, SUBNET);
}

void initWifi(uint8_t id)
{
  configWifi(id);
  //在这里检测是否成功连接到目标网络，未连接则阻塞。
  unsigned long startTime = 0 - 20001;
  while ( WiFi.status() != WL_CONNECTED) {
    // wait 10 seconds for connection:
    if (millis() - startTime > 20000)
    {
      startTime = millis();
#ifdef _DEBUG_MODE_
      Serial.print("Connect to WiFi: ");
      Serial.println(ssid);
#endif
      WiFi.begin(ssid, password);
    }
    delay(500);
  }
  //Serial.print("Spent times: ");
  //Serial.print(millis() - startTime);
  //Serial.println("ms");
  server.begin();
  server.setNoDelay(true);  //加上后才正常些
#ifdef _DEBUG_MODE_
  Serial.println("WiFi connected.");
#endif
}

void printIP()
{
#ifdef _DEBUG_MODE_
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}
void printMAC()
{
#ifdef _DEBUG_MODE_
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
#endif
}

void connectServer()
{
  if (!wifiClients[0] || !wifiClients[0].connected())
  {
    if (serverStartConnectTime = 0 || millis() - serverStartConnectTime > 5000)
    {
      serverStartConnectTime = millis();
      if (wifiClients[0]) wifiClients[0].stop();
      wifiClients[0].connect(host, tcpPort);
      wifiClientStartConnectTime[0] = millis();
#ifdef _DEBUG_MODE_
      if (debugClient && debugClient.connected())
      {
        debugClient.print("connect to ");
        debugClient.print(host);
        debugClient.print(":");
        debugClient.print(tcpPort);
        debugClient.println(" ...");
      }
#endif
    }
    delay(500);
  }
}

bool handleCmd(byte* input)
{
  bool flag = false;
  if (*input == 0xFE && *(input + 1) == 0x7C)
  {
    if (*(input + 2) == 0xFF && *(input + 3) == 0xFF)
    {
      //FE 7C 01 01 04 FC 01 05
      if (*(input + 5) == 0x01)
      {
        CLIENT_ID = *(input + 6);
        EEPROM.write(0, CLIENT_ID);
        EEPROM.commit();
        Serial.print("ID update to:");
        Serial.println(EEPROM.read(0), DEC);
      }
      else if (*(input + 5) == 0xFC)
      {
        printIP();
        printMAC();
      }
      flag = true;
    }
  }
  return flag;
}

//void sendUDPPacket()
//{
//  IPAddress addr(192, 168, 0, 255);
//  udp.beginPacket(addr, udpPort);
//  uint8_t msg[7] = {0xFE, 0x7C, 0xFF, CLIENT_ID, 0xFF, 0xFA, 0};
//  udp.write(msg, 7);
//  udp.endPacket();
//}


void showConnected(WiFiClient wificlient)
{
  uint8_t i;
  wificlient.print("local ip=");
  wificlient.println(WiFi.localIP());
  for (i = 0; i < MAX_WIFI_CLIENTS + reserve; i++) {
    if (wifiClients[i] && wifiClients[i].connected()) {
      wificlient.print("client[");
      wificlient.print(i);
      wificlient.print("]=");
      wificlient.print(wifiClients[i].remoteIP());
      wificlient.print(":");
      wificlient.println(wifiClients[i].remotePort());
    }
  }
}

bool wifi2serial(WiFiClient wificlient)
{
  bool flag = false;
  if (wificlient && wificlient.connected())
  {
    if (wificlient.available())
    {
      flag = true;
      delay(100);
      size_t rlen = wificlient.available();
      byte rbuf[rlen];
      byte *pr;
      pr = rbuf;
      while (wificlient.available()) {
        byte b = wificlient.read();
        *pr = b;
        pr++;
      }
      if (*rbuf == 0xFE && *(rbuf + 1) == 0x7C && *(rbuf + 2) == 0xFF && *(rbuf + 3) == 0xFF)
      {
        if (*(rbuf + 5) == 0xFC)
        {
          showConnected(wificlient);
          debugClient = wificlient;
        }
        else if (*(rbuf + 5) == 0xFB)
        {
          if (*(rbuf + 6) == 0xFF)
          {
            if (debugClient && debugClient.connected())
            {
              debugClient.println("Disconnected all");
            }
            uint8_t i;
            for (i = 0; i < MAX_WIFI_CLIENTS + reserve; i++) {
              if (wifiClients[i]) wifiClients[i].stop();
            }
          }
          else if (wifiClients[0]) {
            if (debugClient && debugClient.connected())
            {
              debugClient.println("Disconnected all");
            }
            wifiClients[0].stop();
          }

        }
        else if (*(rbuf + 5) == 0xFD)
        {
          if (wifiClients[0] && wifiClients[0].connected())
          {
            wifiClients[0].print("Hi!Message from ");
            wifiClients[0].println(WiFi.localIP());
            if (debugClient && debugClient.connected())
            {
              debugClient.print("Sended Message to ");
              debugClient.println(wifiClients[0].remoteIP());
            }
          }
        }
      }
      else Serial.write(rbuf, rlen);
      delay(100);
    }
  }
  return flag;
}
void serial2wifies()
{
  if (Serial.available())
  {
    delay(100);
    size_t len = Serial.available();
    byte sbuf[len];
    Serial.readBytes(sbuf, len);
    if (!handleCmd(sbuf)) {
      if (*sbuf != 0xFE || *(sbuf + 1) != 0x7C)
      {
        if (debugClient && debugClient.connected())
        {
          debugClient.write(sbuf, len);
        }
      }
      else
      {
        uint8_t i;
        for (i = 0; i < MAX_WIFI_CLIENTS + reserve; i++)
        {
          if (wifiClients[i] && wifiClients[i].connected())
          {
            wifiClients[i].write(sbuf, len);
          }
        }
      }
    }
  }
}
#ifdef _D1_MODE_
void showD1Pin(String pinStr)
{
  if (wifiClients[0] && wifiClients[0].connected())
  {
    wifiClients[0].println(pinStr);
  }
}
#endif

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(16);

#ifdef _D1_MODE_
  pinMode(D0_PIN, INPUT);
  pinMode(D1_PIN, INPUT);
  pinMode(D2_PIN, INPUT);
  pinMode(D3_PIN, INPUT);
  pinMode(D4_PIN, INPUT);
  pinMode(D5_PIN, INPUT);
  pinMode(D12_PIN, INPUT);
  pinMode(D13_PIN, INPUT);
  pinMode(D14_PIN, INPUT);
  pinMode(D15_PIN, INPUT);
  pinMode(D16_PIN, INPUT);
#endif

  delay(10);
  CLIENT_ID = EEPROM.read(0);
  delay(10);
  if (CLIENT_ID != 0xFF)
  {
    delay(1000);
    initWifi(CLIENT_ID);
    printIP();
    printMAC();
    delay(500);
  }
  else
  {
#ifdef _DEBUG_MODE_
    Serial.print("CLIENT_ID=");
    Serial.println(CLIENT_ID, DEC);
#endif
  }
  //  udp.begin(udpPort);
}

void loop()
{
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient())
  {
    for (i = reserve; i < MAX_WIFI_CLIENTS + reserve; i++) {
      //find free/disconnected spot
      if (!wifiClients[i] || !wifiClients[i].connected()) {
        if (wifiClients[i]) wifiClients[i].stop();
        wifiClients[i] = server.available();
        wifiClientStartConnectTime[i] = millis();
        // Serial1.print("New client: "); Serial1.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  for (i = reserve; i < MAX_WIFI_CLIENTS + reserve; i++) {
    if (millis() - wifiClientStartConnectTime[i] > ALIVETIMEOUT)
    {
      if (wifiClients[i]) wifiClients[i].stop();
    }
  }
  if (millis() - heartBeatStartTime > HEARTBEATTIMEOUT)
  {
    heartBeatStartTime = millis();
    for (i = 0; i < MAX_WIFI_CLIENTS + reserve; i++) {
      if (wifiClients[i] && wifiClients[i].connected()) {
        wifiClients[i].println("Heart Beat.");
        delay(100);
      }
    }
  }

#ifndef _SERVER_MODE_
  connectServer();
#endif
  for (i = 0; i < MAX_WIFI_CLIENTS + reserve; i++)
  {
    if (wifi2serial(wifiClients[i])) wifiClientStartConnectTime[i] = millis();
  }
  serial2wifies();

}

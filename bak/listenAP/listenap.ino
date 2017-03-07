#include "stainfo.h"
#include "cppstream.h"
#include "printer.h"
#include <ESP8266WiFi.h>

StationInfoManager manager;  //定义一个StaionInfo管理器


//连接或断开事件：设备接入时connected为true，设备断开时connected为false
void onStationChanged(const StationInfo& stainfo, bool connected)
{
  Serial << stainfo.ip << "(" << HexPrinter(Printer(stainfo.mac, 6), false, ":") << ") " << (connected ? "connected" : "disconnected") << endl;
  Serial << "Device list: " << endl;
  for (const StationInfo& sta : manager.list() )
  {
  	Serial << "    " << sta.ip << "(" << HexPrinter(Printer(sta.mac, 6), false, ":") << ") " << endl;
  }
  Serial << endl << endl;
}

void setup()
{
  Serial.begin(115200);
  Serial << endl;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("hello");
  manager.begin(onStationChanged);  //注册事件函数，通知设备连接和断开消息

}

void loop() 
{
  manager.loop(); //固定写法
}

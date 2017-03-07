#include <ZTiming.h>
#include <Messenger.h>
#include <ZOptByCmd.h>
#include <ZUdpClock.h>

#define RELAY1_PIN 4
#define RELAY2_PIN 5


unsigned long REPORT_INTERVAL = 60000; //毫秒
const uint8_t WORKING_LIMIT_HOUR  = 3;  //小时
const unsigned long SEC_3_HOUR = 10800000UL;

const char *ZTRUE = "true";
const char *ZFALSE = "false";

uint8_t rfMsg[10] = {0xFE, 0x7C, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t msg02Cb[5] = {0xFD,0x02,0,0,0};

bool isPowerOnByPm25 = false;
bool isLowSpeed = false;
bool isMiddleSpeed = false;
bool isHighSpeed = false;
bool isRelay_4_On = false;
bool fresh_air_state = false;

bool relay_1_state = false;
bool relay_2_state = false;

unsigned long report_startTime = 0;
unsigned long power_on_startTime = 0;
unsigned long fresh_air_startTime = 0;
float ht[2];
int pm[3];
char timestamp[22];

bool parsePMSensorData(float* ht, int* pm)
{
  bool flag = false;
  if (Serial.available())
  {
    uint8_t MAXLEN = 30;
    char buf[MAXLEN];
    size_t len = Serial.readBytesUntil(char(0x0A), buf, MAXLEN);
    while (Serial.available())
    {
      Serial.read();
    }
    if (buf[len - 1] == 0x0D)
    {
      buf[len - 1] = 0x20;
    }
    buf[len] = 0;
    if (len > 10 && buf[len - 1] == 0x20)
    {
      uint8_t j = 0;
      char* space;
      char* space_tmp;
      while (j < 5)
      {
        space_tmp = j == 0 ? buf : (space + 1);
        space = strchr(space_tmp, char(0x20));
        if (space != 0)
        {
          *space = 0;
          if (j < 2) ht[j] = atof(space_tmp);
          else pm[j - 2] = atoi(space_tmp);
        }
        j++;
      }
      flag = true;
    }
  }
  return flag;
}


void setup() {
  Serial.begin(9600);
  Messenger.begin();
  ZTiming.begin();
  //pinMode(D2_PIN, INPUT_PULLUP);
  //pinMode(D4_PIN, INPUT_PULLUP);
  //pinMode(D5_PIN, INPUT_PULLUP);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
}

void loop() {
  Messenger.loop();
  ZTiming.loop();
  uint8_t date[7];
  bool pm_state_changed = false;
  //接收PM2.5检测数据
  if(parsePMSensorData(ht, pm))
  {
//    Messenger.print("Temp=");
//    Messenger.print(String(ht[0]));
//    Messenger.print(" ,Humi=");
//    Messenger.print(String(ht[1]));
//    Messenger.print(" ,PM1.0=");
//    Messenger.print(String(pm[0]));
//    Messenger.print(" ,PM2.5=");
//    Messenger.print(String(pm[1]));
//    Messenger.print(" ,PM10=");
//    Messenger.println(String(pm[2]));
    //pm2.5高于60启动，低于10停止
    if(pm[1] > 60 && !isPowerOnByPm25)
    {
      isPowerOnByPm25 = true;
      pm_state_changed = true;
      power_on_startTime = millis();
    }
    if(pm[1]<25 && isPowerOnByPm25)
    {
      isPowerOnByPm25 = false;
      pm_state_changed = true;
    }
    //上报传感器数据
    if ((millis() - report_startTime > REPORT_INTERVAL) || (report_startTime == 0))
    {
      //Messenger.sendMsg(rfMsg, 8);
      report_startTime = millis();
    }
    ZClock.getDateTime(date, timestamp);
  }
  Messenger.debugInfo("update time", timestamp, 1);
  Messenger.debugInfo("Temp.", String(ht[0]), 2);
  Messenger.debugInfo("Humi.", String(ht[1]), 3);
  Messenger.debugInfo("Pm1.0", String(pm[0]), 4);
  Messenger.debugInfo("Pm2.5", String(pm[1]), 5);
  Messenger.debugInfo("Pm10", String(pm[2]), 6);
  //限制运行最长3个小时
  if(isPowerOnByPm25 && (millis() - power_on_startTime > SEC_3_HOUR))
  {
    isPowerOnByPm25 = false;
    pm_state_changed = true;
  }
  Messenger.debugInfo("On By Pm2.5", isPowerOnByPm25 ? ZTRUE : ZFALSE, 7);

  //侦听请求应答
  if(Messenger.listen(msg02Cb))
  {
    if(msg02Cb[1]==0x02)
    {
      fresh_air_state = msg02Cb[2] & 0x01;
      Messenger.print("fresh_air_state=");
      Messenger.println(fresh_air_state?"on":"off");
    }
  }
  //发送指令给新风机
  if((fresh_air_state!=isPowerOnByPm25) && (millis() - fresh_air_startTime > 10000))
  {
    rfMsg[1] = 0x7F;
    rfMsg[2] = 0x35;
    rfMsg[5] = 0x02;
    rfMsg[6] = 0x03;
    rfMsg[7] = isPowerOnByPm25 ? 0x01 : 0xFF;
    rfMsg[8] = 0;
    Messenger.sendMsg(rfMsg, 9);
    fresh_air_startTime = millis();
  }
  //  boolean isPowerOn[3] = {false, false, false};
  //  OptByCmd* optByCmd;
  //  uint8_t i;
  //  uint8_t date[7];
  //  updateDateTime(date);
  //  int curTime = date[2] * 60 + date[1];
  //  for (i = 0; i < 3; i++)
  //  {
  //    optByCmd = Messenger.getOptByCmdFlag(i);
  //    isPowerOn[i] = (*optByCmd).enable && (*optByCmd).isOn;
  //    if (isPowerOn[i])
  //    {
  //      int cmdTime = (*optByCmd).time[0] * 60 + (*optByCmd).time[1];
  //      int limitTime = cmdTime + WORKING_LIMIT_HOUR * 60;
  //      isPowerOn[i] = (cmdTime <= curTime && curTime < limitTime) || (cmdTime <= (curTime + 1440) && (curTime + 1440) < limitTime);
  //    }
  //  }

  boolean isPowerOnByTimingOrCmd = false;
  OptByCmd* optByCmd = Messenger.getOptByCmdFlag(0);
  uint8_t i;
  if (ZClock.updateDateTime(date))
  {
    isPowerOnByTimingOrCmd =  ZTiming.checkTimingList(0, 8, optByCmd, date);
    Messenger.debugInfo("On By Timing Or cmd", isPowerOnByTimingOrCmd ? ZTRUE : ZFALSE, 8);
    //非定时期间开启不超过3小时
    if (isPowerOnByTimingOrCmd && (*optByCmd).enable && (*optByCmd).isOn )
    {
      int curTime = date[2] * 60 + date[1];
      int cmdTime = (*optByCmd).time[0] * 60 + (*optByCmd).time[1];
      int limitTime = cmdTime + WORKING_LIMIT_HOUR * 60;
      isPowerOnByTimingOrCmd = (cmdTime <= curTime && curTime < limitTime) || (cmdTime <= (curTime + 1440) && (curTime + 1440) < limitTime);//+1440 处理跨天的情况
      if(!isPowerOnByTimingOrCmd) (*optByCmd).enable = false;
    }
  }
  else 
  {
    //当时钟失效时
    if(isPowerOnByTimingOrCmd != (*optByCmd).enable && (*optByCmd).isOn)
    {
      isPowerOnByTimingOrCmd = (*optByCmd).enable && (*optByCmd).isOn;
      if(isPowerOnByTimingOrCmd) power_on_startTime = millis();
    }
    if(millis() - power_on_startTime > SEC_3_HOUR) //3 hours auto turn off
    {
      isPowerOnByTimingOrCmd = false;
    }
  }

  //0x01 = 0001  power on ,default Middle
  //0x03 = 0011  Low
  //0x07 = 0111  Middle
  //0x0F = 1111  High
  bool speedState[3] = {Messenger.checkRemoteCommand(0x02), Messenger.checkRemoteCommand(0x04), Messenger.checkRemoteCommand(0x08)};


  //更新继电器状态
  //高中低速互斥
  isLowSpeed = !speedState[2] && !speedState[1] && speedState[0];
  isMiddleSpeed = !speedState[2] && speedState[1];
  //isHighSpeed = speedState[2];

  //C -  O
  //     |
  //  C  -  O
  //  |     |
  //  L   C - O
  //      M   H
  //L 100
  //M 110
  //H 111
  //继电器1
  if ( relay_1_state != isLowSpeed) {
    relay_1_state = isLowSpeed;
    digitalWrite(RELAY1_PIN, !relay_1_state);
  }
  //继电器2
  if ( relay_2_state != (isPowerOnByTimingOrCmd || isPowerOnByPm25)) {
    relay_2_state = isPowerOnByTimingOrCmd || isPowerOnByPm25;
    digitalWrite(RELAY2_PIN, !relay_2_state);
  }
  
  Messenger.debugInfo("Power On", relay_1_state ? ZTRUE : ZFALSE, 9);
  Messenger.debugInfo("Fan Speed", isHighSpeed ? "High" : (isLowSpeed ? "LOW" : "Middle"), 10);
  //更新继电器状态
  uint8_t relayState = 0;
  relayState |= relay_1_state ? 0x01 : 0;
  relayState |= !relay_2_state ? 0x02 : 0;
  Messenger.setRelayState(relayState);
  //设置空闲状态
  Messenger.setIdle(!relay_1_state && !relay_2_state);

}

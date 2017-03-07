#include <ZTiming.h>
#include <Messenger.h>
#include <ZOptByCmd.h>
#include <ZUdpClock.h>

#define D2_PIN 2
#define D4_PIN 4
#define D5_PIN 5

#define RELAY1_PIN 12
#define RELAY2_PIN 13
#define RELAY3_PIN 14

unsigned long REPORT_INTERVAL_10min = 600000; //毫秒
unsigned long REPORT_INTERVAL_1min = 60000; //毫秒
const uint8_t WORKING_LIMIT_HOUR  = 3;  //小时

const char *ZTRUE = "true";
const char *ZFALSE = "false";

unsigned long *p_REPORT_INTERVAL = &REPORT_INTERVAL_10min;

bool isWaterLow = false;
bool isWaterMiddle = false;
bool isWaterHigh = false;

bool isLowSpeed = false;
bool isMiddleSpeed = false;
bool isHighSpeed = false;
bool isRelay_4_On = false;

bool relay_1_state = false;
bool relay_2_state = false;
bool relay_3_state = false;
bool relay_4_state = false;

unsigned long report_startTime = 0;
unsigned long fresh_air_startTime = 0;
uint8_t rfMsg[8] = {0xFE, 0x7C, 0x32, 0, 1, 0x12, 0, 0};

void setup() {

  Messenger.begin();
  ZTiming.begin();
  pinMode(D2_PIN, INPUT_PULLUP);
  pinMode(D4_PIN, INPUT_PULLUP);
  pinMode(D5_PIN, INPUT_PULLUP);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  //pinMode(RELAY4_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  //digitalWrite(RELAY4_PIN, HIGH);
}

//String timeStr = "11:10-12:20,13:30-14:40,15:60-16:00,17:01-18:32";

void loop() {
  Messenger.loop();
  ZTiming.loop();

  bool val_D2;
  bool val_D4;
  bool val_D5;
  val_D2 = !digitalRead(D2_PIN);
  if (isWaterLow != val_D2)
  {
    delay(100);
    val_D2 = !digitalRead(D2_PIN);
    if (isWaterLow != val_D2)
    {
      isWaterLow = val_D2;
    }
  }
  val_D4 = !digitalRead(D4_PIN);
  if (isWaterMiddle != val_D4)
  {
    delay(100);
    val_D4 = !digitalRead(D4_PIN);
    if (isWaterMiddle != val_D4)
    {
      isWaterMiddle = val_D4;
    }
  }
  val_D5 = !digitalRead(D5_PIN);
  if (isWaterHigh != val_D5)
  {
    delay(100);
    val_D5 = !digitalRead(D5_PIN);
    if (isWaterHigh != val_D5)
    {
      isWaterHigh = val_D5;
    }
  }
  Messenger.debugInfo("REPORT INTERVAL", String(*p_REPORT_INTERVAL), 1);
  if ((millis() - report_startTime > *p_REPORT_INTERVAL) || report_startTime == 0)
  {
    report_startTime = millis();
    uint8_t state = 0;
    if (isWaterLow) state |= 0x01;
    if (isWaterMiddle) state |= 0x02;
    if (isWaterHigh) state |= 0x04;
    if (state == 0x01) p_REPORT_INTERVAL = &REPORT_INTERVAL_1min;
    else p_REPORT_INTERVAL = &REPORT_INTERVAL_10min;
    rfMsg[5] = 0x12;
    rfMsg[6] = state == 0 ? 0xFF : state;
    Messenger.sendMsg(rfMsg, 8);
  }


  Messenger.debugInfo("Recycle Water Low", isWaterLow ? ZTRUE : ZFALSE, 2);
  Messenger.debugInfo("Recycle Water Middle", isWaterMiddle ? ZTRUE : ZFALSE, 3);
  Messenger.debugInfo("Recycle Water High", isWaterHigh ? ZTRUE : ZFALSE, 4);

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

  boolean isPowerOn = false;
  OptByCmd* optByCmd = Messenger.getOptByCmdFlag(0);
  uint8_t i;
  uint8_t date[7];
  if (ZClock.updateDateTime(date))
  {
    isPowerOn =  ZTiming.checkTimingList(0, 8, optByCmd, date);
    Messenger.debugInfo("On By Timing", isPowerOn ? ZTRUE : ZFALSE, 5);
    //非定时期间开启不超过3小时
    if (isPowerOn && (*optByCmd).enable && (*optByCmd).isOn )
    {
      int curTime = date[2] * 60 + date[1];
      int cmdTime = (*optByCmd).time[0] * 60 + (*optByCmd).time[1];
      int limitTime = cmdTime + WORKING_LIMIT_HOUR * 60;
      isPowerOn = (cmdTime <= curTime && curTime < limitTime) || (cmdTime <= (curTime + 1440) && (curTime + 1440) < limitTime);//+1440 处理跨天的情况
      if(!isPowerOn) (*optByCmd).enable = false;
    }
  }
  else 
  {
    //当时钟失效时
    if(isPowerOn != (*optByCmd).enable && (*optByCmd).isOn)
    {
      isPowerOn = (*optByCmd).enable && (*optByCmd).isOn;
      if(isPowerOn) fresh_air_startTime = millis();
    }
    if(millis() - fresh_air_startTime > 10800000UL) //3 hours auto turn off
    {
      isPowerOn = false;
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
  isHighSpeed = speedState[2];

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
  if ( relay_1_state != isPowerOn) {
    relay_1_state = isPowerOn;
    digitalWrite(RELAY1_PIN, !relay_1_state);
  }
  //继电器2
  if ( relay_2_state != isLowSpeed) {
    relay_2_state = isLowSpeed;
    digitalWrite(RELAY2_PIN, relay_2_state);
  }
  //继电器3
  if ( relay_3_state !=  isHighSpeed) {
    relay_3_state = isHighSpeed;
    digitalWrite(RELAY3_PIN, !relay_3_state);
  }
  Messenger.debugInfo("Fresh Air Power On", isPowerOn ? ZTRUE : ZFALSE, 6);
  Messenger.debugInfo("Fan Speed", isHighSpeed ? "High" : (isLowSpeed ? "LOW" : "Middle"), 7);
  //更新继电器状态
  uint8_t relayState = 0;
  relayState |= relay_1_state ? 0x01 : 0;
  relayState |= !relay_2_state ? 0x02 : 0;
  relayState |= relay_3_state ? 0x04 : 0;
  Messenger.setRelayState(relayState);
  //response
  uint8_t sbuf[2];
  sbuf[0] = 0x02;
  sbuf[1] = relayState;
  Messenger.response(sbuf, 2);
  //设置空闲状态
  Messenger.setIdle(!relay_1_state && !relay_2_state && !relay_3_state);

}

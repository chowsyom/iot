#include <ZTiming.h>
#include <Messenger.h>
#include <ZHCRS501.h>

#define _1ST_TOILET_  //选择主次卫

#define MANUAL_PIN 2
#define HCSR501_PIN 3
#define INFRARED_SWITCH_PIN 4


#define NIGHT_LIGHT_PIN 5
#ifdef _1ST_TOILET_
  #define RELAY_0_PIN 6
  #define RELAY_1_PIN 10
  #define RELAY_2_PIN 9
  #define RELAY_3_PIN 8
  #define RELAY_4_PIN 7
#else
  #define RELAY_1_PIN 7
  #define RELAY_2_PIN 8
  #define RELAY_3_PIN 9
  #define RELAY_4_PIN 10
#endif
  #define RELAY_5_PIN 11
#ifndef _1ST_TOILET_
  #define RELAY_6_PIN 12
#endif
#ifdef _1ST_TOILET_
//牛眼射灯
bool isRelay_0_On = false;
#endif
bool isRelay_1_On = false;
bool isRelay_2_On = false;
bool isRelay_3_On = false;
bool isRelay_4_On = false;
bool isRelay_5_On = false;
#ifndef _1ST_TOILET_
bool isRelay_6_On = false;
#endif
bool isNightLight_On = false;

bool isToilet_using = false;
bool door_state = false;
bool manual_state = false;
bool body_active = false;
bool water_heater_switch_to =false;
bool manual_enable = false;

#ifdef _1ST_TOILET_
bool water_heater_state = false;
unsigned long msg_sended_time = 0;
#endif

bool isRelay_1_EnablePeriod = false;
bool isRelay_2_EnablePeriod = false;
bool isRelay_2_OnByTimer = false;
bool isRelay_4_OnByTimer = false;

const char *ZTRUE = "1";
const char *ZFALSE = "0";

const unsigned long TIMEOUT_10_SEC = 10000;   //毫秒
const unsigned long TIMEOUT_20_SEC = 20000;   //毫秒
const unsigned long TIMEOUT_30_SEC = 30000;   //毫秒
const unsigned long TIMEOUT_60_SEC = 60000;   //毫秒
const unsigned long TIMEOUT_120_SEC = 120000;   //毫秒
const unsigned long TIMEOUT_10_MIN = 600000;   //毫秒
const unsigned long TIMEOUT_30_MIN = 1800000;   //毫秒

unsigned long body_active_startTime = 0;
unsigned long manual_changed_startTime = 0;
unsigned long off_2_startTime = 0;
unsigned long close_door_startTime = 0;
unsigned long msg11_startTime = 0;
uint8_t rfMsg[10] = {0xFE, 0x7C, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t msg02Cb[5] = {0xFD,0x02,0,0,0};
uint8_t msg11Cb[5] = {0xFD,0x11,0,0,0};

void fadeIn()
{
  uint8_t i = 0;
  while(i<255)
  {
    analogWrite(NIGHT_LIGHT_PIN, i);
    i+=5;
    delay(100);
  }
  digitalWrite(NIGHT_LIGHT_PIN, HIGH);
}
void fadeOut()
{
  uint8_t i = 255;
  while(i>0)
  {
    analogWrite(NIGHT_LIGHT_PIN, i);
    i-=5;
    delay(100);
  }
  digitalWrite(NIGHT_LIGHT_PIN, LOW);
}

void setup() {
  Serial.begin(115200);
  Messenger.begin();
  ZTiming.begin();

#ifdef _1ST_TOILET_
  pinMode(RELAY_0_PIN, OUTPUT);
#endif
  pinMode(MANUAL_PIN, INPUT_PULLUP);
  pinMode(HCSR501_PIN, INPUT);
  pinMode(INFRARED_SWITCH_PIN, INPUT_PULLUP);
  
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);
  pinMode(RELAY_5_PIN, OUTPUT);
  #ifndef _1ST_TOILET_
  pinMode(RELAY_6_PIN, OUTPUT);
  #endif
  pinMode(NIGHT_LIGHT_PIN, OUTPUT);

  

#ifdef _1ST_TOILET_
  digitalWrite(RELAY_0_PIN, LOW);
#endif
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(RELAY_3_PIN, HIGH);
  digitalWrite(RELAY_4_PIN, HIGH);
  digitalWrite(RELAY_5_PIN, HIGH);
  #ifndef _1ST_TOILET_
  digitalWrite(RELAY_6_PIN, HIGH);
  #endif
  digitalWrite(NIGHT_LIGHT_PIN, LOW);
  //analogWrite(NIGHT_LIGHT_PIN, 0);
  manual_state = !digitalRead(MANUAL_PIN);
}

void loop() {

  bool manual_state_changed = false;
  bool hcrs_state_changed = false;
  bool door_open_flag = false;
  bool isContinueActive = false;

  Messenger.loop();
  ZTiming.loop();

  uint8_t date[7];
  if(ZClock.updateDateTime(date))
  {
    isRelay_1_EnablePeriod = ZTiming.checkTimingList(0, 1, date);
    isRelay_2_EnablePeriod =  ZTiming.checkTimingList(1, 2, date);
    isRelay_2_OnByTimer =  ZTiming.checkTimingList(2, 4, Messenger.getOptByCmdFlag(1), date);
    isRelay_4_OnByTimer =  ZTiming.checkTimingList(4, 8, Messenger.getOptByCmdFlag(3), date);
  }
  #ifdef _1ST_TOILET_
    //牛眼灯状态判断条件
    bool relay_0_switch_to = isRelay_0_On;
  #endif
  //主灯状态判断条件
  bool relay_1_switch_to = isRelay_1_On;
  //led 夜灯
  bool condition1_1 = isNightLight_On;
  //排风扇
  bool relay_2_switch_to = isRelay_2_On;
  //马桶盖
  bool relay_3_switch_to = isRelay_3_On;
  
  //读取手动开关状态
  bool isManual = !digitalRead(MANUAL_PIN);
  Messenger.debugInfo("SW" ,isManual?ZTRUE:ZFALSE);
  if (manual_state != isManual)
  {
    delay(100);
    isManual = !digitalRead(MANUAL_PIN);
    if (manual_state != isManual)
    {
      manual_state = isManual;
      manual_state_changed = true;
      manual_changed_startTime = millis();
      Messenger.print("Switch is ");
      Messenger.println(manual_state?"on":"off");
    }
  }
  //红外镜面反射光电开关infraredSwitch
  bool isDoorClose = !digitalRead(INFRARED_SWITCH_PIN);
  if (door_state != isDoorClose)
  {
    delay(100);
    isDoorClose = !digitalRead(INFRARED_SWITCH_PIN);
    if (door_state != isDoorClose)
    {
      door_state = isDoorClose;
      if (!door_state) door_open_flag = true;
      else close_door_startTime = millis();
      Messenger.print("Door is ");
      Messenger.println(door_state?"close":"open");
    }
  }
  //分门开着和关闭2种情况
  //1.门关着，则亮灯；手动开关只控制牛眼灯和热水器切换。
  //2.门开着，则由人体感应控制灯；手动开关可控制 主灯、牛眼灯和热水器切换。
  if(door_state)
  {
    //门关着时
    relay_1_switch_to = true;
    relay_2_switch_to = true;
    relay_3_switch_to = true;
    //撤销开着门时手动控制的状态
    manual_enable = false;
    //撤销排风扇关闭计时
    off_2_startTime = 0;
    //手动控制
    if(manual_state_changed)
    {
      //手动开关灯时切换燃气热水器
      water_heater_switch_to = !water_heater_switch_to;
      #ifdef _1ST_TOILET_
        //开关牛眼灯
        relay_0_switch_to = water_heater_switch_to;
      #endif
    }
  }
  else
  {
    //门开着时
    //手动控制
    if(manual_state_changed)
    {
      manual_enable = true;
      //主灯灭，则只开主灯
      if(!isRelay_1_On)
      {
        relay_1_switch_to = true;
      }
      else
      {
        //主灯亮，则先开关牛眼灯和热水器，否则全部关掉
        //切换燃气热水器
        water_heater_switch_to = !water_heater_switch_to;
        #ifdef _1ST_TOILET_
          //开关牛眼灯
          relay_0_switch_to = water_heater_switch_to;
        #endif
        relay_1_switch_to = !isRelay_1_On || water_heater_switch_to;
      }
    }
    else if(manual_enable)
    {
      //维持手动控制后的状态直到超时
      //relay_1_switch_to = isRelay_1_On;
      //避免手动控制影响
      relay_3_switch_to = true;
      const unsigned long* timeout_m = relay_1_switch_to ? &TIMEOUT_120_SEC : &TIMEOUT_10_SEC;
      if(millis() - manual_changed_startTime > *timeout_m)
      {
        manual_enable = false;
      }
    }
    //人体感应控制
    if(!manual_enable)
    {
      //读取HC-RS501释热传感器状态
      bool isHcrs501Fired = hcrs501_isFired(1, HCSR501_PIN);
      isContinueActive = hcrs501_continue_active(0, isHcrs501Fired);
      if (body_active != isHcrs501Fired)
      {
        delay(100);
        isHcrs501Fired = hcrs501_isFired(1, HCSR501_PIN);
        if (body_active != isHcrs501Fired)
        {
          body_active = isHcrs501Fired;
          hcrs_state_changed = true;
          Messenger.print("Hcrs501 is ");
          Messenger.println(body_active?"on":"off");
        }
      }
      //主灯
      #ifdef _1ST_TOILET_
        const unsigned long* timeout_1 = isContinueActive ? (door_open_flag ? &TIMEOUT_20_SEC : &TIMEOUT_30_SEC) : &TIMEOUT_10_SEC;
      #else
        const unsigned long* timeout_1 = isContinueActive ? (door_open_flag ? &TIMEOUT_20_SEC : &TIMEOUT_60_SEC) : &TIMEOUT_30_SEC;
      #endif
      //排风扇
      const unsigned long* timeout_2 = isRelay_2_EnablePeriod ? &TIMEOUT_120_SEC : &TIMEOUT_30_SEC;
      //重置计时
      if(body_active) body_active_startTime = millis();
      //主灯被人体感应触发后在timeout_1时间内持续有效
      bool relay_1_hold_on = (body_active_startTime > 1000) && (millis() - body_active_startTime  < *timeout_1);
      //排风扇被人体感应触发后在timeout_2时间内持续有效
      bool relay_2_hold_on = (body_active_startTime > 1000) && (millis() - body_active_startTime  < *timeout_2);
      //马桶盖
      bool relay_3_hold_on = (body_active_startTime > 1000) && (millis() - body_active_startTime  < TIMEOUT_30_MIN);
      //主灯触发条件
      relay_1_switch_to = isRelay_1_EnablePeriod && relay_1_hold_on;
      //led 夜灯
      condition1_1 = !isRelay_1_EnablePeriod && relay_1_hold_on;
      //排风扇 不关门情况下，人体感器触发时，10分钟内最多运行1次
      relay_2_switch_to = isRelay_2_EnablePeriod && relay_2_hold_on && ((off_2_startTime == 0) || (millis() - off_2_startTime > TIMEOUT_10_MIN));
      //马桶盖
      relay_3_switch_to = relay_3_hold_on;
    }
  }

  Messenger.debugInfo("DoorX", door_state?ZTRUE:ZFALSE);
  Messenger.debugInfo("R1 E",isRelay_1_EnablePeriod?ZTRUE:ZFALSE);
  Messenger.debugInfo("R2 E",isRelay_2_EnablePeriod?ZTRUE:ZFALSE);
  Messenger.debugInfo("R2 O", isRelay_2_OnByTimer?ZTRUE:ZFALSE);
  Messenger.debugInfo("R4 O", isRelay_4_OnByTimer?ZTRUE:ZFALSE);



  //更新继电器状态
  //继电器1
  if (isRelay_1_On != (relay_1_switch_to || Messenger.checkRemoteCommand(0x01)))
  {
    isRelay_1_On = (relay_1_switch_to || Messenger.checkRemoteCommand(0x01));
    Messenger.print("Relay No.1 is ");
    Messenger.println(isRelay_1_On?"on":"off");
    //设置继电器
    digitalWrite(RELAY_1_PIN, !isRelay_1_On);
  }
  //如果主灯亮，则关夜灯
  if(isRelay_1_On)
  {
    condition1_1 = false;
  }
  //led夜灯
  if (isNightLight_On != condition1_1)
  {
    isNightLight_On = condition1_1;
    if(isNightLight_On) fadeIn();
    else fadeOut();
  }
  //牛眼灯
  #ifdef _1ST_TOILET_
    if(isRelay_0_On != relay_0_switch_to)
    {
      isRelay_0_On = relay_0_switch_to;
      Messenger.print("Relay No.0 is ");
      Messenger.println(isRelay_0_On?"on":"off");
      digitalWrite(RELAY_0_PIN, isRelay_0_On);
    }
  #endif
  
  //继电器2
  if (isRelay_2_On != (relay_2_switch_to || isRelay_2_OnByTimer || Messenger.checkRemoteCommand(0x02)))
  {
    isRelay_2_On = relay_2_switch_to || isRelay_2_OnByTimer || Messenger.checkRemoteCommand(0x02);
    if(!isRelay_2_On) off_2_startTime = millis();
    Messenger.print("Relay No.2 is ");
    Messenger.println(isRelay_2_On?"on":"off");
    digitalWrite(RELAY_2_PIN, !isRelay_2_On);
  }

  //继电器3
  if (isRelay_3_On != (relay_3_switch_to || Messenger.checkRemoteCommand(0x04)))
  {
      isRelay_3_On = (relay_3_switch_to || Messenger.checkRemoteCommand(0x04));
      digitalWrite(RELAY_3_PIN, !isRelay_3_On);
  }
  
  //继电器4
  if (isRelay_4_On != isRelay_4_OnByTimer) {
    isRelay_4_On = isRelay_4_OnByTimer;
    Messenger.print("Relay No.4 is ");
    Messenger.println(isRelay_4_On?"on":"off");
    digitalWrite(RELAY_4_PIN, !isRelay_4_On);
  }
 
  //继电器5
   #ifndef _1ST_TOILET_
    if (isRelay_5_On != Messenger.checkRemoteCommand(0x10)) 
    {
      isRelay_5_On = Messenger.checkRemoteCommand(0x10);
      Messenger.print("Relay No.5 is ");
      Messenger.println(isRelay_5_On?"on":"off");
      digitalWrite(RELAY_5_PIN, !isRelay_5_On);
    }
    //向主卫燃气指示灯发应答指令{0xFE, 0x7C, 0x33, 0x34, 0x01, 0xFD, 0x02, 0x10, 0x01, 0};
    uint8_t sbuf[3];
    sbuf[0] = 0x02;
    sbuf[1] = 0x10;
    sbuf[2] = isRelay_5_On ? 0x01 : 0xFF;
    Messenger.response(sbuf, 3);
  #else
    //侦听请求应答
    if(Messenger.listen(msg02Cb))
    {
      if(msg02Cb[1]==0x02)
      {
        if(msg02Cb[2]==0x10)
        {
          water_heater_state = msg02Cb[3]==0x01;
           Messenger.print("water_heater_state=");
           Messenger.println(water_heater_state?"on":"off");
        }
      }
    }
    if((msg_sended_time == 0) || (water_heater_switch_to != water_heater_state) && (millis() - msg_sended_time > 10000))
    {
        //发送热水器切换指令
        msg_sended_time = millis();
        rfMsg[1] = 0x7F;//自定义回复
        rfMsg[2] = 0x34;
        rfMsg[5] = 0x02;
        rfMsg[6] = 0x10;
        rfMsg[7] = water_heater_switch_to ? 0x01 : 0xFF;
        rfMsg[8] = 0;
        Messenger.sendMsg(rfMsg, 9);
    }
    if (isRelay_5_On != water_heater_state) 
    {
      isRelay_5_On = water_heater_state;
      Messenger.print("Relay No.5 is ");
      Messenger.println(isRelay_5_On?"on":"off");
      digitalWrite(RELAY_5_PIN, !isRelay_5_On);
    }
  #endif

   #ifndef _1ST_TOILET_
  //继电器6
  if (isRelay_6_On != (Messenger.checkRemoteCommand(0x20) || water_heater_switch_to)) {
    isRelay_6_On = Messenger.checkRemoteCommand(0x20) || water_heater_switch_to;
    Messenger.print("Relay No.6 is ");
    Messenger.println(isRelay_6_On?"on":"off");
    digitalWrite(RELAY_6_PIN, !isRelay_6_On);
  }
  #endif

  //侦听请求应答
//  if(Messenger.listen(msg11Cb))
//  {
//    if(msg11Cb[1]==0x11)
//    {
//      #ifdef _1ST_TOILET_
//        isToilet_using = msg11Cb[2] & 0x01;
//      #else
//        isToilet_using = msg11Cb[2] & 0x10;
//      #endif
//      Messenger.print("Toilet is ");
//      Messenger.println(isToilet_using?"using.":"not used.");
//    }
//  }
//  Messenger.debugInfo("USING", isToilet_using?ZTRUE:ZFALSE);
//  if ((isToilet_using != door_state) && (millis() - msg11_startTime > 10000))
//  {
//    msg11_startTime = millis();
//#ifdef _1ST_TOILET_
//    uint8_t toiletId = 0x02;
//    uint8_t toiletUsed = 0x01;
//#else
//    uint8_t toiletId = 0x20;
//    uint8_t toiletUsed = 0x10;
//#endif
//    rfMsg[1] = 0x7F;
//    rfMsg[2] = 0x32;
//    rfMsg[5] = 0x11;
//    rfMsg[6] = door_state ? toiletUsed : 0;//状态
//    rfMsg[6] = rfMsg[6] | toiletId;//加卫生间id
//    rfMsg[7] = 0;
//    Messenger.sendMsg(rfMsg, 8);
//  }

  //更新继电器状态
  uint8_t relayState = 0;
  relayState |= isRelay_1_On ? 0x01 : 0;
  relayState |= isRelay_2_On ? 0x02 : 0;
  relayState |= isRelay_3_On ? 0x04 : 0;
  relayState |= isRelay_4_On ? 0x08 : 0;
  relayState |= isRelay_5_On ? 0x10 : 0;
  #ifndef _1ST_TOILET_
  relayState |= isRelay_6_On ? 0x20 : 0;
  #endif
  Messenger.setRelayState(relayState);
  //设置空闲状态
  //Messenger.setIdle(!isRelay_1_On && !isRelay_2_On && !isRelay_3_On && !isRelay_4_On && !isNightLight_On);
}

#include <ZTiming.h>
#include <Messenger.h>
#include <ZHCRS501.h>
#include <ZSD3231.h>

#define HCSR501_1_PIN 2
#define HCSR501_2_PIN 4
#define PHOTO_RESISTOR_PIN 3
//4 5 短路，5废弃
#define RELAY1_PIN 6
#define RELAY2_PIN 7
#define RELAY3_PIN 8
#define RELAY4_PIN 11

#define MANUAL_PIN 9
#define NIGHT_LIGHT_PIN 10

unsigned long TIMEOUT_10_SEC = 10000;   //毫秒
unsigned long TIMEOUT_30_SEC = 30000;   //毫秒
unsigned long TIMEOUT_60_SEC = 60000;   //毫秒
unsigned long TIMEOUT_2_MIN = 120000;
unsigned long TIMEOUT_5_MIN = 300000;
//unsigned long TIMEOUT_10_MIN = 600000;

bool body_1_active = false;
bool body_2_active = false;
bool manual_state = false;
bool bright_state = false;
bool relay_2_on_manual = false;
bool relay_3_on_manual = false;

const char *ZTRUE = "true";
const char *ZFALSE = "false";



bool relay_1_state = false;
bool relay_2_state = false;
bool relay_3_state = false;
bool relay_4_state = false;
bool night_light_state = false;

unsigned long relay_1_startTime = 0;
unsigned long relay_2_startTime = 0;
unsigned long relay_3_startTime = 0;
unsigned long night_light_startTime = 0;
unsigned long* p_RELAY_1_DELAY;
unsigned long* p_RELAY_3_DELAY;

bool isRelay_1_EnablePeriod = false;
bool isRelay_3_EnablePeriod = false;

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
  Messenger.begin();
  ZTiming.begin();
  pinMode(HCSR501_1_PIN, INPUT);
  pinMode(HCSR501_2_PIN, INPUT);
  pinMode(MANUAL_PIN, INPUT_PULLUP);
  pinMode(PHOTO_RESISTOR_PIN, INPUT);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  pinMode(NIGHT_LIGHT_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);
  digitalWrite(NIGHT_LIGHT_PIN, LOW);

  p_RELAY_1_DELAY = &TIMEOUT_10_SEC;
  p_RELAY_3_DELAY = &TIMEOUT_2_MIN;
}

void loop() {
  Messenger.loop();
  ZTiming.loop();

 
  uint8_t date[7];
  if(ZClock.updateDateTime(date))
  {
    isRelay_1_EnablePeriod = ZTiming.checkTimingList(0, 1, date);
    isRelay_3_EnablePeriod = ZTiming.checkTimingList(1, 2, date);
  }

  
  delay(10);
  //读取HC-RS501释热传感器状态
  bool isHcrs_1_Fired = hcrs501_isFired(1, HCSR501_1_PIN);
  //判断人体持续活动
  bool isContinueActive = hcrs501_continue_active(0, isHcrs_1_Fired);
  if (body_1_active != isHcrs_1_Fired)
  {
    delay(100);
    isHcrs_1_Fired = hcrs501_isFired(1, HCSR501_1_PIN);
    if (body_1_active != isHcrs_1_Fired)
    {
      body_1_active = isHcrs_1_Fired;
      Messenger.print("Hcrs501 No.1 is ");
      Messenger.println(body_1_active?"on":"off");
    }
  }

  
  //读取HC-RS501释热传感器状态
  bool isHcrs_2_Fired = hcrs501_isFired(2, HCSR501_2_PIN);
  //判断人体持续活动
  if(!isContinueActive) isContinueActive = hcrs501_continue_active(1, isHcrs_2_Fired);
  if (body_2_active != isHcrs_2_Fired)
  {
    delay(100);
    isHcrs_2_Fired = hcrs501_isFired(2, HCSR501_2_PIN);
    if (body_2_active != isHcrs_2_Fired)
    {
      body_2_active = isHcrs_2_Fired;
      Messenger.print("Hcrs501 No.2 is ");
      Messenger.println(body_2_active?"on":"off");
    }
  }

  //读取手动开关状态
  bool isManual = !digitalRead(MANUAL_PIN);
  Messenger.debugInfo("SW" ,isManual?ZTRUE:ZFALSE, 2);
  bool manual_state_changed = false;
  if (manual_state != isManual)
  {
    delay(100);
    isManual = !digitalRead(MANUAL_PIN);
    if (manual_state != isManual)
    {
      manual_state = isManual;
      manual_state_changed = true;
      Messenger.print("Switch is ");
      Messenger.println(manual_state?"on":"off");
      delay(100);
    }
  }
  Messenger.debugInfo("Relay_1_EnablePeriod" ,(isRelay_1_EnablePeriod?"true":"false"), 3);
  
 //读取光敏电阻状态
  bool isBright = digitalRead(PHOTO_RESISTOR_PIN);
  if (bright_state != isBright)
  {
    delay(100);
    isBright = !digitalRead(PHOTO_RESISTOR_PIN);
    if (bright_state != isBright)
    {
      bright_state = isBright;
    }
  }
  Messenger.debugInfo("Photo Resistor" ,(bright_state?"true":"false"), 4);
  
  //继电器1 - 主灯 
  //在工作时间内，任意一个人体感应触发，延迟p_RELAY_1_DELAY时长自动关闭，人体感应一直触发则延迟重新计时。
  //手动开关忽略延迟直接触发，且如果是开启则延迟加长，关则30秒内不响应人体感应。
  bool isRelay_1_changed = false;
  bool condition_1 = ((body_1_active || body_2_active) && isRelay_1_EnablePeriod) || Messenger.checkRemoteCommand(0x01);
  bool light_1_timeout = (millis() - relay_1_startTime > *p_RELAY_1_DELAY) || (relay_1_startTime == 0);
  if (manual_state_changed || (light_1_timeout && relay_1_state != condition_1)) {
    if (manual_state_changed)
    {
      relay_1_state = !relay_1_state;
      relay_2_on_manual = relay_1_state;
      relay_3_on_manual = relay_1_state;
      p_RELAY_1_DELAY = relay_1_state ? &TIMEOUT_5_MIN : &TIMEOUT_30_SEC;//关则30秒内不响应人体感应
      //p_RELAY_3_DELAY = p_RELAY_1_DELAY;
      relay_1_startTime = millis();
    }
    else {
      p_RELAY_1_DELAY = body_2_active ?  (isContinueActive ? &TIMEOUT_2_MIN : &TIMEOUT_60_SEC) : &TIMEOUT_10_SEC;
      //p_RELAY_3_DELAY = p_RELAY_1_DELAY;
       bool ignore = false;
      //光线足则不自动开灯
      if(!relay_1_state && !relay_2_state && bright_state) ignore = true;
      relay_1_state = condition_1 && !ignore;
    }
    
    Messenger.print("relay No.1 is ");
    Messenger.println(relay_1_state ? "on" : "off");
    digitalWrite(RELAY1_PIN, !relay_1_state);
    isRelay_1_changed = true;
  }
  if (relay_1_state && ((body_1_active || body_2_active ) && isRelay_1_EnablePeriod)) relay_1_startTime = millis();
  Messenger.debugInfo("RELAY_1_DELAY" ,String(*p_RELAY_1_DELAY), 5);

  //继电器2  - 副灯
  //手动开启
  //仅手动关或者超时后熄灭
  bool isRelay_2_changed = false;
  if (light_1_timeout) relay_2_on_manual = false;
  if (relay_2_state != (relay_2_on_manual || Messenger.checkRemoteCommand(0x02)))
  {
    relay_2_state = relay_2_on_manual || Messenger.checkRemoteCommand(0x02);
    Messenger.print("relay No.2 is ");
    Messenger.println(relay_2_state ? "on" : "off");
    digitalWrite(RELAY2_PIN, !relay_2_state);
    isRelay_2_changed = true;
  }
  //修正延迟
  if(*p_RELAY_1_DELAY == TIMEOUT_10_SEC)
  {
     p_RELAY_1_DELAY = body_2_active ?  &TIMEOUT_60_SEC : &TIMEOUT_10_SEC;
  }
  else if(*p_RELAY_1_DELAY == TIMEOUT_60_SEC)
  {
     p_RELAY_1_DELAY = isContinueActive ? &TIMEOUT_2_MIN : &TIMEOUT_60_SEC;
  }
  p_RELAY_3_DELAY = p_RELAY_1_DELAY;
  //小夜灯
  //当在主灯非工作区间，主副灯都不亮时，人体感应触发开启。
  bool condition1_1 = !relay_1_state && !relay_2_state && !isRelay_1_EnablePeriod && (body_1_active || body_2_active);
  //释热传感器延迟短，加TIMEOUT_30_SEC延迟，isRelay_1_changed修正状态变化及时性
  if ((night_light_state != condition1_1) && ( (millis() - night_light_startTime > TIMEOUT_30_SEC) || isRelay_1_changed || isRelay_2_changed) )
  {
    night_light_state = condition1_1;
    if(night_light_state) fadeIn();
    else fadeOut();
  }
  if (night_light_state && (body_1_active || body_2_active)) night_light_startTime = millis();
  Messenger.debugInfo("relay_3_on_manual" ,relay_3_on_manual?"true":"false", 6);
  Messenger.debugInfo("Relay_3_EnablePeriod" ,isRelay_3_EnablePeriod?"true":"false", 7);
  //继电器3  - 排风扇 人体感应触发，可手动开关
  if (light_1_timeout) relay_3_on_manual = false;
  bool condition_3 = ( body_2_active && isRelay_3_EnablePeriod) || Messenger.checkRemoteCommand(0x04);
  bool relay_3_timeout = relay_3_startTime == 0 || (millis() - relay_3_startTime > *p_RELAY_3_DELAY);
  if (manual_state_changed || ((relay_3_state != condition_3) && relay_3_timeout))
  {
    if(manual_state_changed)
    {
      relay_3_state = relay_3_on_manual;
      relay_3_startTime = millis();
    }
    else relay_3_state = condition_3;
    Messenger.print("Relay No.3 is ");
    Messenger.println(relay_3_state?"on":"off");
    digitalWrite(RELAY3_PIN, !relay_3_state);
  }
  if (relay_3_state && (body_1_active || body_2_active)) relay_3_startTime = millis();
  Messenger.debugInfo("RELAY_3_DELAY" ,String(*p_RELAY_3_DELAY), 8);
  //继电器4  - 新风机
  if (relay_4_state != (Messenger.checkRemoteCommand(0x08))) {
    relay_4_state =  Messenger.checkRemoteCommand(0x08);
    Messenger.print("relay No.4 ");
    Messenger.println(relay_4_state ? "on" : "off");
    digitalWrite(RELAY4_PIN, !relay_4_state);
  }
  
  //更新继电器状态
  uint8_t relayState = 0;
  relayState |= relay_1_state ? 0x01 : 0;
  relayState |= relay_2_state ? 0x02 : 0;
  relayState |= relay_3_state ? 0x04 : 0;
  relayState |= relay_4_state ? 0x08 : 0;
  Messenger.setRelayState(relayState);

}

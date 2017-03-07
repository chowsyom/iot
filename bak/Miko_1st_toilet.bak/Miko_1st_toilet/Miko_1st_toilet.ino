#include <ZTiming.h>
#include <ZRF315MHz.h>
#include <ZHCRS501.h>

//#define _1ST_TOILET_  //选择主次卫

#ifdef _1ST_TOILET_
#define HCSR501_PIN A0
#define INFRARED_SWITCH_PIN A1
#define MANUAL_PIN 3
#define RELAY_0_PIN 4
#else
#define HCSR501_PIN A1
#define MANUAL_PIN A2
#define INFRARED_SWITCH_PIN A3
#endif

#define NIGHT_LIGHT_PIN 2
#define RELAY_1_PIN 5
#define RELAY_2_PIN 6
#define RELAY_3_PIN 7
#define RELAY_4_PIN 8

//10,11,12 保留给RF315M通信

#ifdef _1ST_TOILET_
//牛眼射灯
bool isRelay_0_On = false;
#endif
bool isRelay_1_On = false;
bool isRelay_2_On = false;
bool isRelay_3_On = false;
bool isRelay_4_On = false;
bool isNightLight_On = false;

bool isToilet_using = false;
bool hcrs501_sensor_state = false;
bool door_is_close = false;
bool manual_state = false;


unsigned long RELAY_1_DELAY_0 = 5000;   //毫秒
unsigned long RELAY_1_DELAY_1 = 20000;   //毫秒
unsigned long RELAY_1_DELAY_2 = 120000;   //毫秒
unsigned long* p_RELAY_1_DELAY;
unsigned long RELAY_2_DELAY = 120000;   //毫秒
unsigned long on1_startTime = -1 - RELAY_1_DELAY_2;
unsigned long on1_1_startTime = -1 - RELAY_1_DELAY_2;
unsigned long on2_startTime = -1 - RELAY_2_DELAY;
unsigned long on3_startTime = -1 - RELAY_1_DELAY_1;
unsigned long active_startTime = 0;
unsigned long deactive_startTime = -1 - RELAY_1_DELAY_1;
unsigned long deactive_endTime = 0;
unsigned long close_door_startTime = 0;
uint8_t rfMsg[8] = {0xFE, 0x7C, 0x07, 0, 1, 0x11, 0, 0};

void setup() {
  Messager.begin();
  ZTiming.begin();


#ifdef _1ST_TOILET_
  pinMode(MANUAL_PIN, INPUT_PULLUP);
  pinMode(RELAY_0_PIN, OUTPUT);
#else
  pinMode(MANUAL_PIN, INPUT);
#endif

  pinMode(HCSR501_PIN, INPUT);
  pinMode(INFRARED_SWITCH_PIN, INPUT);
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);
  pinMode(NIGHT_LIGHT_PIN, OUTPUT);

#ifdef _1ST_TOILET_
  digitalWrite(RELAY_0_PIN, LOW);
#endif
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(RELAY_3_PIN, HIGH);
  digitalWrite(RELAY_4_PIN, HIGH);
  digitalWrite(NIGHT_LIGHT_PIN, LOW);

  p_RELAY_1_DELAY = &RELAY_1_DELAY_0;
}

void loop() {

  bool manual_state_changed = false;
  bool door_open_flag = false;
  bool isContinueActive = false;

  Messager.working();
  ZTiming.working();

  boolean isRelay_1_EnablePeriod = ZTiming.checkTimingList(0, 1);
  boolean isRelay_2_EnablePeriod =  ZTiming.checkTimingList(1, 2);
  boolean isRelay_3_OnByTimer =  ZTiming.checkTimingList(2, 4, Messager.getOptByCmdFlag(2));
  boolean isRelay_4_OnByTimer =  ZTiming.checkTimingList(4, 8, Messager.getOptByCmdFlag(3));

  unsigned int sensorVal  = 0;
  //读取HC-RS501释热传感器状态
  bool isHcrs501Fired = hcrs501_isFired(1, HCSR501_PIN);

  //判断人体持续活动
  if (hcrs501_sensor_state != isHcrs501Fired)
  {
    hcrs501_sensor_state = isHcrs501Fired;
    if (hcrs501_sensor_state)
    {
      deactive_endTime = millis();
      active_startTime = millis();
    }
    else
    {
      deactive_startTime = millis();
      deactive_endTime = 0;
      active_startTime = 0;
    }
  }
  if ((deactive_endTime > 0 && deactive_endTime - deactive_startTime < RELAY_1_DELAY_1) || (active_startTime > 0 && millis() - active_startTime > RELAY_1_DELAY_1))
  {
    isContinueActive = true;
    deactive_startTime = -1 - RELAY_1_DELAY_1;
    deactive_endTime = 0;
    active_startTime = 0;
  }

  //读取手动开关状态
  bool isManual = false;
#ifdef _1ST_TOILET_
  sensorVal = digitalRead(MANUAL_PIN);
  isManual = !sensorVal;
#else
  sensorVal = analogRead(MANUAL_PIN);
  isManual = sensorVal > 256;
#endif
  Messager.tryReplySensorValue(sensorVal, 2);
  if (manual_state != isManual)
  {
    manual_state = isManual;
    manual_state_changed = true;
  }

  delay(1);
  //红外镜面反射光电开关infraredSwitch
  sensorVal = analogRead(INFRARED_SWITCH_PIN);
  if (door_is_close != sensorVal < 256)
  {
    door_is_close = sensorVal < 256;
    if (!door_is_close) door_open_flag = true;
    else close_door_startTime = millis();
  }
  Messager.tryReplySensorValue(sensorVal, 3);
  Messager.tryReplySensorValue(isRelay_1_EnablePeriod, 4);

  //Serial.print("HCRS505=");
  //Serial.print(isONByHCRS505Sensor);
  //  Serial.print(VW_MAX_MESSAGE_LEN);
  //    Serial.print(",HCRS501=");
  //    Serial.print(hcrs501_sensor_state);
  //    Serial.print(",MANUAL=");
  //    Serial.print(isManual);
  //    Serial.print(",SWITCH=");
  //    Serial.println(door_is_close);

  //    Serial.print("ZTiming=");
  //    Serial.print(isRelay_1_EnablePeriod);
  //    Serial.print(",");
  //    Serial.print(isRelay_2_EnablePeriod);
  //    Serial.print(",");
  //    Serial.print(isRelay_3_OnByTimer);
  //    Serial.print(",");
  //    Serial.println(isRelay_4_OnByTimer);
  //更新继电器状态
  //继电器1
  /*
     人体感器触发打开，触发后延迟RELAY_1_DELAY毫秒关闭
     关门后打开，开门后关闭
     手动开关立刻触发，状态反转
  */
  //发生开门事件
  if (door_open_flag)
  {
    p_RELAY_1_DELAY = &RELAY_1_DELAY_0;
  }
  bool condition1 = (hcrs501_sensor_state && isRelay_1_EnablePeriod) || door_is_close || Messager.checkRemoteCommand(0x01);
  bool light_on_timeout = millis() - on1_startTime > *p_RELAY_1_DELAY;
  Messager.tryReplySensorValue(condition1, 5);
  Messager.tryReplySensorValue(*p_RELAY_1_DELAY, 6);
  bool isRelay_1_changed = false;
  if (manual_state_changed || (light_on_timeout && isRelay_1_On != condition1)) {
    if (manual_state_changed)
    {
      manual_state_changed = false;
      isRelay_1_On = !isRelay_1_On;
#ifdef _1ST_TOILET_
      isRelay_0_On = isRelay_1_On;
#endif
      p_RELAY_1_DELAY = isRelay_1_On ? &RELAY_1_DELAY_2 : &RELAY_1_DELAY_1;
      on1_startTime = millis();
    }
    else {
      p_RELAY_1_DELAY = &RELAY_1_DELAY_1;
      isRelay_1_On = condition1;
    }
#ifdef _1ST_TOILET_
    //牛眼射灯 不受非手动控制的影响，仅手动关或者超时后熄灭
    if (millis() - on1_startTime > *p_RELAY_1_DELAY) isRelay_0_On = false;
    digitalWrite(RELAY_0_PIN, isRelay_0_On);
#endif
    digitalWrite(RELAY_1_PIN, !isRelay_1_On);
    isRelay_1_changed = true;
  }
  bool condition1_1 = !isRelay_1_EnablePeriod && !isRelay_1_On && hcrs501_sensor_state;
  //释热传感器延迟短，加RELAY_1_DELAY_1延迟，isRelay_1_changed修正状态变化及时性
  if (isNightLight_On != condition1_1 && (millis() - on1_1_startTime > RELAY_1_DELAY_1 || isRelay_1_changed))
  {
    isNightLight_On = condition1_1;
    digitalWrite(NIGHT_LIGHT_PIN,  isNightLight_On);
  }
  if (isNightLight_On && hcrs501_sensor_state) on1_1_startTime = millis();
  if (isRelay_1_On && (hcrs501_sensor_state && isRelay_1_EnablePeriod)) on1_startTime = millis();
  //继电器2
  //人体感器和关门后30秒触发，触发后延迟RELAY_2_DELAY毫秒关闭
  bool condition2 = (hcrs501_sensor_state && isRelay_2_EnablePeriod && isContinueActive) || (door_is_close && millis() - close_door_startTime > RELAY_1_DELAY_1) || Messager.checkRemoteCommand(0x02);
  if (millis() - on2_startTime > RELAY_2_DELAY && isRelay_2_On != condition2) {
    isRelay_2_On = condition2;
    digitalWrite(RELAY_2_PIN, !isRelay_2_On);
  }
  if (isRelay_2_On && (hcrs501_sensor_state && isRelay_2_EnablePeriod)) on2_startTime = millis();
  //继电器3
  //洁身器 关门时电源开启，开门后延迟RELAY_1_DELAY_1毫秒关闭
  if (isRelay_3_On != (isRelay_3_OnByTimer || door_is_close) && millis() - on3_startTime > RELAY_1_DELAY_1) {
    isRelay_3_On = isRelay_3_OnByTimer || door_is_close;
    digitalWrite(RELAY_3_PIN, !isRelay_3_On);
  }
  if (isRelay_3_On && door_is_close) on3_startTime = millis();
  //继电器4
  if (isRelay_4_On != isRelay_4_OnByTimer) {
    isRelay_4_On = isRelay_4_OnByTimer;
    digitalWrite(RELAY_4_PIN, !isRelay_4_On);
  }
  if (isToilet_using != door_is_close)
  {
    isToilet_using = door_is_close;
    rfMsg[5] = 0x11;
    rfMsg[6] = isToilet_using ? 0x01 : 0xFF;
    Messager.sendMsg(rfMsg, 8);
  }
  //设置空闲状态
  Messager.setIdle(!isRelay_1_On && !isRelay_2_On && !isRelay_3_On && !isRelay_4_On && !isNightLight_On);
  //防止量子纠缠现象
  delay(1);
}

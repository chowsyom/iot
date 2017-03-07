#include <ZTiming.h>
#include <ZRF315MHz.h>
#include <ZHCRS501.h>
//#include <ZHCRS505.h>


//#define HCSR505_PIN A0
#define HCSR501_PIN A1
#define MANUAL_PIN A2
#define INFRARED_SWITCH_PIN A3


#define RELAY_1_PIN 5
#define RELAY_2_PIN 6
#define RELAY_3_PIN 7
#define RELAY_4_PIN 8
#define NIGHT_LIGHT_PIN 2
//10,11,12 保留给RF315M通信


bool isRelay_1_On = false;
bool isRelay_2_On = false;
bool isRelay_3_On = false;
bool isRelay_4_On = false;
bool isNightLight_On = false;

bool isToilet_using = false;

bool manual_state = false;

unsigned long* RELAY_1_DELAY;
unsigned long RELAY_1_DELAY_0 = 5000;   //毫秒
unsigned long RELAY_1_DELAY_1 = 30000;   //毫秒
unsigned long RELAY_1_DELAY_2 = 300000;   //毫秒
unsigned long RELAY_2_DELAY = 120000;   //毫秒
unsigned long on1_startTime = -1 - RELAY_1_DELAY_2;
unsigned long on2_startTime = -1 - RELAY_2_DELAY;
unsigned long on3_startTime = -1 - RELAY_1_DELAY_1;
uint8_t rfMsg[8] = {0xFE, 0x7C, 0x07, 0, 1, 0x11, 0, 0};

void setup() {
  Messager.begin();
  ZTiming.begin();


  //pinMode(HCSR505_PIN, INPUT);
  pinMode(HCSR501_PIN, INPUT);
  pinMode(MANUAL_PIN, INPUT);
  pinMode(INFRARED_SWITCH_PIN, INPUT);
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);
  pinMode(NIGHT_LIGHT_PIN, OUTPUT);

  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(RELAY_3_PIN, HIGH);
  digitalWrite(RELAY_4_PIN, HIGH);
  digitalWrite(NIGHT_LIGHT_PIN, LOW);

  RELAY_1_DELAY = &RELAY_1_DELAY_1;
}

void loop() {
  Messager.working();
  ZTiming.working();


  boolean isRelay_1_EnablePeriod = ZTiming.checkTimingList(0, 1);
  boolean isRelay_2_EnablePeriod =  ZTiming.checkTimingList(1, 2);
  boolean isRelay_3_OnByTimer =  ZTiming.checkTimingList(2, 4, Messager.getOptByCmdFlag(2));
  boolean isRelay_4_OnByTimer =  ZTiming.checkTimingList(4, 8, Messager.getOptByCmdFlag(3));

  unsigned int sensorVal  = 0;
  //读取HC-RS501释热传感器状态
  bool isONByHCRS501Sensor = hcrs501_isFired(1, HCSR501_PIN);
  //读取手动开关状态
  bool isONByManual = false;
  bool manual_flag = false;
  sensorVal = analogRead(MANUAL_PIN);
  isONByManual = sensorVal > 256;
  Messager.tryReplySensorValue(sensorVal, 2);
  if (manual_state != isONByManual)
  {
    manual_state = isONByManual;
    manual_flag = true;
  }

  boolean isONByINFRAREDSWITCH = false;
  //红外镜面反射光电开关infraredSwitch
  sensorVal = analogRead(INFRARED_SWITCH_PIN);
  isONByINFRAREDSWITCH = sensorVal > 256;
  Messager.tryReplySensorValue(sensorVal, 3);
  Messager.tryReplySensorValue(isRelay_1_EnablePeriod, 4);

  //Serial.print("HCRS505=");
  //Serial.print(isONByHCRS505Sensor);
  //  Serial.print(VW_MAX_MESSAGE_LEN);
  //  Serial.print(",HCRS501=");
  //  Serial.print(isONByHCRS501Sensor);
  //  Serial.print(",MANUAL=");
  //  Serial.print(isONByManual);
  //  Serial.print(",SWITCH=");
  //  Serial.println(isONByINFRAREDSWITCH);

  //  Serial.print("ZTiming=");
  //  Serial.print(isRelay_1_EnablePeriod);
  //  Serial.print(",");
  //  Serial.print(isRelay_2_EnablePeriod);
  //  Serial.print(",");
  //  Serial.print(isRelay_3_OnByTimer);
  //  Serial.print(",");
  //  Serial.println(isRelay_4_OnByTimer);

  //更新继电器状态
  //继电器1
  bool condition1 = (isONByHCRS501Sensor && isRelay_1_EnablePeriod) || isONByINFRAREDSWITCH || Messager.checkRemoteCommand(0x01);
  Messager.tryReplySensorValue(condition1, 5);
  Messager.tryReplySensorValue(*RELAY_1_DELAY, 6);
  if (manual_flag || (millis() - on1_startTime > *RELAY_1_DELAY && isRelay_1_On != condition1)) {
    if (manual_flag)
    {
      manual_flag = false;
      isRelay_1_On = !isRelay_1_On;
      RELAY_1_DELAY = isRelay_1_On ? &RELAY_1_DELAY_2 : &RELAY_1_DELAY_1;
      on1_startTime = millis();
    }
    else isRelay_1_On = condition1;
    digitalWrite(RELAY_1_PIN, !isRelay_1_On);
  }
  bool condition1_1 = !isRelay_1_EnablePeriod && !isRelay_1_On && isONByHCRS501Sensor;
  //释热传感器延迟长，暂不额外加延迟
  if (isNightLight_On != condition1_1)
  {
    isNightLight_On = condition1_1;
    digitalWrite(NIGHT_LIGHT_PIN,  isNightLight_On);
  }
  if (isRelay_1_On && (isONByHCRS501Sensor && isRelay_1_EnablePeriod)) on1_startTime = millis();
  //继电器2
  bool condition2 = (isONByHCRS501Sensor && isRelay_2_EnablePeriod) || isONByINFRAREDSWITCH || Messager.checkRemoteCommand(0x02);
  if (millis() - on2_startTime > RELAY_2_DELAY && isRelay_2_On != condition2) {
    isRelay_2_On = condition2;
    digitalWrite(RELAY_2_PIN, !isRelay_2_On);
  }
  if (isRelay_2_On && (isONByHCRS501Sensor && isRelay_2_EnablePeriod)) on2_startTime = millis();
  //继电器3
  if (isRelay_3_On != (isRelay_3_OnByTimer || isONByINFRAREDSWITCH) && millis() - on3_startTime > RELAY_1_DELAY_1) {
    isRelay_3_On = isRelay_3_OnByTimer || isONByINFRAREDSWITCH;
    digitalWrite(RELAY_3_PIN, !isRelay_3_On);
  }
  if (isRelay_3_On && isONByINFRAREDSWITCH) on3_startTime = millis();
  //继电器4
  if (isRelay_4_On != isRelay_4_OnByTimer) {
    isRelay_4_On = isRelay_4_OnByTimer;
    digitalWrite(RELAY_4_PIN, !isRelay_4_On);
  }

  if (isToilet_using != isONByINFRAREDSWITCH)
  {
    isToilet_using = isONByINFRAREDSWITCH;
    rfMsg[5] = 0x11;
    rfMsg[6] = isToilet_using ? 0x01 : 0xFF;
    Messager.sendMsg(rfMsg, 8);
  }
  //设置空闲状态
  Messager.setIdle(!isRelay_1_On && !isRelay_2_On && !isRelay_3_On && !isRelay_4_On && !isNightLight_On);

}

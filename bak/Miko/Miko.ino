#include <ZRF315MHzMaster.h>
//#include <ZRF315MHz.h>
#include <ZHCRS501.h>
//#include <ZHCRS505.h>

//#define HCSR505_PIN A0
#define HCSR501_PIN A1
#define MANUAL_PIN A2
#define INFRARED_SWITCH_PIN A3


#define ON1_PIN 5
#define ON2_PIN 6
#define ON3_PIN 7
#define ON4_PIN 8
//#define ON5_PIN 2
//#define ON5_PIN 9
//10,11,12 保留给RF315M通信

//SoftwareSerial Serial1(RX_PIN, TX_PIN);
ZRF315MHz messager;

boolean isON1 = false;
boolean isON2 = false;
boolean isON3 = false;
boolean isON4 = false;
//boolean isON5 = false;

boolean manual_state = false;

unsigned long* ON1_DELAY;
unsigned long ON1_DELAY_1 = 30000;   //毫秒
unsigned long ON1_DELAY_2 = 300000;   //毫秒
unsigned long ON2_DELAY = 120000;   //毫秒
unsigned long ON3_DELAY = 30000;   //毫秒
//unsigned long ON5_DELAY = 20000;   //毫秒
unsigned long on1_startTime = -1 - ON1_DELAY_2;
unsigned long on2_startTime = -1 - ON2_DELAY;
unsigned long on3_startTime = -1 - ON3_DELAY;
//unsigned long on5_startTime = -1 - ON5_DELAY;

void setup() {
  Serial.begin(9600);
  messager.begin();

  //pinMode(HCSR505_PIN, INPUT);
  pinMode(HCSR501_PIN, INPUT);
  pinMode(MANUAL_PIN, INPUT);
  pinMode(INFRARED_SWITCH_PIN, INPUT);
  pinMode(ON1_PIN, OUTPUT);
  pinMode(ON2_PIN, OUTPUT);
  pinMode(ON3_PIN, OUTPUT);
  pinMode(ON4_PIN, OUTPUT);
  //pinMode(ON5_PIN, OUTPUT);

  digitalWrite(ON1_PIN, HIGH);
  digitalWrite(ON2_PIN, HIGH);
  digitalWrite(ON3_PIN, HIGH);
  digitalWrite(ON4_PIN, HIGH);
  //digitalWrite(ON5_PIN, HIGH);

  ON1_DELAY = &ON1_DELAY_1;
}

void loop() {

  handleSerialInputMsg();
  messager.working();


  boolean isONByRmCmd1 = messager.checkRemoteCommand(0x01);
  boolean isONByRmCmd2 = messager.checkRemoteCommand(0x02);
  boolean isONByTimer3 = checkTimingList(0, 4, 2);
  boolean isONByTimer4 = checkTimingList(4, 8, 3);

  unsigned int sensorVal  = 0;
  //读取HC-RS505释热传感器状态
  //bool isONByHCRS505Sensor = hcrs505_isFired(messager, 1, HCSR505_PIN);
  //读取HC-RS501释热传感器状态
  bool isONByHCRS501Sensor = hcrs501_isFired(messager, 2, HCSR501_PIN);
  //读取手动开关状态
  bool isONByManual = false;
  bool manual_flag = false;
  sensorVal = analogRead(MANUAL_PIN);
  isONByManual = sensorVal > 256;
  messager.tryReplySensorValue(sensorVal, 3);
  if (manual_state != isONByManual)
  {
    manual_state = isONByManual;
    manual_flag = true;
  }

  boolean isONByINFRAREDSWITCH = false;
  //红外镜面反射光电开关infraredSwitch
  sensorVal = analogRead(INFRARED_SWITCH_PIN);
  isONByINFRAREDSWITCH = sensorVal > 256;
  messager.tryReplySensorValue(sensorVal, 4);

  //Serial.print("HCRS505=");
  //Serial.print(isONByHCRS505Sensor);
//  Serial.print(VW_MAX_MESSAGE_LEN);
//  Serial.print(",HCRS501=");
//  Serial.print(isONByHCRS501Sensor);
//  Serial.print(",MANUAL=");
//  Serial.print(isONByManual);
//  Serial.print(",SWITCH=");
//  Serial.println(isONByINFRAREDSWITCH);

  //Serial.print("isONByTimer=");
  //Serial.print(isONByRmCmd1);
  //Serial.print(",");
  //Serial.print(isONByRmCmd2);
  //Serial.print(",");
  //Serial.print(isONByTimer3);
  //Serial.print(",");
  //Serial.println(isONByTimer4);

  //更新继电器状态
  //继电器1
  if (manual_flag || (millis() - on1_startTime > *ON1_DELAY && isON1 != (isONByHCRS501Sensor || isONByINFRAREDSWITCH || isONByRmCmd1))) {
    if (manual_flag)
    {
      isON1 = !isON1;
      manual_flag = false;
      ON1_DELAY = isON1 ? &ON1_DELAY_2 : &ON1_DELAY_1;
      on1_startTime = millis();
    }
    else isON1 = isONByHCRS501Sensor || isONByINFRAREDSWITCH || isONByRmCmd1;
    digitalWrite(ON1_PIN, !isON1);
  }
  if (isON1 && isONByHCRS501Sensor) on1_startTime = millis();
  //继电器2
  if (millis() - on2_startTime > ON2_DELAY && isON2 != (isONByHCRS501Sensor || isONByINFRAREDSWITCH || isONByRmCmd2)) {
    isON2 = isONByHCRS501Sensor || isONByINFRAREDSWITCH || isONByRmCmd2;
    digitalWrite(ON2_PIN, !isON2);
  }
  if (isON2 && isONByHCRS501Sensor) on2_startTime = millis();
  //继电器3
  if (millis() - on3_startTime > ON3_DELAY && isON3 != (isONByTimer3 || isONByINFRAREDSWITCH)) {
    isON3 = isONByTimer3 || isONByINFRAREDSWITCH;
    digitalWrite(ON3_PIN, !isON3);
  }
  if (isON3) on3_startTime = millis();
  //继电器4
  if (isON4 != isONByTimer4) {
    isON4 = isONByTimer4;// || (onByRMCmd & 0x08);
    digitalWrite(ON4_PIN, !isON4);
  }
  //  //继电器5
  //  if (millis() - on5_startTime > ON5_DELAY && isON5 != isONByHCRS505Sensor) {
  //    isON5 = isONByHCRS505Sensor;
  //    digitalWrite(ON5_PIN, isON5);
  //  }
  //  if (isON5 && isONByHCRS505Sensor) on5_startTime = millis();

}

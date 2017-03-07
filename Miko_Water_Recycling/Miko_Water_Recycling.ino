#include <Messenger.h>

#define STATE_ROM_ADDR 51

#define D2_PIN 2
#define D4_PIN 4
#define D5_PIN 5

#define RELAY1_PIN 12
#define RELAY2_PIN 13
#define RELAY3_PIN 14
#define RELAY4_PIN 16

const unsigned long WATER_FULL_DELAY = 30000; //毫秒
const unsigned long RELAY1_DELAY  = 4500000;  //90分钟=90*60*1000=4500000   //毫秒
const unsigned long RELAY2_DELAY = 600000;   //毫秒
const char *ZTRUE = "true";
const char *ZFALSE = "false";

bool isRelay_1_On = false;
bool isRelay_2_On = false;
bool isRelay_3_On = false;
bool isRelay_4_On = false;

bool isToilet_1_using = false;
bool isToilet_2_using = false;
bool isBucket_have_water = true;
bool isBucket_Full = false;

bool isWaterFilterRun = false;
bool isWaterFilterTimeout = false;
bool isWaterLow = false;
bool isWaterHigh = false;

bool relay1_state = false;
bool relay2_state = false;
bool relay3_state = false;
bool relay4_state = false;

unsigned long waterfull_startTime = 0;
unsigned long relay1_startTime = 0;
unsigned long relay2_startTime = 0;
uint8_t msg_11[5];
uint8_t msg_12[5];

void handleCustomMessage()
{
  uint8_t sbuf[2];
  
  msg_11[0] = 0x11;
  if (Messenger.listen(msg_11))
  {
      if (msg_11[1] & 0x02)
      {
        isToilet_1_using = msg_11[1] & 0x01;
        EEPROM.write(STATE_ROM_ADDR, isToilet_1_using ? 1 : 0);
        EEPROM.commit();
        Messenger.print("Toilet No.1 ");
        Messenger.println(isToilet_1_using ? "using." : "not used.");
        delay(10);
      }
      else if (msg_11[1] & 0x20)
      {
        isToilet_2_using = msg_11[1] & 0x10;
        EEPROM.write(STATE_ROM_ADDR+1, isToilet_2_using ? 1 : 0);
        EEPROM.commit();
        Messenger.print("Toilet No.2 ");
        Messenger.println(isToilet_2_using ? "using." : "not used.");
        delay(10);
      }
      //发应答指令
      sbuf[0] = 0x11;
      sbuf[1] = isToilet_1_using ? 0x03 : 0x02;
      sbuf[1] = sbuf[1] | (isToilet_2_using ? 0x30 : 0x20);
      Messenger.response(sbuf, 2);
  }

  
  msg_12[0] = 0x12;
  if (Messenger.listen(msg_12))
  {
      isBucket_have_water = msg_12[1] != 0xFF && msg_12[1] >= 0x01;
      isBucket_Full = msg_12[1] != 0xFF && msg_12[1] >= 0x07;
      EEPROM.write(STATE_ROM_ADDR+2, isBucket_have_water ? 1 : 0);
      EEPROM.write(STATE_ROM_ADDR+3, isBucket_Full ? 1 : 0);
      EEPROM.commit();
      Messenger.print("Bucket ");
      Messenger.print(isBucket_have_water ? "have" : "not");
      Messenger.println(" water.");
      delay(100);
  }
}

void setup() {
  Messenger.begin();
  isToilet_1_using = EEPROM.read(STATE_ROM_ADDR) == 1;
  isToilet_2_using = EEPROM.read(STATE_ROM_ADDR+1) == 1;
  isBucket_have_water = EEPROM.read(STATE_ROM_ADDR+2) == 1;
  isBucket_Full = EEPROM.read(STATE_ROM_ADDR+3) == 1;
  
  pinMode(D2_PIN, INPUT_PULLUP);
  pinMode(D4_PIN, INPUT_PULLUP);
  pinMode(D5_PIN, INPUT_PULLUP);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);

}

void loop() {
  Messenger.loop();
  delay(10);
  handleCustomMessage();
  delay(10);
  bool filterState = !digitalRead(D2_PIN);
  if (filterState)
  {
    delay(1000);
    filterState = !digitalRead(D2_PIN);
  }
  if (isWaterFilterRun != filterState)
  {
    isWaterFilterRun = filterState;
    Messenger.print("Water filter ");
    Messenger.println(isWaterFilterRun ? "runing" : "stop");
  }
  isWaterLow = digitalRead(D4_PIN);
  isWaterHigh = !digitalRead(D5_PIN);

  Messenger.debugInfo("Water Filter Run", isWaterFilterRun ? ZTRUE : ZFALSE, 1);
  Messenger.debugInfo("Water Low", isWaterLow ? ZTRUE : ZFALSE, 2);
  Messenger.debugInfo("Water High", isWaterHigh ? ZTRUE : ZFALSE, 3);
  Messenger.debugInfo("Water Filter Timeout", isWaterFilterTimeout ? ZTRUE : ZFALSE, 4);
  Messenger.debugInfo("Toilet 1 Using", isToilet_1_using ? ZTRUE : ZFALSE, 5);
  Messenger.debugInfo("Toilet 2 Using", isToilet_2_using ? ZTRUE : ZFALSE, 6);
  Messenger.debugInfo("Bucket Have Water", isBucket_have_water ? ZTRUE : ZFALSE, 7);
  Messenger.debugInfo("Bucket Full", isBucket_Full ? ZTRUE : ZFALSE, 8);
  delay(500);
  if (Messenger.checkRemoteCommand(0x01)) isWaterFilterTimeout = false;
  //更新继电器状态
  //继电器1
  if (!isWaterHigh) waterfull_startTime = millis();
  if (isRelay_1_On = (isWaterFilterRun && !isWaterFilterTimeout))
  {
    relay2_startTime = millis();
  }
  delay(1);
  //高水位持续WATER_FULL_DELAY后停止制水
  if (millis() - waterfull_startTime > WATER_FULL_DELAY) {
    isRelay_1_On = false;
    Messenger.println("WATER_FULL, stop water filter.");
    delay(5000);
  }
  //制水时间超过RELAY1_DELAY停止制水
  if (millis() - relay1_startTime > RELAY1_DELAY ) {
    isRelay_1_On = false;
    isWaterFilterTimeout = true;
    Messenger.println("Water filter work time too long...");
    delay(5000);
  }
  delay(1);
  if (relay1_state != isRelay_1_On)
  {
    relay1_state = isRelay_1_On;
    digitalWrite(RELAY1_PIN, !relay1_state);
  }
  if (!isRelay_1_On) relay1_startTime = millis();
  delay(1);
  //继电器2
  //制水停止RELAY2_DELAY后，排水停止
  if (millis() - relay2_startTime < RELAY2_DELAY )
  {
    //高于高水位时打开，低于低水位时关闭
    if (isWaterHigh) isRelay_2_On = true;
    if (isWaterLow) isRelay_2_On = false;
    else if (!isRelay_1_On) isRelay_2_On = true;
    //isRelay_2_On = isRelay_2_On && isWaterHigh;
  }
  else {
    isRelay_2_On = false;
    //Messenger.println("RELAY2_DELAY timeout");
  }
  if (relay2_state != isRelay_2_On)
  {
    relay2_state = isRelay_2_On;
    digitalWrite(RELAY2_PIN, !relay2_state);
  }

  //继电器3
  if (relay3_state != Messenger.checkRemoteCommand(0x04)) {
    relay3_state = Messenger.checkRemoteCommand(0x04);
    digitalWrite(RELAY3_PIN, !relay3_state);
  }
  delay(1);
  //继电器4
  if (relay4_state != ((Messenger.checkRemoteCommand(0x08) || (!isBucket_have_water || isToilet_1_using || isToilet_2_using)) && !isBucket_Full)) {
    relay4_state =  (Messenger.checkRemoteCommand(0x08) || (!isBucket_have_water || isToilet_1_using || isToilet_2_using)) && !isBucket_Full;
    Messenger.print("Switch to ");
    Messenger.println(relay4_state ? "clean water" : "recycle water");
    digitalWrite(RELAY4_PIN, !relay4_state);
  }
  Messenger.debugInfo("Using Water", relay4_state ? "Clean" : "Recycle", 9);
  //更新继电器状态
  uint8_t relayState = 0;
  relayState |= relay1_state ? 0x01 : 0;
  relayState |= relay2_state ? 0x02 : 0;
  relayState |= relay3_state ? 0x04 : 0;
  relayState |= relay4_state ? 0x08 : 0;
  Messenger.setRelayState(relayState);
  //设置空闲状态
  Messenger.setIdle(!relay1_state && !relay2_state && !relay3_state && !relay4_state);

}

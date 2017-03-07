#include <Messenger.h>

#define _1ST_TOILET_  //选择主次卫

#define IN_1_PIN  2
#define RELAY_1_PIN 0

const char *ZTRUE = "true";
const char *ZFALSE = "false";

uint8_t rfMsg[10] = {0xFE, 0x7C, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t msg11Cb[5] = {0xFD,0x11,0,0,0};

bool lid_state = false;
bool relay_1_state = false;
bool isClean_water = false;
bool isUsing_toilet_lid = false;

unsigned long no_using_startTime = 0;
unsigned long msg11_startTime = 0;

void setup() {
  Serial.begin(115200);
  Messenger.begin();
  pinMode(IN_1_PIN, INPUT_PULLUP);
  pinMode(RELAY_1_PIN, OUTPUT);
  digitalWrite(RELAY_1_PIN, HIGH);
}

void loop() {
  Messenger.loop();

  bool lid_using = !digitalRead(IN_1_PIN);
  if (lid_state != lid_using)
  {
    delay(500);
    lid_using = !digitalRead(IN_1_PIN);
    if (lid_state != lid_using)
    {
      lid_state = lid_using;
      if(!lid_state) no_using_startTime = millis();
    }
  }
  if(isUsing_toilet_lid != lid_state)
  {
    //关闭时延迟10秒,避免频繁开关电源和误判
    if(lid_state || (millis() - no_using_startTime > 10000))
    {
      isUsing_toilet_lid = lid_state;
    }
  }

  Messenger.debugInfo("lid using", lid_using ? ZTRUE : ZFALSE);
  Messenger.debugInfo("lid state", lid_state ? ZTRUE : ZFALSE);
  Messenger.debugInfo("isUsing_toilet_lid", isUsing_toilet_lid ? ZTRUE : ZFALSE);
  
  //侦听请求应答
  if(Messenger.listen(msg11Cb))
  {
    if(msg11Cb[1]==0x11)
    {
      #ifdef _1ST_TOILET_
        isClean_water = msg11Cb[2] & 0x01;
      #else
        isClean_water = msg11Cb[2] & 0x10;
      #endif
      Messenger.print("Clean water = ");
      Messenger.println(isClean_water?"true":"false");
    }
  }
  Messenger.debugInfo("clean water", isClean_water?ZTRUE:ZFALSE);
  if ((isClean_water != isUsing_toilet_lid) && (millis() - msg11_startTime > 10000))
  {
    msg11_startTime = millis();
#ifdef _1ST_TOILET_
    uint8_t toiletId = 0x02;
    uint8_t toiletUsed = 0x01;
#else
    uint8_t toiletId = 0x20;
    uint8_t toiletUsed = 0x10;
#endif
    rfMsg[1] = 0x7F;
    rfMsg[2] = 0x32;
    rfMsg[5] = 0x11;
    rfMsg[6] = isUsing_toilet_lid ? toiletUsed : 0;//状态
    rfMsg[6] = rfMsg[6] | toiletId;//加卫生间id
    rfMsg[7] = 0;
    Messenger.sendMsg(rfMsg, 8);
  }

  
  //继电器1
  if ( relay_1_state != (Messenger.checkRemoteCommand(0x01) || isUsing_toilet_lid)) {
    relay_1_state = Messenger.checkRemoteCommand(0x01) || isUsing_toilet_lid;
    digitalWrite(RELAY_1_PIN, !relay_1_state);
  }
  //继电器2
//  if ( relay_2_state != (isPowerOnByTimingOrCmd || isPowerOnByPm25)) {
//    relay_2_state = isPowerOnByTimingOrCmd || isPowerOnByPm25;
//    digitalWrite(RELAY2_PIN, !relay_2_state);
//  }
  
  //更新继电器状态
  uint8_t relayState = 0;
  relayState |= relay_1_state ? 0x01 : 0;
  Messenger.setRelayState(relayState);
  //设置空闲状态
  Messenger.setIdle(!relay_1_state);

}

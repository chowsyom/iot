#define _NO_LOAD_TIMING_H_
#include <ZRF315MHzMaster.h>
//#include <ZRF315MHz.h>

#define RELAY1_PIN 5
#define RELAY2_PIN 6
#define RELAY3_PIN 7
#define RELAY4_PIN 8
//10,11,12 保留给RF315M通信

ZRF315MHz messager;

boolean isRelay_1_On = false;
boolean isRelay_2_On = false;
boolean isRelay_3_On = false;
boolean isRelay_4_On = false;

boolean relay_1_state = false;
boolean relay_2_state = false;
boolean relay_3_state = false;

void setup() {
  //Serial.begin(9600);
  messager.begin();
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

#ifdef _ZRF315MHzMaster_H_
  handleSerialInputMsg();
#endif
  messager.working();
  boolean isRelay_1_On_Cmd = messager.checkRemoteCommand(0x01);
  boolean isRelay_2_On_Cmd = messager.checkRemoteCommand(0x02);
  boolean isRelay_3_On_Cmd = messager.checkRemoteCommand(0x04);
  boolean isRelay_4_On_Cmd = messager.checkRemoteCommand(0x08);
  //Serial.print(isRelay_1_On_Cmd);
  //Serial.print(",");
  //Serial.print(isRelay_2_On_Cmd);

  //更新继电器状态
  //继电器1,2,3互斥
    isRelay_1_On = !isRelay_3_On_Cmd && !isRelay_2_On_Cmd && isRelay_1_On_Cmd;
    isRelay_2_On = !isRelay_3_On_Cmd && isRelay_2_On_Cmd;
    isRelay_3_On = isRelay_3_On_Cmd;
   //继电器1
  if ( relay_1_state !=  isRelay_1_On) {
    relay_1_state = isRelay_1_On;
    digitalWrite(RELAY1_PIN, !relay_1_state);
  } 
  //继电器2
  if ( relay_2_state !=  isRelay_2_On) {
    relay_2_state = isRelay_2_On;
    digitalWrite(RELAY2_PIN, !relay_2_state);
  }
  //继电器3
  if ( relay_3_state !=  isRelay_3_On) {
    relay_3_state = isRelay_3_On;
    digitalWrite(RELAY3_PIN, !relay_3_state);
  }

}

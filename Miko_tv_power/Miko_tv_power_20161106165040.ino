#include <ZRF315MHz.h>

#define RELAY1_PIN 5
#define RELAY2_PIN 6
//10,11,12 保留给RF315M通信

boolean isRelay_1_On = false;
boolean isRelay_2_On = false;

void setup() {
  //Serial.begin(9600);
  Messager.begin();
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
}

void loop() {
  Messager.working();
  boolean isRelay_1_On_Cmd = Messager.checkRemoteCommand(0x01);
  boolean isRelay_2_On_Cmd = Messager.checkRemoteCommand(0x02);

  //Serial.print(isRelay_1_On_Cmd);
  //Serial.print(",");
  //Serial.print(isRelay_2_On_Cmd);

  //更新继电器状态
  //继电器1
  if ( isRelay_1_On !=  isRelay_1_On_Cmd) {
    isRelay_1_On = isRelay_1_On_Cmd;
    EEPROM.write(60, isRelay_1_On ? 1 : 0);
    digitalWrite(RELAY1_PIN, !isRelay_1_On);
  }
  //继电器2
  if (isRelay_2_On !=  isRelay_2_On_Cmd) {
    isRelay_2_On = isRelay_2_On_Cmd;
    EEPROM.write(61, isRelay_2_On ? 1 : 0);
    digitalWrite(RELAY2_PIN, !isRelay_2_On);
  }

}

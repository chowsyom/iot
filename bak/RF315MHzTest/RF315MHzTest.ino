#include <ZRF315MHz.h>

ZRF315MHz messager;

void setup() {
  Serial.begin(9600);
  messager.begin();
}

void loop() {

 uint8_t input[MAX_INPUT_SIZE + 1];
  //处理串口输入的信息
  messager.handleSerialInputMsg(input);
  //如果收到远程指令，处理指令
  if (messager.receiveMsg()) messager.handleCommand();
  else messager.checkMsgTimeout();

}

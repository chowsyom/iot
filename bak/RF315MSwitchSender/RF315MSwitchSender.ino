/*
  Example for different sending methods
  
  https://github.com/sui77/rc-switch/
  
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
bool bWriteLow = false;
void setup() {

  Serial.begin(9600);
  
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);
  
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);
  
  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);
  
}

void loop() {

  /* See Example: TypeA_WithDIPSwitches */
  //mySwitch.switchOn("11111", "00010");
  //delay(1000);
 // mySwitch.switchOn("11111", "00010");
 // delay(1000);

  /* Same switch as above, but using decimal code */
//  mySwitch.send(5393, 24);
//  delay(1000);  
//  mySwitch.send(5396, 24);
//  delay(1000);  

  /* Same switch as above, but using binary code */
  //mySwitch.send("000000000001010100010001");
  //delay(1000);  
//  mySwitch.send("000000000001010100010100");
//  delay(1000);

  /* Same switch as above, but tri-state code */ 
//  mySwitch.sendTriState("00000FFF0F0F");
//  delay(1000);  
//  mySwitch.sendTriState("00000FFF0FF0");
//  delay(1000);

int bStop[]={10896,352,1104,332,1100,344,1088,372,1052,380,1052,360,1080,344,1096,340,1100,332,1104,320,1104,336,1044,1084,352,384,1040,1060,384,368,1064,1040,404,336,1100,336,1072,392,1044,1076,368,360,1068,356,1064,340,1092,1028,424};
for(int i = 0;i<67;i++)
  {
    if(bWriteLow)digitalWrite(10, LOW);
    else digitalWrite(10, HIGH);
    delayMicroseconds(bStop[i]);//这里延迟上面取到的微秒数，轮流一高一低一高一低
    bWriteLow = !bWriteLow;
  }

  delay(5000);
}

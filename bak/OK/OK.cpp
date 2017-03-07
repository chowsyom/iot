#include "OK.h"

void mm()
{
  // 产生一个10us的高脉冲去触发TrigPin 
        digitalWrite(TrigPin, LOW); 
        delayMicroseconds(2); 
       digitalWrite(TrigPin, HIGH); 
        delayMicroseconds(10);
       digitalWrite(TrigPin, LOW); 
    // 检测脉冲宽度，并计算出距离
        distance = pulseIn(EchoPin, HIGH) / 58.00;
        Serial.print(distance); 
       Serial.print("cm"); 
        Serial.println(); 
        delay(1000); 
}
#include "OK.h"

void mm()
{
  // ����һ��10us�ĸ�����ȥ����TrigPin 
        digitalWrite(TrigPin, LOW); 
        delayMicroseconds(2); 
       digitalWrite(TrigPin, HIGH); 
        delayMicroseconds(10);
       digitalWrite(TrigPin, LOW); 
    // ��������ȣ������������
        distance = pulseIn(EchoPin, HIGH) / 58.00;
        Serial.print(distance); 
       Serial.print("cm"); 
        Serial.println(); 
        delay(1000); 
}
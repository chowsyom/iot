#include <RCSwitch.h>

#define LED 12

unsigned long FRESH_AIR_POWER_ON[3] = {5758211,5758256,5758220};
unsigned long FRESH_AIR_POWER_OFF[3] = {5758400,0,0};
uint8_t CMD[9] = {0xFE, 0x7C, 0x35, 0x3C, 0x01, 0x02, 0, 0, 0};
uint8_t counter = 1;

RCSwitch rcSwitch = RCSwitch();

void sendCommand(uint8_t name, uint8_t value)
{
   digitalWrite(LED,HIGH);
   CMD[4] = counter++;
   CMD[6] = name;
   CMD[7] = value;
   delay(10);
   Serial.write(CMD,9);
   delay(500);
   Serial.write(CMD,9);
   delay(200);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED,LOW);
  rcSwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
}

void loop() {
   if(Serial.available())
  {
    //Serial.println("Serial Got Data.");
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    Serial.readBytes(buf, len);
//    for(uint8_t i=0;i<len;i++)
//    {
//      Serial.print(buf[i],HEX);
//      Serial.print(" ");
//    }
    if(buf[0] == 0xFE && buf[1] == 0x7C && buf[2] == 0x3C && buf[5] == 0xFD)
    {
      //delay(3000);
      digitalWrite(LED,LOW);
      Serial.println("Sended.");
    }
  }
  if (rcSwitch.available()) 
  {
    unsigned long value = rcSwitch.getReceivedValue();
    if (value)
    {
        for(uint8_t i=0;i<3;i++)
        {
            //0x01 = 0001  power on ,default Middle
            //0x03 = 0011  Low
            //0x07 = 0111  Middle
            //0x0F = 1111  High
            if(value == FRESH_AIR_POWER_ON[i])
            {
              sendCommand(i==0?0x03:0x07,0x01);
              break;
            }
            if(value == FRESH_AIR_POWER_OFF[i])
            {
              sendCommand(0x0F,0);
              break;
            }
        }
    }
    rcSwitch.resetAvailable();
  }

}

#include <ZTiming.h>



void setup() {
  Serial.begin(9600);
  Wire.begin();
  readTimingListFromRom(1);
  char input[]="09:00-10:00,01;18:23-18:25,31;18:26-18:27,31;00:00-00:00,04;08:00-09:00,05;17:00-18:00,06;21:00-22:00,07;22:00-19:30,04";
  uint8_t timeArray[41];
  uint8_t len = 0;
  parseTimingList(input, timeArray, &len);
  timeArray[len] = 0;
  uint8_t i=0;
  Serial.print("timeArray[");
  Serial.print(len,DEC);
  Serial.print("]=[");
  for(i=0;i<len;i++){
    if(timeArray[i]==0) timeArray[i] = 0xFF;
    Serial.print(timeArray[i]);
    Serial.print(" ");
  }
  Serial.println("]");
  //writeTimingListToRom(1,timeArray);
  //readTimingListFromRom(1);
  //setOptCmdTime(1,18,24,false);
}

void loop() {
  
  
  Serial.println(checkTimingList(0,4,1));
  delay(5000);

}

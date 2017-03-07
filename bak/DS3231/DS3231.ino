#include <Wire.h>

#define RTC_Address   0x68  //0x32//时钟器件IIC总线地址

//从SD2403获取日期时间
unsigned char date[7];
String format(char c) {
  return (c < 10 ? "0" : "") + String(int(c));
}
void updateDateTime(void) {
  unsigned char n = 0;
  Wire.beginTransmission(RTC_Address);
  Wire.write(uint8_t(0x00));
  Wire.endTransmission();

  Wire.requestFrom(RTC_Address, 7);
  while (Wire.available())
  {
    date[n] = Wire.read();
    if (n == 2) {
      date[n] = (date[n] & 0x7f);
    }
    date[n] = (((date[n] & 0xf0) >> 4) * 10) + (date[n] & 0x0f);
    n++;
  }
  delayMicroseconds(1);
  Wire.endTransmission();
}
String  getDateTime(void)
{
  updateDateTime();
  return "20" + format(date[6]) + "-" + format(date[5]) + "-" + format(date[4]) + " " + format(date[2]) + ":" + format(date[1]) + ":" + format(date[0]) + ", " + String(int(date[3]));
}
byte decToBcd(byte val)
{
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}
void setDateTime(byte* date)
{
  Wire.beginTransmission(RTC_Address);
  Wire.write(uint8_t(0x00));
  char i;
  for (i = 0; i < 7; i++)
  {
    byte val = decToBcd(date[i]);
    if (i == 2) {
      val = val & 0b10111111;
    }
    Wire.write(val);
  }
  Wire.endTransmission();

  // Clear OSF flag
  Wire.write(uint8_t(0x0f));
  Wire.endTransmission();
  Wire.requestFrom(RTC_Address, 1);
  byte ctrl = Wire.read();
  Wire.write(uint8_t(0x0f));
  Wire.write(ctrl & 0b01111111);
  Wire.endTransmission();
}




void setup() {
  Serial.begin(9600);
  Wire.begin();
  //byte date[7]={0,20,20,2,20,9,16};
  //setDateTime(date);
}

void loop() {
  Serial.println(getDateTime());
  delay(1000);
}


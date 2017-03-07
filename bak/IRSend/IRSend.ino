#include <IRremote.h>
#include <Wire.h>

#define RTC_Address   0x32//时钟器件IIC总线地址

void setup()
{
  Serial.begin(9600);
  Wire.begin();
}

IRsend irsend;
int toggle = 0; // The RC5/6 toggle state
unsigned long repeatTime = millis();
void sendCode(int type, unsigned long value, int bits) {
  if (type == NEC) {
    irsend.sendNEC(value, bits);
    Serial.print("Sent NEC ");
    Serial.println(value, HEX);
  }
  else if (type == SONY) {
    irsend.sendSony(value, bits);
    Serial.print("Sent Sony ");
    Serial.println(value, HEX);
  }
  else if (type == RC5 || type == RC6) {
    if (millis() - repeatTime > 100) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    repeatTime = millis();
    // Put the toggle bit into the code to send
    value = value & ~(1 << (bits - 1));
    value = value | (toggle << (bits - 1));
    if (type == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(value, HEX);
      irsend.sendRC5(value, bits);
    }
    else {
      irsend.sendRC6(value, bits);
      Serial.print("Sent RC6 ");
      Serial.println(value, HEX);
    }
  }
}

void sendCode(unsigned int *rawbuf, int rawlen) {
  irsend.sendRaw(rawbuf, rawlen, 38);
  Serial.println("Sent raw");
}

//从SD2403获取日期时间
unsigned char date[7];
void updateDateTime(void) {
  unsigned char n = 0;
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
String format(char c) {
  return (c < 10 ? "0" : "") + String(int(c));
}
String  getDateTime(void)
{
  return "20" + format(date[6]) + "-" + format(date[5]) + "-" + format(date[4]) + " " + format(date[2]) + ":" + format(date[1]) + ":" + format(date[0]);
}

//unsigned int power1[] = {700 ,950 ,1650 ,1700 ,800 ,900 ,1600 ,1850 ,1500 ,900 ,800 ,900 ,750 ,1750 ,1600 ,900 ,800 ,900 ,750};
//unsigned int power2[] = {700 , 900 , 800 , 900 , 750 , 900 , 800 , 900 , 1600 , 1750 , 1600 , 950 , 700 , 950 , 750 , 1850 , 1500 , 900 , 800 , 900 , 750};

struct stuff {
  int month;
  int day;
  int week;
  int hour;
  int minute;
  int second;
  int type;
  unsigned long value;
  int bits;
};

struct stuff schedule[] = {{ -1, -1, -1, 23, 0, 10, RC5, 0xe88, 12},{ -1, -1, -1, 23, 1, 20, RC5, 0xe84, 12}, { -1, -1, -1, 7, 30, 30, RC5, 0xe86, 12},{ -1, -1, -1, 7, 30, 50, RC5, 0xe84, 12}};


int flag = 0;
const int uptimeInterval = 1000;
unsigned long lastUpdateTime = 0;
void loop() {

  if (Serial.read() != -1) {
    Serial.println(String(lastUpdateTime) + ">" + String(uptimeInterval));
  }
  
  if (millis() - lastUpdateTime > uptimeInterval) {
    updateDateTime();
    Serial.println(getDateTime());
    for (int i = 0; i < 4; i++) {
      if (schedule[i].second == date[0]) {
        for (int j = 0; j < 3; j++) {
          sendCode(schedule[i].type, schedule[i].value, schedule[i].bits);
          delay(40);
        }
      }
    }
    lastUpdateTime = millis();
  }


}

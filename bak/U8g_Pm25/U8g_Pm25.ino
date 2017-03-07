/* PM1.0 PM2.5 PM10
* 激光PM2.5传感器：攀藤 
* Arduino Nano / UNO R3
* SSD1306 I2C
*
* @Author: Z.Z.Y <chowsoym@sohu.com>
*/

#include <U8glib.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

struct {
  int pm10;//显示PM1.0
  int pm25;//显示PM2.5
  int pm100;//显示PM10
} AQI;
//画LOGO
void showLogo(uint8_t x, uint8_t y, char* str)
{
  u8g.drawRFrame(x, y, 30, 16, 1);
  u8g.setFont(u8g_font_orgv01);
  u8g.setPrintPos(x + 3, y + 10);
  u8g.print(str);
}
//画数值
void showValue(uint8_t x, uint8_t y, int value, const u8g_fntpgm_uint8_t *font = u8g_font_unifont)
{
  u8g.setPrintPos(x, y);
  u8g.setFont(font);
  u8g.print(value);
}

void showPM10(void) {
  uint8_t x = 5, y = 2;//PM1.0显示位置
  showLogo(x, y, "pm 1.0");
  showValue(x + 33, y + 13, AQI.pm10);
}
void showPM25(void) {
  uint8_t x = 5, y = 35;//PM2.5显示位置
  showLogo(x, y, "pm2.5");
  u8g.setScale2x2();
  showValue(x / 2 + (AQI.pm25 < 100 ? (AQI.pm25 < 10 ? 32 : 27) : 18), y / 2 + 13, AQI.pm25, u8g_font_courB18);
  u8g.undoScale();
}

void showPM100(void) {
  uint8_t x = 69, y = 2;//PM10显示位置
  showLogo(x, y, "pm 10");
  showValue(x + 33, y + 13, AQI.pm100);
}

void setup() {
  Serial.begin(9600);
  //设置屏幕字体和背景颜色
  u8g.setColorIndex(255);
  u8g.setHiColorByRGB(255, 255, 255);
  delay(800);
}

//画界面
void draw(void) {
  showPM10();
  showPM25();
  showPM100();
}


void loop() {
  //读取传感器
  if (Serial.available()) {
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    //读取传感器原始数据
    Serial.readBytes(buf, len);
    //pm2.5传感器原始数据以42 4d开头  edit
    if(buf[0]==0x42&&buf[1]==0x4d)
    {
      //计算数值
      AQI.pm10 = buf[10]*256+buf[11];
      AQI.pm25 = buf[12]*256+buf[13];
      AQI.pm100 = buf[14]*256+buf[15];
    }
  }
  //显示到屏幕
  u8g.firstPage();
  do
  {
    draw();
  } while ( u8g.nextPage());
  delay(1000);
}

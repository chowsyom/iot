/* PM1.0 PM2.5 PM10
* 激光PM2.5传感器：攀藤 
* Arduino Nano / UNO R3
* SSD1306 I2C
*
* @Author: Z.Z.Y <chowsoym@sohu.com>
*/

#include <U8glib.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
uint8_t CMD[10] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79,0};
unsigned long updateStartTime = 0;
int Co2 = 0;
//画LOGO
void showLogo(uint8_t x, uint8_t y,const char* str)
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

void showCO2(void) {
  uint8_t x = 2, y = 25;//CO2显示位置
  showLogo(x, y, "CO2");
  u8g.setScale2x2();
  showValue(x / 2 + (Co2 < 1000 ? (Co2 < 100 ? 32 : 27) : 18), y / 2 + 10, Co2, u8g_font_courB12);
  u8g.undoScale();
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
  showCO2();

}
void loop() {

  //读取传感器
  if (Serial.available()) {
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    //读取传感器原始数据
    Serial.readBytes(buf, len);
    //传感器原始数据以42 4d开头  edit
    if(buf[0] == 0xFF && buf[1] == 0x86)
    {
      //计算数值
      Co2 = buf[2] * 256 + buf[3];
    }
  }
  if(millis() - updateStartTime > 3000)
  {
     updateStartTime = millis();
     Serial.write(CMD, 10);
  }
  //显示到屏幕
  u8g.firstPage();
  do
  {
    draw();
  } while ( u8g.nextPage());
  delay(1000);
}

/* PM2.5&温湿度显示
* 激光PM2.5传感器：攀藤 G5 PMS5005
* 温湿度传感器：DHT22
* Arduino Nano
* SSD1306
*
* @Author: Coeus <r.anerg at gmail.com>
*/
#include <DHT.h>
#include <U8glib.h>
//#include <SoftwareSerial.h>

#define DHTPIN 3        //DHT SIG口接PIN3
#define DHTTYPE DHT22   //定义DHT型号

//SoftwareSerial mySerial(9, 10);
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
DHT dht(DHTPIN, DHTTYPE);

struct PARAMS {
  float T;//显示温度
  float H;//显示湿度
  long P;//显示PM2.5
} _params;


void setup() {
  Serial.begin(9600);
  //设置屏幕字体和背景颜色
  u8g.setColorIndex(255);
  u8g.setHiColorByRGB(255, 255, 255);
  delay(800);
}

void showTemperature(void) {
  int x = 30, y = 10;//温度显示位置
  //画LOGO
  u8g.drawRFrame(x, y, 16, 16, 1);
  u8g.drawFrame(x + 5, y + 3, 3, 6);
  u8g.drawDisc(x + 6, y + 10, 2);
  u8g.setFont(u8g_font_04b_03b);
  u8g.setPrintPos(x + 10, y + 8);
  u8g.print("C");

  //设置双倍大小字体显示
  u8g.setScale2x2();
  u8g.setPrintPos(x / 2 + 11, y / 2 + 7);
  u8g.setFont(u8g_font_orgv01);
  u8g.print(_params.T);
  u8g.undoScale();
}

void showPM25(void) {
  int x = 30, y = 28;//PM2.5显示位置
  //画LOGO
  u8g.drawRFrame(x, y, 16, 16, 1);
  u8g.setFont(u8g_font_orgv01);
  u8g.setPrintPos(x + 3, y + 10);
  u8g.print("pm");

  //设置双倍大小字体显示
  u8g.setScale2x2();
  u8g.setPrintPos(x / 2 + 11, y / 2 + 7);
  u8g.setFont(u8g_font_orgv01);
  u8g.print(_params.P);
  u8g.undoScale();
}

void showRH() {
  int x = 30, y = 46;//湿度显示位置
  //画LOGO
  u8g.drawRFrame(x, y, 16, 16, 1);
  u8g.drawFrame(x + 5, y + 3, 3, 6);
  u8g.drawDisc(x + 6, y + 10, 2);
  u8g.setFont(u8g_font_04b_03b);
  u8g.setPrintPos(x + 10, y + 8);
  u8g.print("H");

  //设置双倍大小字体显示
  u8g.setScale2x2();
  u8g.setPrintPos(x / 2 + 11, y / 2 + 7);
  u8g.setFont(u8g_font_orgv01);
  u8g.print(_params.H);
  u8g.undoScale();
}


//画界面
void draw(void) {
  showTemperature();
  showPM25();
  showRH();
}

//获取温湿度
void getDHT22() {
  _params.H = dht.readHumidity();
  _params.T = dht.readTemperature();
}

//获取PM2.5的值
void getPM25(unsigned char ucData) {
}

int i = 0;

void loop() {
  i++;
  _params.T = 23.33+i;
  _params.H = 60.56+i;
  _params.P = 15+i;

  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage());
}

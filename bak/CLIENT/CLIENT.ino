#include<SoftwareSerial.h>
#include "DHT.h"

#define SSId       "ES_001"                //type your own SSID name
#define PASSWORD   "go2map123"             //type your own WIFI password

#define SERVER      "192.168.0.108"
#define PORT       80

#define RXPIN 4
#define TXPIN 5
//#define PM25RXPIN 7
//#define PM25TXPIN 6

#define GREENLEDPIN 6
#define DHTPIN 9
//#define DHTPIN2 10

//#define BRIGHTPIN_D 9
//#define BRIGHTPIN_A A3
//#define BRIGHTPIN2 11

//#define GASPIN_D 9
//#define GASPIN_A A3

//#define HBIPIN 11

//#define SOUNDPIN 9
//#define SOUNDPIN_A A3

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

//Communication mode
#define    TCP     1
#define    tcp     1
#define    UDP     0
#define    udp     0

#define DebugBaudRate 115200
#define ESP8266BaudRate 115200


SoftwareSerial softSerial(RXPIN, TXPIN);

#define DEBUG

#ifdef DEBUG
#define DBG(message)    softSerial.print(message)
#define DBGW(message)    softSerial.write(message)
#else
#define DBG(message)
#define DBGW(message)
#endif // DEBUG


//SoftwareSerial pm25Serial(PM25RXPIN, PM25TXPIN);
DHT dht(DHTPIN, DHTTYPE);

String clientId = "0";

unsigned long lastActiveTime = 0;
const unsigned long maxIdleInterval = 60000;
const int resetFlagInterval = 30000;
const int intCmdInterval = 5000;
const int loopCmdInterval = 3000;

#define initBatCmdLen 6
#define loopBatCmdLen 3
String initBatCmd[initBatCmdLen] = {"AT+CWMODE=1", "AT+RST", "AT+GMR", "AT+CWJAP=\"" + String(SSId) + "\",\"" + String(PASSWORD) + "\"", "AT+CIFSR", "AT+CIPMUX=0"}; // "AT+GMR","AT+CWMODE=1","AT+CWMODE?","AT+RST", "AT+CIPMUX=0","AT+CIPSTART=TCP,"+server+","+port  "AT+CIPCLOSE"  "AT+CIPSERVER=1,"+port
String loopBatCmd[loopBatCmdLen] = {"AT+CIPSTART=\"TCP\",\"" + String(SERVER) + "\"," + String(PORT), "", ""};
int initBatCmdIdx = 0;
int loopBatCmdIdx = 0;

const int updateInterval = 0;
unsigned long lastUpdateTime = 0 - 5 - updateInterval;
boolean updateFlag = true;

String softRxMsg = "";
boolean cmdFlag = true;
bool flagLed = true;
int interval = intCmdInterval;
unsigned long startTime = 0 - 5 - interval;
boolean sendReady = false;

boolean isDebug = false;


void sendData(String str)
{
  loopBatCmd[1] = "AT+CIPSEND=" + String(str.length());
  loopBatCmd[2] = str;
}
/*
String getPm25() {
  long  pmcf10 = 0;
  long  pmcf25 = 0;
  long  pmcf100 = 0;
  long  pmat10 = 0;
  long  pmat25 = 0;
  long  pmat100 = 0;
  int count = 0;
  unsigned char c;
  unsigned char high;
  String pmStr = "";
  while (pm25Serial.available()) {
    c = pm25Serial.read();
    if ((count == 0 && c != 0x42) || (count == 1 && c != 0x4d)) {
      //Serial.println("check failed");
      break;
    }
    if (count > 15) {
      //Serial.println("complete");
      break;
    }
    else if (count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) high = c;
    else if (count == 5) {
      pmcf10 = 256 * high + c;
      pmStr += "CF=1, PM1.0=";
      pmStr += String(pmcf10);
      pmStr += " ug/m3";
    }
    else if (count == 7) {
      pmcf25 = 256 * high + c;
      pmStr += "CF=1, PM2.5=";
      pmStr += String(pmcf25);
      pmStr += " ug/m3";
    }
    else if (count == 9) {
      pmcf100 = 256 * high + c;
      pmStr += "CF=1, PM10=";
      pmStr += String(pmcf100);
      pmStr += " ug/m3";
    }
    else if (count == 11) {
      pmat10 = 256 * high + c;
      pmStr += "atmosphere, PM1.0=";
      pmStr += String(pmat10);
      pmStr += " ug/m3";
    }
    else if (count == 13) {
      pmat25 = 256 * high + c;
      pmStr += "atmosphere, PM2.5=";
      pmStr += String(pmat25);
      pmStr += " ug/m3";
    }
    else if (count == 15) {
      pmat100 = 256 * high + c;
      pmStr += "atmosphere, PM10=";
      pmStr += String(pmat100);
      pmStr += " ug/m3";
    }
    count++;
  }
  while (pm25Serial.available()) pm25Serial.read();
  return pmStr;
}
*/

void setup()
{
  Serial.begin(DebugBaudRate);
  softSerial.begin(ESP8266BaudRate);
  softSerial.listen();
  //pm25Serial.begin(9600);
  //pm25Serial.listen();
  pinMode(GREENLEDPIN, OUTPUT);
  dht.begin();
  //dht2.begin();
}
//arduino reset
void(* resetFunc) (void) = 0;

// The loop function is called in an endless loop
void loop()
{
  if (isDebug) {
    DBG("\r\n--1--\r\n" + String(millis() - lastActiveTime)  + ">" + String(maxIdleInterval) + "\r\n");
  }
  //重启arduino
  if (millis() - lastActiveTime > maxIdleInterval * 2) {
    resetFunc();
  }
  //重启ESP8266
  if (millis() - lastActiveTime > maxIdleInterval) {
    cmdFlag = true;
    initBatCmdIdx = 0;
  }
  //重置cmdFlag
  if (millis() - lastActiveTime > resetFlagInterval) {
    cmdFlag = true;
  }

  //调试
  if (softSerial.available() > 0)
  {
    if (softSerial.find("debug") == true) {
      DBG("\r\n------------debug---------\r\n");
      isDebug = true;
    }
  }
  //处理应答
  if (Serial.available() > 0)
  {
    lastActiveTime = millis();
    softRxMsg += (char) Serial.read();
  }
  else if (softRxMsg != "")
  {
    DBG(softRxMsg + "\r\n");
    if (loopBatCmdIdx >= 1 && softRxMsg.indexOf("ERROR") > -1 ) {
      loopBatCmdIdx = 0;
    }

    softRxMsg = "";
    cmdFlag = true;
    startTime = millis();
  }

  if (isDebug) {
    DBG("\r\n--2--\r\n" + String(millis() - lastUpdateTime)  + ">" + String(updateInterval) + "&&" + String(updateFlag) + "\r\n");
  }
  //定时获取数据
  if (millis() - lastUpdateTime > updateInterval && updateFlag == true) {
    updateFlag = false;
    loopBatCmdIdx = 1;

    String data = "";

    //温湿度传感器
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(t) || isnan(h)) {
      DBG("Failed to read from DHT\r\n");
    } else {
      data += String(h) + "%," + String(t) + " *C ";
    }
    //data += getPm25();
    /*
      data += " - ";
      float h2 = dht2.readHumidity();
      float t2 = dht2.readTemperature();
      if (isnan(t2) || isnan(h2)) {
      DBG("Failed to read from DHT2\r\n");
      } else {
      data += String(h2)+"%,"+String(t2)+" *C";
      }*/
    /*
      //亮度传感器
      int bright_D = digitalRead(BRIGHTPIN_D);
      int bright_A = analogRead(BRIGHTPIN_A);
      int bright2 = digitalRead(BRIGHTPIN2);
      data += String(bright_D)+","+String(bright_A)+","+String(bright2);
    */
    /*
      //气体传感器
      int gas_D = digitalRead(GASPIN_D);
      int gas_A = analogRead(GASPIN_A);
      data += String(gas_D)+","+String(gas_A);
    */

    sendData(clientId + (flagLed ? "on" : "off") + " " + data);
    flagLed = !flagLed;
    lastUpdateTime = millis();
  }
  if (isDebug) {
    DBG("\r\n--3--\r\n" + String(millis() - startTime)  + ">" + String(interval) + "&&" + String(cmdFlag) + "\r\n");
    isDebug = false;
  }
  //发送指令
  if (millis() - startTime > interval && cmdFlag == true) {
    cmdFlag = false;
    String AT = "";
    if (initBatCmdIdx < initBatCmdLen) {
      interval = intCmdInterval;
      AT = initBatCmd[initBatCmdIdx];
      initBatCmdIdx++;

    }
    else {
      interval = loopCmdInterval;
      if (loopBatCmdIdx < loopBatCmdLen) {
        lastUpdateTime = millis();
        AT = loopBatCmd[loopBatCmdIdx];
        loopBatCmdIdx++;
      }
      if (loopBatCmdIdx >= loopBatCmdLen) {
        updateFlag = true;
      }
    }
    if (AT != "") {
      DBG("\r\n----------\r\n");
      DBG(AT + "\r\n");
      Serial.println(AT + "\r\n");
    }

  }
}








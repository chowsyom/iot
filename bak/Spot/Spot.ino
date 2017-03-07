#include <SPI.h>
#include<SoftwareSerial.h>
#include <Wire.h>
#include <EEPROM.h>
#include <VirtualWire.h>


#define DISTANCE 1900 //毫米
#define US100_DELAY 1000   //毫秒
#define HCSR501_DELAY 10000   //毫秒

#define ON1_DELAY 10000   //毫秒
#define ON2_DELAY 60000   //毫秒
#define ON3_DELAY 10000   //毫秒

#define RF_TIMEOUT 10000   //毫秒

#define HCSR501_PIN A0
#define INFRARED_SWITCH_PIN A1

#define RX_PIN 3
#define TX_PIN 4

#define ON1_PIN 5
#define ON2_PIN 6
#define ON3_PIN 7
#define ON4_PIN 8
//#define ON5_PIN 9
//10,11,12 保留给RF315M通信


#define INPUT_SIZE 119
#define SERIAL_INPUT_SIZE 254
#define RTC_Address   0x68  //时钟器件IIC总线地址

SoftwareSerial Serial1(RX_PIN, TX_PIN);

uint8_t CLIENT_ID = EEPROM.read(0);
uint8_t timeList[8][5];
struct
{
  uint8_t time[2] = {0xFF, 0xFF};
  bool isOn = false;
} optByCmdTimeList[4];


boolean isON1 = false;
boolean isON2 = false;
boolean isON3 = false;
boolean isON4 = false;
boolean isONByUS100Sensor = false;
boolean isONByHCRS501Sensor = false;
boolean isONByINFRAREDSWITCH = false;

uint8_t onByRMCmd = 0;
uint8_t replyTimes = 0;
unsigned long us100_startTime = 0 - 5 - US100_DELAY;
unsigned long hcsr501_startTime = 0 - 5 - HCSR501_DELAY;
unsigned long on1_startTime = 0 - 5 - ON1_DELAY;
unsigned long on2_startTime = 0 - 5 - ON2_DELAY;
unsigned long on3_startTime = 0 - 5 - ON3_DELAY;


//从SD3231获取日期时间
String format(uint8_t c) {
  return (c < 10 ? "0" : "") + String(int(c));
}
void updateDateTime(uint8_t* date) {
  uint8_t n = 0;
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
String  getDateTime(uint8_t* date)
{
  updateDateTime(date);
  return "20" + format(date[6]) + "-" + format(date[5]) + "-" + format(date[4]) + " " + format(date[2]) + ":" + format(date[1]) + ":" + format(date[0]) + ", " + String(int(date[3]));
}
byte decToBcd(byte val)
{
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}
//设置SD3231时间
void setDateTime(uint8_t* date)
{
  Wire.beginTransmission(RTC_Address);
  Wire.write(uint8_t(0x00));
  uint8_t i;
  for (i = 0; i < 7; i++)
  {
    uint8_t val = decToBcd(date[i]);
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
  uint8_t ctrl = Wire.read();
  Wire.write(uint8_t(0x0f));
  Wire.write(ctrl & 0b01111111);
  Wire.endTransmission();
}

//获取超声波测量的距离
void getDistance(unsigned int *Len_mm)
{
  uint8_t HighLen = 0;
  uint8_t LowLen  = 0;
  Serial1.flush();     // 清空串口接收缓冲区
  Serial1.write(0X55); // 发送0X55，触发US-100开始测距
  delay(100);          //延时100毫秒
  if (Serial1.available() >= 2)           //当串口接收缓冲区中数据大于2字节
  {
    HighLen = Serial1.read();                   //距离的高字节
    LowLen  = Serial1.read();                   //距离的低字节
    *Len_mm  = HighLen * 256 + LowLen;           //计算距离值
    //    Serial.print("Present distance is: ");   //输出结果至串口监视器
    //    Serial.print(Len_mm, DEC);            //输出结果至串口监视器
    //    Serial.println("mm");                  //输出结果至串口监视器
  }
}


//更新定时列表
void updateTimeList(char* input, uint8_t* timeArray, uint8_t* len)
{
  // Read each command pair
  char* timeStr = strtok(input, ";");
  while (timeStr != 0)
  {
    // Split the timeStr in two part
    char* colon = strchr(timeStr, ':');
    if (colon != 0)
    {
      // Actually split the string in 2: replace '-' or ':' with 0
      *colon = 0;
      uint8_t startHour = (uint8_t) atoi(timeStr);

      //Serial.print("startHour=");
      //Serial.println(startHour, DEC);
      (*len)++;

      if (0 <= startHour && startHour < 24)
        *timeArray = startHour;
      timeArray++;
      //EEPROM.write(addr, startHour);
      ++colon;

      char* hyphen = strchr(colon, '-');
      if (hyphen != 0)
      {
        *hyphen = 0;
        uint8_t startMin = (uint8_t) atoi(colon);
        //Serial.print("startMin=");
        //Serial.println(startMin, DEC);
        (*len)++;
        if (0 <= startMin && startMin < 60)
          *timeArray = startMin;
        timeArray++;
        //EEPROM.write(addr, startMin);
        ++hyphen;
        colon = strchr(hyphen, ':');
        if (colon != 0)
        {
          *colon = 0;
          uint8_t endHour = (uint8_t) atoi(hyphen);
          (*len)++;
          //Serial.print("endHour=");
          //Serial.println(endHour, DEC);
          if (0 <= endHour && endHour < 24)
            *timeArray = endHour;
          timeArray++;
          //EEPROM.write(addr, endHour);
          ++colon;
          char* comma = strchr(colon, ',');
          if (comma != 0)
          {
            *comma = 0;
            uint8_t endMin = (uint8_t) atoi(colon);
            (*len)++;
            //Serial.print("endMin=");
            //Serial.println(endMin, DEC);
            if (0 <= endMin && endMin < 60)
              *timeArray = endMin;
            timeArray++;
            //EEPROM.write(addr, endMin);
            ++comma;
            uint8_t weeks = (uint8_t) (atoi(comma) & 0x7F);
            (*len)++;
            //Serial.print(",weeks=");
            //Serial.println(weeks, DEC);
            *timeArray = weeks;
            timeArray++;
            //EEPROM.write(addr, weeks);
          }
        }
      }
    }
    // Find the next command in input string
    timeStr = strtok(0, ";");
  }
  //updateTimeListFromRom();
}


//检查定时开关
boolean checkTimeList(uint8_t addr1, uint8_t addr2, uint8_t RelayId) {
  bool isOn = false;
  bool isInTimes = false;
  uint8_t i;
  uint8_t date[7];
  updateDateTime(date);
  for (i = addr1; i < addr2; i++)
  {
    int startTime = timeList[i][0] * 60 + timeList[i][1];
    int endTime =  timeList[i][2] * 60 + timeList[i][3];
    int curTime = date[2] * 60 + date[1];
    if (date[3] < 1 || date[3] > 127) date[3] = 0x7F;
    uint8_t pos;
    if (endTime < startTime) {
      if (curTime < endTime)
      {
        startTime -= 24 * 60;
        pos = (7 + date[3] - 2) % 7;
      }
      else
      {
        endTime += 24 * 60;
        pos = (7 + date[3] - 1) % 7;
      }
    }
    if (((timeList[i][4] >> pos) & 0x01) && startTime <= curTime  && curTime < endTime)
    {
      isOn = true;
      isInTimes = true;
      //Serial.println(format(timeList[i][0]) + ":" + format(timeList[i][1]) + "-" + format(timeList[i][2]) + ":" + format(timeList[i][3]) + "," + format(timeList[i][4]) + " -> turn on");
      //Serial.println("turned on by timer.");
      int offByCmdTime = -1;
      if (optByCmdTimeList[RelayId].time[0] != 0xFF)
      {
        offByCmdTime = optByCmdTimeList[RelayId].time[0] * 60 + optByCmdTimeList[RelayId].time[1];
      }
      if (optByCmdTimeList[RelayId].isOn)
      {
        optByCmdTimeList[RelayId].time[0] = 0xFF;
      }
      else if (offByCmdTime > -1)
      {
        if (startTime <= offByCmdTime  && offByCmdTime < endTime)
        {
          isOn = false;
          //Serial.println("turned off by remote command.");
        }
      }
      break;
    }
  }
  if (!isInTimes)
    if (!optByCmdTimeList[RelayId].isOn) optByCmdTimeList[RelayId].time[0] = 0xFF;
    else if (optByCmdTimeList[RelayId].time[0] != 0xFF) isOn = true;
  return isOn;
}
//int getValidEndTime(uint8_t addr1, uint8_t addr2, int checkTime, uint8_t checkDow)
//{
//  int validEndTime;
//  int validEndTimes[addr2 - addr1];
//  uint8_t i, j = 0;
//  for (i = addr1; i < addr2; i++)
//  {
//    int startTime = timeList[i][0] * 60 + timeList[i][1];
//    int endTime =  timeList[i][2] * 60 + timeList[i][3];
//    uint8_t dows = timeList[i][3];
//    if (checkDow < 1 || checkDow > 127) checkDow = 0x7F;
//    uint8_t pos = (7 + checkDow - (endTime < startTime ? 2 : 1)) % 7;
//    if (endTime < startTime && checkTime >= startTime) endTime += 24 * 60;
//    if ((dows >> pos) & 0x01)
//    {
//      validEndTimes[j++] = endTime;
//    }
//  }
//  quickSort(validEndTimes, 0, j);
//  validEndTime = validEndTimes[0];
//  for (i = 0; i < j; i++)
//  {
//    if (validEndTimes[i] > checkTime)
//    {
//      validEndTime = validEndTimes[i];
//      break;
//    }
//  }
//  return validEndTime;
//}
////快速排序
//void quickSort(int *a, uint8_t i, uint8_t j)
//{
//  uint8_t m, n, temp;
//  uint8_t k;
//  m = i;
//  n = j;
//  k = a[(i + j) / 2]; /*选取的参照*/
//  do {
//    while (a[m] < k && m < j) m++; /* 从左到右找比k大的元素*/
//    while (a[n] > k && n > i) n--; /* 从右到左找比k小的元素*/
//    if (m <= n) { /*若找到且满足条件，则交换*/
//      temp = a[m];
//      a[m] = a[n];
//      a[n] = temp;
//      m++;
//      n--;
//    }
//  } while (m <= n);
//  if (m < j) quickSort(a, m, j); /*运用递归*/
//  if (n > i) quickSort(a, i, n);
//}
//boolean checkTimeIn(int startTime, int endTime, uint8_t dows, int checkTime, uint8_t checkDow)
//{
//  bool isOn = false;
//  if (checkDow < 1 || checkDow > 127) checkDow = 0x7F;
//  uint8_t pos = (7 + checkDow - 1) % 7;
//  if (endTime < startTime) {
//    if (checkTime >= startTime) endTime += 24 * 60;
//    else if (checkTime <= endTime) {
//      startTime -= 24 * 60;
//      pos = (7 + checkDow - 2) % 7;
//    }
//  }
//  if (((dows >> pos) & 0x01) && startTime <= checkTime  && checkTime < endTime)
//  {
//    isOn = true;
//  }
//  return isOn;
//}

//从ROM读取时间列表
void updateTimeListFromRom(void) {

  if (EEPROM.read(1) < 255) {
    uint8_t addr;
    for (addr = 0; addr < 40; addr++) {
      uint8_t val = EEPROM.read(addr + 1);
      if (val == 255) val = 0;
      uint8_t i = addr / 5;
      uint8_t j = addr % 5;
      if (j == 4) val = val & 0x7F;
      else if (j % 2 == 0) val = val % 24;
      else val = val % 60;
      timeList[i][j] = val;
      //Serial.println("EEPROM[" + String(addr) + "]=" + val);
    }
    uint8_t i;
    for (i = 0; i < 8; i++) {
      Serial.print(format(timeList[i][0]));
      Serial.print(":");
      Serial.print(format(timeList[i][1]));
      Serial.print("-");
      Serial.print(format(timeList[i][2]));
      Serial.print(":");
      Serial.print(format(timeList[i][3]));
      Serial.print(",");
      Serial.println(format(timeList[i][4]));
    }
  }
}


//用RF315MHz广播信息
uint8_t* msg_cache;
uint8_t sendedSize = 0;
uint8_t receiveSize = 0;
uint8_t receiveBuf[VW_MAX_MESSAGE_LEN];
unsigned long rf_startTime;
bool replyEnable = false;
bool isLocalSetting = false;
void sendMsg(void)
{
  sendMsg(msg_cache, sendedSize);
}
void sendMsg(char* msg)
{
  sendMsg((uint8_t *)msg, strlen(msg));
}
void sendMsg(uint8_t* msg, uint8_t len)
{
  replyEnable = false;
  isLocalSetting = false;
  if (msg[2] == CLIENT_ID)
  {
    uint8_t i;
    for (i = 0; i < len; i++)
    {
      receiveBuf[i] = msg[i];
    }
    receiveSize = len;
    isLocalSetting = true;
  }
  else
  {
    msg_cache = msg;
    sendedSize = len;
    vw_send(msg, len);
    vw_wait_tx(); // Wait until the whole message is gone
    Serial.print("Message sended ");
    printString(msg, len);
    rf_startTime = millis();
    if (msg[4] != 0xFD)
    {
      replyEnable = true; //0xFE|0x7C|0x01|0xFF|0x01|0x01~0xFE
    }
  }
}

//用RF315MHz接收信息
boolean receiveMsg(void)
{
  boolean flag = false;
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (isLocalSetting)
  {
    flag = true;
    isLocalSetting = false;
    Serial.println("Got a local setting.");
  }
  else if (vw_get_message(receiveBuf, &buflen)) // Non-blocking
  {
    int i;
    receiveBuf[buflen] = 0;
    Serial.print("Got ");
    printString(receiveBuf, buflen);
    receiveSize = buflen;
    flag = true;
  }
  return flag;
}
void printString(uint8_t* str, uint8_t len)
{
  Serial.print("[");
  Serial.print(len, DEC);
  Serial.print(" bytes]:[");
  uint8_t *p;
  p = str;
  while (*p != 0)
  {
    Serial.print(*p, HEX);
    p++;
    Serial.print(" ");
  }
  Serial.println("]");
}

//超时重发
void checkMsgTimeout(void)
{
  if (replyTimes < 3)
  {
    if (millis() - rf_startTime > RF_TIMEOUT && replyEnable)
    {
      sendMsg();
      replyTimes++;
    }
  }
  else
  {
    replyEnable = false;
    replyTimes = 0;
    Serial.println("Message sended fail.");
  }
}

uint8_t retMsg[7] = {0xFE, 0x7C, 0, CLIENT_ID, 0xFD, 0, 0};
//处理接收到的指令
void handleCommand(void)
{
  //解析指令
  //0xFE|0x7C|接收端设备号(0xFF全部)|发送端设备号|指令/状态编号(0xFD)|指令值(0xFF=0)
  //指令/状态编号 : 0x01=设置继电器状态,0x02=,0x03=设置系统时间,0x04=设置定时列表,0x05=红外释热传感器,0x06=温湿度,0x07=PM2.5,0x08=CO2,0xFD=回复收到指令长度
  //0xFE|0x7C|0x01|0xFF|0x01|0x01~0xFE
  //0xFE|0x7C|0x01|0xFF|0x02|0x01~0xFE
  //0xFE|0x7C|0x01|0xFF|0x03|0xFF 0x2F 0x0E 0x04 0x16 0x09 0x10 //秒,分,时,周,日,月,年 2016-09-22 w4 14:47:00
  //0xFE|0x7C|0x01|0xFF|0x04|0x01,0x0F,0x1F,0x0F,0x2F,...    //0x04设置定时 week1(1111111),hour11,minute11,hour12,minute12,week2(1111111),hour21,minute21,hour22,minute22,...,

  //FE 7C 01 01 01 0F
  //FE 7C 01 01 03 FF 2F 0E 08 16 09 10
  //FE 7C 01 01 04 08:00-10:00,01;00:00-00:00,02;00:00-00:00,03;00:00-00:00,04;08:00-09:00,05;17:00-18:00,06;21:00-22:00,07;22:00-19:30,04
  if (receiveSize > 5 && receiveBuf[0] == 0xFE && receiveBuf[1] == 0x7C) //以0xFE, 0x7C开头
  {
    if (receiveBuf[2] == CLIENT_ID)  //匹配设备号
    {
      //匹配指令
      if (receiveBuf[4] != 0xFD && receiveBuf[3] != CLIENT_ID)
      {
        //回复发送端收到指令的长度
        retMsg[2] = receiveBuf[3];
        retMsg[5] = receiveSize;
        sendMsg(retMsg, 7);
      }
      uint8_t i;
      uint8_t date[7];
      uint8_t* addr;
      addr = receiveBuf + 5;
      switch (receiveBuf[4])
      {
        case 0xFD:
          //判断回复收到指令长度是否正确
          if (receiveBuf[5] != sendedSize) sendMsg();
          else {
            replyEnable = false;
            replyTimes = 0;
            Serial.println("Message sended ok.");
          }
          break;
        case 0x01:
          //设置设备编号
          CLIENT_ID = *addr;
          EEPROM.write(0, *addr);
          Serial.print("Updata this terminal ID to No.");
          Serial.println(*addr, DEC);
          break;
        case 0x02:
          //设置继电器状态
          updateDateTime(date);
          onByRMCmd = *addr == 0xFF ? 0 : *addr;
          for (i = 0; i < 4; i++)
          {
            bool isON = (onByRMCmd >> i) & 0x01;
            optByCmdTimeList[i].time[0] = date[2];
            optByCmdTimeList[i].time[1] = date[1];
            optByCmdTimeList[i].isOn = isON;
          }
          //Serial.print("Relays state changed > ");
          //Serial.println(onByRMCmd, BIN);
          break;
        case 0x03:
          //更新系统时间
          i = 0;
          while (*addr != 0 && i < 7)
          {
            //Serial.print(*addr==0xFF?0:*addr, DEC);
            //Serial.print(" ");
            date[i] = *addr == 0xFF ? 0 : *addr;
            addr++;
            i++;
          }
          setDateTime(date);//uint8_t date[7];
          Serial.print("Update system clock:");
          Serial.println(getDateTime(date));//data值已被改变
          break;
        case 0x04:
          //更新定时时间列表
          i = 1;
          while (*addr != 0)
          {
            EEPROM.write(i, *addr == 0xFF ? 0 : *addr);
            //Serial.print(*addr==0xFF?0:*addr, DEC);
            //Serial.print(" ");
            addr++;
            i++;
          }
          Serial.println("Update time list:");
          updateTimeListFromRom();
          break;
        case 0x05:
          break;
        case 0x06:
          break;
        case 0x07:
          break;
        case 0x08:
          break;
      }
    }
  }
}

void handleSerialInputMsg(uint8_t* input)
{
  //处理串口输入的指令
  char serialInput[SERIAL_INPUT_SIZE + 1];
  byte inputSize = Serial.readBytes(serialInput, SERIAL_INPUT_SIZE);
  if (inputSize > 10) {
    //Serial.print("inputSize=");
    //Serial.println(inputSize, DEC);
    serialInput[inputSize] = 0;// Add the final 0 to end the C string
    char* serialInputPart = strtok(serialInput, " ");
    uint8_t i = 0;
    while (serialInputPart != 0)
    {
      uint8_t len = strlen(serialInputPart);
      if (len == 2)
      {
        char* stopstring;
        input[i++] = (uint8_t) strtol(serialInputPart, &stopstring, 16);
      }
      else break;
      serialInputPart = strtok(0, " ");
    }
    inputSize = i;
    //预处理输入的指令 0xFE|0x7C|0x01|0xFF|0x03|0x0F,0x09,0x13,0x02,0x0F,0x1F,0x2F
    if (input[0] == 0xFE && input[1] == 0x7C)
    {
      uint8_t len = 0;
      uint8_t j;
      if (input[4] == 0x04 && serialInputPart != 0)
      {
        updateTimeList(serialInputPart, input + 5, &len);
        inputSize = len + 5;
      }
      input[3] = CLIENT_ID;
      input[inputSize] = 0;
      //Serial.print("serial input[");
      //Serial.print(inputSize, DEC);
      //Serial.print("]=[");
      uint8_t k;
      for (k = 0; k < inputSize; k++)
      {
        //Serial.print(input[k], k < 5 ? HEX : DEC);
        //Serial.print(" ");
        if (input[k] == 0) input[k] = 0xFF;
      }
      //Serial.println("]");
      sendMsg(input, inputSize);
    }

  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Wire.begin();
  pinMode(HCSR501_PIN, INPUT);
  pinMode(INFRARED_SWITCH_PIN, INPUT);
  pinMode(ON1_PIN, OUTPUT);
  pinMode(ON2_PIN, OUTPUT);
  pinMode(ON3_PIN, OUTPUT);
  pinMode(ON4_PIN, OUTPUT);
  //pinMode(ON5_PIN, OUTPUT);
  digitalWrite(ON1_PIN, HIGH);
  digitalWrite(ON2_PIN, HIGH);
  digitalWrite(ON3_PIN, HIGH);
  digitalWrite(ON4_PIN, HIGH);
  //digitalWrite(ON5_PIN, HIGH);
  Serial.print("Hi! This is Miko No.");
  Serial.println(CLIENT_ID, DEC);
  uint8_t date[7];
  Serial.println(getDateTime(date));
  updateTimeListFromRom();
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);  // Bits per sec
  vw_rx_start();       // Start the receiver PLL running

}

//String timeStr = "11:10-12:20,13:30-14:40,15:60-16:00,17:01-18:32";

void loop() {

  uint8_t input[INPUT_SIZE + 1];
  //处理串口输入的信息
  handleSerialInputMsg(input);

  //如果收到远程指令，处理指令
  if (receiveMsg()) handleCommand();
  else checkMsgTimeout();

  //uint8_t date[7];
  //Serial.println(getDateTime(date));
  boolean isONByTimer3 = checkTimeList(0, 4, 2);
  boolean isONByTimer4 = checkTimeList(4, 8, 3);

  //读取HC-RS501释热传感器状态
  if (millis() - hcsr501_startTime > HCSR501_DELAY) {
    isONByHCRS501Sensor = analogRead(HCSR501_PIN) > 512;
    if (isONByHCRS501Sensor) {
      hcsr501_startTime = millis();
    }
  }
  unsigned int Len_mm  = 0;
  //获取US-100传感器测距数据
  if (millis() - us100_startTime > US100_DELAY) {
    getDistance(&Len_mm);
    isONByUS100Sensor = Len_mm < DISTANCE;
    if (isONByUS100Sensor) {
      us100_startTime = millis();
    }
  }
  //红外镜面反射光电开关infraredSwitch
  isONByINFRAREDSWITCH = analogRead(INFRARED_SWITCH_PIN) > 512;


  //更新继电器状态
  //继电器1
  if (millis() - on1_startTime > ON1_DELAY && isON1 != (isONByHCRS501Sensor || isONByINFRAREDSWITCH || (onByRMCmd & 0x01))) {
    isON1 = isONByHCRS501Sensor || isONByINFRAREDSWITCH || (onByRMCmd & 0x01);
    digitalWrite(ON1_PIN, !isON1);
  }
  if (isON1 && isONByHCRS501Sensor) on1_startTime = millis();
  //继电器2
  if (millis() - on2_startTime > ON2_DELAY && isON2 != (isONByHCRS501Sensor || isONByINFRAREDSWITCH || (onByRMCmd & 0x02))) {
    isON2 = isONByHCRS501Sensor || isONByINFRAREDSWITCH || (onByRMCmd & 0x02);
    digitalWrite(ON2_PIN, !isON2);
  }
  if (isON2 && isONByHCRS501Sensor) on2_startTime = millis();
  //继电器3
  if (millis() - on3_startTime > ON3_DELAY && isON3 != (isONByTimer3 || isONByUS100Sensor || isONByINFRAREDSWITCH)) {
    isON3 = isONByTimer3 || isONByUS100Sensor || isONByINFRAREDSWITCH;
    digitalWrite(ON3_PIN, !isON3);
  }
  if (isON3 && isONByUS100Sensor) on3_startTime = millis();
  //继电器4
  if (isON4 != isONByTimer4) {
    isON4 = isONByTimer4;// || (onByRMCmd & 0x08);
    digitalWrite(ON4_PIN, !isON4);
  }



}

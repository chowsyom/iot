#include <SPI.h>
#include <Ethernet.h>
//#include <SD.h>
#include <Wire.h>

#define RTC_Address   0x68  //0x32//时钟器件IIC总线地址

const int  red = 9;       //设置输出端口
const int  blue = 3;       //设置输出端口
int red_color, asc; //定义各颜色的PWM值参数
bool flag = false;
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
//服务端IP地址
IPAddress ip(192, 168, 0, 108);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

//SD卡
const int chipSelect = 4;

bool isSDReady = false;
bool isDebug = false;


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
  return "20" + format(date[6]) + "-" + format(date[5]) + "-" + format(date[4]) + " " + format(date[2]) + ":" + format(date[1]) + ":" + format(date[0]);
}


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Wire.begin();
  //初始化SD卡
  /*Serial.print("\nInitializing SD card...");
    if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    isSDReady = false;
    } else {
    Serial.println("Wiring is correct and a card is present.");
    isSDReady = true;
    }*/
  /*
    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    isSDReady = false;
    }
    else {
    Serial.println("initialization done.");
    isSDReady = true;
    }*/

  //初始化以太网
  Ethernet.begin(mac, ip);
  server.begin();
  pinMode(red, OUTPUT);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

//arduino reset
void(* resetFunc) (void) = 0;

void Trim(String data) {
  char head[4] = {0x0D, 0x0A};
  char tail[7] = {0x0D, 0x0A, 0x0D, 0x0A};
  data.replace(tail, "");
  data.replace(head, "");
}

void loop() {

  //调试
  if (Serial.available() > 0)
  {
    if (Serial.find("debug") == true) {
      Serial.println("\r\n------------debug---------\r\n");
      //Serial.println("\r\n------------"+getDateTime()+"---------\r\n");
      resetFunc();
      isDebug = true;
    }
  }

  // listen for incoming clients
  EthernetClient client = server.available();
  if (isDebug) {
    Serial.println("client=" + String(client));
    isDebug = false;
  }
  if (client) {
    Serial.println("new client " + getDateTime());
    String data = "";
    int ledPin = red;

    int timeout = millis() + 20000;
    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    //当有网络连接
    while (client.connected()) {
      while (client.available()) {
        char c = client.read();
        Serial.write(c);
        data += c;
      }
      if (data != "") {
        Trim(data);
        /*
          if (isSDReady) {
          File sdFile = SD.open("log.txt", FILE_WRITE);
          Serial.println("\nsdFile="+String(sdFile));
          if (sdFile) {
          Serial.println("\n------3--------");
          Serial.println("\ndate=2="+getDateTime());
            sdFile.println("::" + data + "\n");
            sdFile.close();
          }
          else {
            Serial.println("error opening file.");
          }
          if (data.indexOf("cleanlog") > -1) {
            SD.remove("log.txt");
            Serial.println("log cleaned done.");
          }
          }*/
        if (data.substring(0, 1) == "1") {
          ledPin = red;
        }
        else if (data.substring(0, 1) == "2") {
          ledPin = blue;
        }
        //处理请求
        if (data.indexOf("on") > -1) {
          red_color = 240;
          flag = true;
        }
        if (data.indexOf("off") > -1) {
          red_color = 0;
          flag = true;
        }
        if (flag) {
          //client.flush();
          analogWrite(ledPin, red_color);
          //应答
          client.println(String(ledPin) + " SERVER RESPONSE " + getDateTime());
          flag = false;

        }
        break;
      }
    }
    
    // give the client time to receive the data
    delay(1);
    //client.flush();
    // close the connection:
    //client.stop();
    Serial.println("\n----------------------");
  }

}


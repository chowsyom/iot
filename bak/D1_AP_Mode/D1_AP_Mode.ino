
 /*
日期:2016.8.29
IDE 版本:1.0.0，D1 mini
功能：主卧卫生间综合控制，Kitchen_Master，NTP时间获取，分时段自动控制
硬件：装有温湿度传感器
      4个继电器控制顶灯、电风扇、排气扇和吊柜底灯
      GPIO16用于deepSleep，接RST      
*/

#include "PietteTech_DHT.h"     //温湿度库
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define MAX_SRV_CLIENTS 3   //最大同时联接数，即你想要接入的设备数量，8266tcpserver只能接入五个，哎

#define ssid        "自家的wifi“
#define password    "密码”

const char *ap_ssid = "Kitchen_Master";//AP的ssid
const char *ap_password = "eutriv03";//AP的密码

//数字输入管脚定义
//#define Body_Sensor_pin 4  //人体红外感应脚定义
#define DHTTYPE  DHT22       // 传感器型号 DHT11/21/22/AM2301/AM2302
#define DHTPIN   4           //温度、湿度管脚定义
//declaration
void dht_wrapper(); // must be declared before the lib initialization

//模拟输入管脚定义
//#define LightPIN A0  //光敏管脚定义，模拟输入

//数字输出管脚定义
byte top_led = 12;  //顶灯脚
byte e_fan = 14;    //电风扇脚
byte pai_fan = 13;  //排气扇脚，蜂鸣器
byte mid_led = 5;   //吊柜底灯
byte board_led = 2; //板载灯脚

//定义变量
//byte DS1 = 3;       //Slave_1，超声波测定区域，赋初值防止误操作
//byte DS2 = 3;       //Slave_2，超声波测定区域，赋初值防止误操作
byte DS3 = 3;       //Slave_3，超声波测定区域，赋初值防止误操作
//bool Fire_val = 1;  //火焰值，0为有火，1为无火
float Temp_val;     //温度值
float Humi_val;     //湿度值
bool Body_val_1;    //Slave_3人体红外感应状态值，0为没人，1为有人
//bool Body_val_2;    //Slave_3人体红外感应状态值，0为没人，1为有人
byte Light_val;     //光线值
unsigned long Last_time;//最后一次检测到房间有人的时刻
//long Interval = 30000;//间隔时长，设为30秒
long Interval = 180000;//间隔时长，设为3分钟
unsigned long receive_time;//检测到数据的时刻

bool top_led_sta;       //顶灯状态，1为点亮，0为熄灭
bool mid_led_sta;       //厨房吊柜底灯状态，1为点亮，0为熄灭
bool e_fan_sta;         //电风扇状态，1为开启，0为关闭
bool e_fan_enable;      //电风扇冬季不开，夏季高于某度或湿度超过某值可开；1为可开，0为不可开
bool pai_fan_sta;       //排气扇状态，1为开启，0为关闭
bool pai_hand_enable;   //手势控制排气扇，1为开启，0为关闭
bool e_hand_enable ;    //手势控制电风扇，1为开启，0为关闭
bool cover_sta ;        //马桶盖状态，1为打开，0为关着
bool Auto_ctrl_enable = 1 ;  //自动控制状态，1为开启，0为关闭

const byte temp_gate1 = 28;  //电风扇开启条件1，温度阀值1设定,要高于温度阀值2，单位：度
const byte temp_gate2 = 25;  //电风扇开启条件2，温度阀值2设定,单位：度
const byte humi_gate = 70;   //电风扇开启条件3，湿度阀值设定,单位：%
const byte Light_gate = 900 / 4;  //光线阀值，超过900为光线不足，低于则为明亮
unsigned int localPort = 2390;      // 本地端口，侦听UDP packets

 IPAddress ip(192,168,43,1);
 IPAddress gateway(192,168,43,1);
 IPAddress subnet(255,255,255,0);
IPAddress staticIP(192,168,33,120);//设置静态IP地址
/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
IPAddress timeServerIP;         //时间服务器动态IP地址
const char* ntpServerName = "time.nist.gov";//时间服务器域名
//const char* ntpServerName = "ntp.sjtu.edu.cn";//时间服务器域名
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
unsigned long NTPtime;          //获取的NTP时间
unsigned long nowtime;          //实时时间
unsigned long savetime;         //获取NTP时间时，对应的系统时间（millis)
unsigned long fetchtime = 21600; //重新获取（校正）NTP时间的时刻，定于每天6点（6 X 3600）
unsigned long Autotime = 25200; //自动控制开启时刻，定于每天7点（7 X 3600），到0点关闭
bool NTP_got = 0;               //成功获取NTP时间，1为成功，0为失败
int k;                          //累计数，达到240，即60秒

//创建对象
WiFiServer server(8266);//你要设置的TCPserver端口号，范围0-65535
WiFiClient serverClients[MAX_SRV_CLIENTS];
WiFiUDP udp;
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

void setup() {

    Serial.begin(115200);
    Serial.println("Kitchen Master D1 mini Test");

    pinMode(DHTPIN,INPUT); //设置温湿度接口为输入状态
  //pinMode(FirePIN,INPUT); //设置火焰模块接口为输入状态

    pinMode(mid_led,OUTPUT); //设置吊柜底灯接口为输出状态
    pinMode(top_led,OUTPUT); //设置顶灯接口为输出状态
    pinMode(pai_fan,OUTPUT); //设置排气扇接口为输出状态，用蜂鸣器表示
    pinMode(e_fan,OUTPUT); //设置电扇接口为输出状态
    pinMode(board_led,OUTPUT); //设置板载灯为输出状态
    
    digitalWrite(mid_led,LOW);//初始化
    digitalWrite(top_led,LOW);//初始化
    digitalWrite(pai_fan,LOW);//初始化
    digitalWrite(e_fan,LOW);//初始化
    digitalWrite(board_led,LOW);//初始化
    
    Serial.print("Connecting to ");//设置并连接wifi
    Serial.println(ssid);
    //WiFi.config(staticIP);
    WiFi.begin(ssid, password);
 
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");//如果没有连通向串口发送.....
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    ip = WiFi.localIP();
    Serial.println(WiFi.localIP());
     // print your subnet mask:
     subnet = WiFi.subnetMask();
     Serial.print("NETMASK: ");
     Serial.println(subnet);
  // print your gateway address:
    gateway = WiFi.gatewayIP();
    Serial.print("GATEWAY: ");
    Serial.println(gateway);
    Serial.print("Channel: ");
    Serial.println(WiFi.channel());
    
    //设置AP
    Serial.println();
    Serial.print("Configuring access point...\r\n");
    /* You can remove the password parameter if you want the AP to be open. */
    //WiFi.softAPConfig(gateway,gateway,subnet);
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
/*
    WiFi.softAPConfig(ip,gateway,subnet);
    myIP = WiFi.softAPIP();
    Serial.print("AP IP address(after seting): ");
    Serial.println(myIP);
*/
    WiFi.mode(WIFI_AP_STA);
    server.begin();
    server.setNoDelay(true);  //加上后才正常些
}

void loop() { 

    blink();//板载灯闪烁
    
    //作为服务器进行处理
    uint8_t rx_data[10] = {0};//接收数据存储，[0]作为发送端标识，[1]作为检测信号是否连续，之后才是数据
    uint8_t i;
    uint8_t tmp = 0;
    if (server.hasClient())
    {
        for (i = 0; i < MAX_SRV_CLIENTS; i++)
        {
            if (!serverClients[i] || !serverClients[i].connected())
            {
                if (serverClients[i]) serverClients[i].stop();//未联接,就释放
                serverClients[i] = server.available();//分配新的
                continue;
            }
 
        }
        WiFiClient serverClient = server.available();
        serverClient.stop();
    }
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
    {
        if (serverClients[i] && serverClients[i].connected())
        {
            digitalWrite(board_led, 0);//有链接存在,就一直长亮
 
            if (serverClients[i].available())
            {
                int j = 0;
                while (serverClients[i].available()) 
                    {
                    rx_data[j] = serverClients[i].read();
                    //Serial.write(serverClients[i].read());//输出到串口
                    //Serial.println(rx_data[j]);
                    j ++;
                    }
                    tmp = 1;
                    //serverClients[i].write("yes", 3);//响应内容
            }
        }
    }
}

void blink()
{
    static long previousMillis = 0;
    static int currstate = 0;
 
    if (millis() - previousMillis > 500)  //200ms
    {
        previousMillis = millis();
        currstate = 1 - currstate;
        digitalWrite(board_led, currstate);
    }
}

void dht_wrapper() {
    DHT.isrCallback();
}


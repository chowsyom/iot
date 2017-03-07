#include <ESP8266WiFi.h>

#define led 2 //发光二极管连接在8266的GPIO2上
const char *ssid     = "ES_001";//这里写入网络的ssid
const char *password = "go2map123";//wifi密码

WiFiServer server(8266);//端口号，随意修改，范围0-65535
WiFiClient serverClient;

void setup()
{
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(2, HIGH);
  delay(10);
  //Serial.println();
  //Serial.print("Connecting to ");//会通过usb转tll模块发送到电脑，通过ide集成的串口监视器可以获取数据。
  //Serial.println(ssid);
  WiFi.begin(ssid, password);//启动
  //在这里检测是否成功连接到目标网络，未连接则阻塞。
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  server.begin();
  //server.setNoDelay(true);  //加上后才正常些

  //几句提示
  //Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
unsigned long resetStartTime = 0;
unsigned long uploadStartTime = 0;
bool resetFlag = true;
void loop()
{
  if (server.hasClient())
  {
    if (!serverClient || !serverClient.connected())
    {
      if (serverClient) serverClient.stop();//未联接,就释放
      serverClient = server.available();//分配新的
    }
  }
  if (serverClient && serverClient.connected())
  {
    if (serverClient.available())
    {
      size_t rlen = serverClient.available();
      byte rbuf[rlen];
      byte *pr;
      pr = rbuf;
      while (serverClient.available()) {
        byte b = serverClient.read();
        *pr = b;
        pr++;
      }
      Serial.write(rbuf, rlen);
      if (resetFlag)
      {
        resetFlag = false;
        resetStartTime = millis();
        delay(10);
        digitalWrite(2, LOW);
        uploadStartTime = millis();
      }
    }
  }
  if (millis() - uploadStartTime > 100)
  {
    digitalWrite(2, HIGH);
  }
  if (millis() - resetStartTime > 30000)
  {
    //resetFlag = true;
  }
  if (Serial.available())
  {
    size_t len = Serial.available();
    byte sbuf[len];
    Serial.readBytes(sbuf, len);
    if (serverClient && serverClient.connected())
    {
      serverClient.write(sbuf, len);
    }
  }
}

#include <ESP8266WiFi.h>
#define MAX_SRV_CLIENTS 2



const char* ssid = "ZZY-2.4G";
const char* password = "Auglx19780712";



char sensor_info[50];
unsigned long updateStartTime = 0;
WiFiServer server(8888);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21){
    Serial.print("Could not connect to");
    Serial.println(ssid);
    while(1) delay(500);
  }
  //start UART and the server
  server.begin();
  server.setNoDelay(true);
  
  Serial.print("Wifi Ready! ");
  Serial.print(WiFi.localIP());
}

void loop() {
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial.print("New client: ");
        Serial.println(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check UART for data
  if(Serial.available())
  {
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    Serial.readBytes(buf, len);
    if(millis() - updateStartTime > 3000)
    {
      updateStartTime = millis();
      //pm2.5传感器原始数据以42 4d开头  edit
      if(buf[0]==0x42&&buf[1]==0x4d)
      {
        //计算数值  edit
        long pm10 = buf[10]*256+buf[11];
        long pm25 = buf[12]*256+buf[13];
        long pm100 = buf[14]*256+buf[15];
        //格式化输出内容，赋值给变量sensor_info  edit
        sprintf(sensor_info,"PM1.0 = %d ug/m3, PM2.5 = %d ug/m3, PM10 = %d ug/m3\n",pm10,pm25,pm100);
        uint8_t sz = strlen(sensor_info);
        //设置字符结束位，勿改
        sensor_info[sz] = 0;
        Serial.write(sensor_info, sz);
        for(i = 0; i < MAX_SRV_CLIENTS; i++)
        {
          if (serverClients[i] && serverClients[i].connected()){
            serverClients[i].write((uint8_t*) sensor_info, sz);
            delay(1);
          }
        }
      }
    }
  }
}

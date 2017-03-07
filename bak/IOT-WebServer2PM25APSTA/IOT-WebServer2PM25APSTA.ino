#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

////////////////////////////////////////////////////
//终端ID
const char *CLIENT_ID = "AM1";  //要以字母开头 edit
//wifi用户名和密码
const char *ssid = "AM1";
const char *password = "12345678";

////////////////////////////////////////////////////

ESP8266WebServer server ( 80 );
const int led = 13;
char sensor_info[50];
int second = 0;
int minute = 0;
int hour = 0;
unsigned long updateStartTime = 0;

void handleRoot(bool jsFlag) {
  digitalWrite ( led, 1 );
  char temp[1000];

  snprintf ( temp, 1000,
  jsFlag ? "SENSORDATA_CALLBACK(\"%s\",\"%s\",\"%02d:%02d:%02d\");\
  " : "<html>\
<head>\
<meta http-equiv='refresh' content='5'/>\
<title>Air monitor</title>\
</head>\
<body>\
<h1>Client %s:</h1>\
<p>Sensor No.1:</p>\
<p>%s - Uptime: %02d:%02d:%02d</p>\
<p>Sensor No.2:</p>\
<p id=\"iot_2\"></p>\
<p>Sensor No.3:</p>\
<p id=\"iot_3\"></p>\
<p>Sensor No.4:</p>\
<p id=\"iot_4\"></p>\
<script>\
function SENSORDATA_CALLBACK(id,content,updateTime)\
{\
document.getElementById(id).innerHTML = (content==\"\"?\"nothing\":content) + \" - Uptime:\"+updateTime;\
}\
</script>\
</body></html>", //此行以上增加其他传感器 edit
    CLIENT_ID, sensor_info, hour, minute % 60, second % 60
  );
  server.send ( 200, "text/html", temp );
  digitalWrite ( led, 0 );
}

void handleNotFound() {
  digitalWrite ( led, 1 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  digitalWrite ( led, 0 );
}


void setup ( void ) {
  delay(2000);
  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );
  Serial.begin ( 9600 );
  
  Serial.println ( "" );
  Serial.print("Configuring access point...");
  //可以自己设置IP
  IPAddress local_ip(192, 168, 8, 8);
  //网关，前三位与IP一致，最后一位必须为1
  IPAddress gateway(192, 168, 8, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on ( "/", []() {
      handleRoot(false);
  });
  server.on ( "/js", []() {
      handleRoot(true);
  });
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
}

void loop ( void ) {
  server.handleClient();
  if(millis() - updateStartTime  > 10000)
  {
    updateStartTime = millis();
    sensor_info[0] = 0;
  }
   if (Serial.available()) {
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    //读取传感器原始数据
    Serial.readBytes(buf, len);
    //pm2.5传感器原始数据以42 4d开头  edit
    if(buf[0]==0x42&&buf[1]==0x4d)
    {
      updateStartTime = millis();
      second = millis() / 1000;
      minute = second / 60;
      hour = minute / 60;
      //计算数值  edit
      long pm10 = buf[10]*256+buf[11];
      long pm25 = buf[12]*256+buf[13];
      long pm100 = buf[14]*256+buf[15];
      long HCHO = buf[28]*2560+buf[29]*10;
      //格式化输出内容，赋值给变量sensor_info  edit
      sprintf(sensor_info,"PM1.0 = %d ug\/m3, PM2.5 = %d ug\/m3, PM10 = %d ug\/m3, HCHO = %d ug\/m3",pm10,pm25,pm100,HCHO);
      uint8_t sz = strlen(sensor_info);
      //设置字符结束位，勿改
      sensor_info[sz] = 0;
      //Serial.write(sensor_info, sz);
    }
  }
}



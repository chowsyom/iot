#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

////////////////////////////////////////////////////
//终端ID
const char *CLIENT_ID = "iot";  //要以字母开头 edit
//wifi用户名和密码
const char *ssid = "iot";
const char *password = "test12345";

////////////////////////////////////////////////////


ESP8266WebServer server ( 80 );
const int led = 13;
char sensor_info[50];
char CMD[8] = {0x42,0x4D,0xE3,0x58,0x58,0x02,0x22,0};
int second = 0;
int minute = 0;
int hour = 0;
unsigned long updateStartTime = 0;
unsigned long lastUpdateStartTime = 0;

void handleRoot(bool jsFlag) {
  digitalWrite ( led, 1 );
  char temp[1000];

  snprintf ( temp, 1000,
  jsFlag ? "SENSORDATA_CALLBACK(\"%s\",\"%s\",\"%02d:%02d:%02d\");\
  " : "<html>\
<head>\
<meta http-equiv='refresh' content='5'/>\
<title>IOT</title>\
</head>\
<body>\
<h1>Client %s:</h1>\
<p>Sensor No.1:</p>\
<p>%s - Uptime: %02d:%02d:%02d</p>\
<p>Sensor No.2:</p>\
<p id=\"iot\"></p>\
<p>Sensor No.3:</p>\
<p id=\"iot_3\"></p>\
<script>\
function SENSORDATA_CALLBACK(id,content,updateTime)\
{\
document.getElementById(id).innerHTML = (content==\"\"?\"nothing\":content) + \" - Uptime:\"+updateTime;\
}\
</script>\
<script src=\"http://iot.local/js\"></script>\
<script src=\"http://iot_3.local/js\"></script>\
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
  delay(1000);
  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );
  Serial.begin ( 9600 );
  //AP STA mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  //WiFi.begin ( ssid, password );
  //Serial.println ( "" );

  // Wait for connection
  //while ( WiFi.status() != WL_CONNECTED ) {
  //  delay ( 500 );
    //Serial.print ( "." );
  //}

  //Serial.println ( "" );
  //Serial.print ( "Connected to " );
  //Serial.println ( ssid );
  //Serial.print ( "IP address: " );
  //Serial.println ( WiFi.localIP() );

  if ( MDNS.begin ( CLIENT_ID ) ) {
    //Serial.println ( "MDNS responder started" );
  }
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
  //Serial.println ( "HTTP server started" );
}

void loop ( void ) {
  server.handleClient();
  if(millis() - lastUpdateStartTime  > 10000)
  {
    lastUpdateStartTime = millis();
    sensor_info[0] = 0;
  }
  if (Serial.available())
  {
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    //读取传感器原始数据
    Serial.readBytes(buf, len);
    //传感器原始数据以42 4d开头  edit
    if(buf[0]==0x42&&buf[1]==0x4d)
    {
      lastUpdateStartTime = millis();
      second = millis() / 1000;
      minute = second / 60;
      hour = minute / 60;
      //计算数值  edit
      long CO2 = buf[4]*256+buf[5];
      //格式化输出内容，赋值给变量sensor_info  edit
      sprintf(sensor_info,"CO2 = %d ppm",CO2);
      uint8_t sz = strlen(sensor_info);
      //设置字符结束位，勿改
      sensor_info[sz] = 0;
      //Serial.write(sensor_info, sz);
    }
  }
  //每3000毫秒给CO2传感器发一次要返回数据的指令
  if(millis() - updateStartTime > 3000)
  {
     updateStartTime = millis();
     Serial.write(CMD, 8);
  }
}



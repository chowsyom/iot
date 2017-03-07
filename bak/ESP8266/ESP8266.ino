#include<SoftwareSerial.h>

#define SSId       "ES_001"                //type your own SSID name
#define PASSWORD   "go2map123"             //type your own WIFI password
#define PORT       "8080"

#define RXPIN 4
#define TXPIN 5

#define DebugBaudRate 115200
#define ESP8266BaudRate 115200

#define DEBUG

#ifdef DEBUG
#define DBG(message)    Serial.print(message)
#define DBGW(message)    Serial.write(message)
#else
#define DBG(message)
#define DBGW(message)
#endif // DEBUG

SoftwareSerial softSerial(RXPIN, TXPIN);

String ssid = SSId;
String password = PASSWORD;
String port=PORT;
unsigned long start;
int cmdIdx;
String cmd[]={"AT+GMR","AT+CWMODE=1","AT+RST","AT+CWJAP=\""+ssid+"\",\""+password+"\"","AT+CIFSR","AT+CIPMUX=1","AT+CIPSERVER=1,"+port};

void setup()
{
  Serial.begin(DebugBaudRate);
  softSerial.begin(ESP8266BaudRate);
  softSerial.listen();
  start = 0;
  cmdIdx = 0;
}
String softRxMsg = "";
boolean cmdFlag=true;
// The loop function is called in an endless loop
void loop()
{
  if (softSerial.available() > 0)
  {
    softRxMsg += (char) softSerial.read();
  }
  else if (softRxMsg != "")
  {
    DBG(softRxMsg+"\r\n");
    
    handleMessage(softRxMsg);
    
    softRxMsg = "";
    cmdFlag=true;
    start = millis();
  }
  if(millis()-start>6000&&cmdFlag==true&&cmdIdx<7){
    DBG("\r\n----------\r\n");
    DBG(String(cmdIdx)+" -> "+cmd[cmdIdx]+"\r\n");
    delay(1000);
    softSerial.println(cmd[cmdIdx]+"\r\n");
    cmdIdx++;
    cmdFlag=false;
  }
  if(cmdIdx>=7){
    digitalWrite(13,LOW); 
  }
}

void handleMessage(String data){
  if (data.indexOf("+IPD")!=-1){
    Trim(data);
    //data.replace("+IPD","");
    int idx=data.indexOf(",");
    String part=data.substring(idx+1);
    idx=part.indexOf(",");
    String cid = part.substring(0,idx);
    part=part.substring(idx+1);
    idx=part.indexOf(":");
    String dataLen = part.substring(0,idx);
    data=part.substring(idx+1);
    sendMsg(cid,dataLen+","+data);
  }
}
void sendMsg(String clientID,String data){
    softSerial.print("AT+CIPSEND=");
    softSerial.print(clientID);
    softSerial.print(",");
    String len = String(data.length());
    //Serial.println("data lenght="+len);
    softSerial.println(len);
    delay(1000);
    softSerial.println(data);
    delay(1000);
}

void Trim(String data){
  char head[4] = {0x0D,0x0A};   
  char tail[7] = {0x0D,0x0A,0x0D,0x0A};        
  data.replace(tail,"");
  data.replace(head,"");
}


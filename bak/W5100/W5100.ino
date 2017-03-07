#include <SPI.h>
#include <Ethernet.h>

int  red = 9;       //设置输出端口
int i ;
int red_color, asc; //定义各颜色的PWM值参数
String POST = "";
String SET = "";
bool state = LOW;
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

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  pinMode(red, OUTPUT);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  //设置端口为输出
}
void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line-----------
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // it is after the double cr-lf that the variables are
          // read another line!
          String POST = "";
          // int p = 1;
          while (client.available())
          {
            c = client.read();
            // save the variables somewhere
            POST += c;
          }
          Serial.println("POST=" + POST);
          // send a standard http response header
          //打印输出
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.print("<html><head><title>Lamaq Color palette</title>");
          client.print("<style>*{font-family:Arial;}body{background-color:#f2f2f2;}h1{color:#222;}</style>");
          client.print("<body><div style='text-align:center;'>");
          client.print("<div>Color palette");
          client.print("<table width='30%'  align='center'>");
          client.print("<tr><td>");
          client.print("<table width='100%' height='100'  cellspacing='1'><tbody><tr>");
          /*********************************************************/
          client.println("</tr></tbody></table>");
          client.println("<table  width='100%'  id='led'>");
          client.print("<tbody><tr>");
          client.print("<form method='post' id='ledform'>Led color:");
          client.print("<input type='text' name='led' id='led' value='");
          /*******************************************************************/
          //  for (int j=0; j<10; j++){
          i = int(POST[4]);
          if (i == 49) {
            red_color = 240;
            state = HIGH;
          }
          if (i == 48) {
            red_color = 0;
            state = LOW;
          }
          //       }
          // i = int(POST[5]);
          //  {red_color=240;client.print(POST[5]);}
          /*********************************************************/
          client.print(analogRead(red));
          client.print("'>");
          client.print("<input type='text' name='zzy' id='zzy' value='");
          client.print("'>");
          client.println("<input type='submit' value='Submit'>"
                         "</form></tr></tbody></table></td></tr></table></div></div></body></html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
    // i=1;
  }
  analogWrite(red, red_color);
  //digitalWrite(red,state);
}

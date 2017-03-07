/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

const char* ssid = "ES_001";
const char* password = "go2map123";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  //调试
  if (Serial.available() > 0)
  {
    if (Serial.find("debug") == true) {
      Serial.println(WiFi.localIP());
    }
  }



  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1;
  if (req.indexOf("on") != -1)
    val = 0;
  else if (req.indexOf("off") != -1)
    val = 1;

  // Set GPIO2 according to the request
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html><body>";
  if (val != -1) {
    digitalWrite(2, val);
    // Prepare the response
    s += "\r\nGPIO is now ";
    s += (val) ? "high" : "low";
    
  }
  s += "</body></html>abcdefghijklmnopqrstuvwxyz1234567890\n";

  client.flush();



  // Send the response to the client
  client.print(s);
  delay(1000);
  Serial.println("---------------------------");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}


#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01
};
IPAddress ip(192, 168, 0, 109); //<<< ENTER YOUR IP ADDRESS HERE!!!

// initialize the library instance:
EthernetClient client;
const int requestInterval = 10000;
// delay between requests
char serverName[] = "192.168.0.101";
boolean requested;
// whether you've made a request since connecting
long lastAttemptTime = 0;
// last time you connected to the server, in milliseconds
String currentLine = "";
// string to hold the text from server
String tweet = "";
// string to hold the tweet
boolean readingTweet = false;
// if you're currently reading the tweet
void setup() {
  pinMode(9, OUTPUT);
  // reserve space for the strings:
  currentLine.reserve(500);
  tweet.reserve(150);
  // initialize serial:
  Serial.begin(9600);
  // attempt a DHCP connection:
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Ethernet.begin(mac, ip);
  }
  // connect to Twitter:
  connectToServer();
}

void loop() {

  if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();
      Serial.write(inChar);
      // add incoming byte to end of line:
      currentLine += inChar;
      if (currentLine.indexOf("ok") > -1) {
        digitalWrite(9, HIGH);
        Serial.println("LED ON!");
      }
      else {
        digitalWrite(9, LOW);
        Serial.println("LED OFF!");
      }
      // close the connection to the server:
      //client.stop();
    }

  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    connectToServer();
  }
}
bool flag = true;
void connectToServer() {
  // attempt to connect, and wait a millisecond:
  Serial.println("connecting to server...");
  if (client.connect(serverName, 8080)) {
    Serial.println("making HTTP request...");
    // make HTTP GET request to twitter:
    if (flag) {
      client.println("ok");
      flag = false;
    }
    else {
      client.println("error");
      flag = true;
    }
    //client.println("HOST: 192.168.0.101");
    client.println();
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}

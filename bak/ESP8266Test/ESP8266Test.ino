#include <ZESP8266.h>
#include <Messenger.h>

void setup() {
 Serial.begin(115200);
  ZWifi.begin();
}
void loop() {

ZWifi.loop();
uint8_t len = 255;
byte buf[len];
ZWifi.receive(buf, &len);
delay(1000);
}

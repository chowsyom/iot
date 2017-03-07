#include "OK.h"

int TrigPin = 2; 
int EchoPin = 3; 
float distance; 
void setup() {
        Serial.begin(9600); 
        pinMode(TrigPin, OUTPUT); 
        pinMode(EchoPin, INPUT); 
}
void loop() {
  mm();
}
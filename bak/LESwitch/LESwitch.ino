#define LedPin 5
#define InPin A0

void setup() {
Serial.begin(9600);


  pinMode(InPin, INPUT);
  pinMode(LedPin, OUTPUT);
}

void loop() {
  int state = analogRead(InPin);
  Serial.println("InPin="+String(state));
  digitalWrite(LedPin,state>200);
  delay(500);
}

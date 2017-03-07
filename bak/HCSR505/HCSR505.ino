
#define HCSR505 3

void setup() {
  Serial.begin(9600);
  pinMode(HCSR505, INPUT);
}

void loop() {
  int val = digitalRead(HCSR505);
  Serial.println("HCSR505=" + String(val));
  delay(1000);
}

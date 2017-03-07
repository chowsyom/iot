
#define ACPIN A1

void setup() {
  pinMode(ACPIN, INPUT);
  Serial.begin(9600);
}
void loop() {
  int val = analogRead(ACPIN);
  Serial.println(val);
  delay(1000);
}

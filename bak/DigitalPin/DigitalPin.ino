
#define DGPIN 2

void setup() {

  pinMode(DGPIN, INPUT);
  Serial.begin(9600);
}
void loop() {
  int val = digitalRead(DGPIN);
  Serial.println(val);
  delay(1000);
}

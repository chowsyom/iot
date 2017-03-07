
#define input_PIN 3

void setup() {
  Serial.begin(9600);
  //Serial.setTimeout(500);
  pinMode(input_PIN,INPUT);
}

void loop() {
  bool val = digitalRead(input_PIN);
  Serial.print("value = ");
  Serial.println(val);
  delay(1000);
}

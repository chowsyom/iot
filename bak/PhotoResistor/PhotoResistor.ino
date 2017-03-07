
#define Pr_1_1_IN A0
#define Pr_1_2_IN 2
#define Pr_2_IN 3

void setup() {
  pinMode(Pr_1_1_IN, INPUT);
  pinMode(Pr_1_2_IN, INPUT);
  pinMode(Pr_2_IN, INPUT);
  Serial.begin(115200);
}
void loop() {
  int val = analogRead(Pr_1_1_IN);
  Serial.println(val);
  delay(1000);
  int val2 = digitalRead(Pr_1_2_IN);
  Serial.println(val2);
  delay(1000);
  int val3 = digitalRead(Pr_2_IN);
  Serial.println(val3);
  delay(1000);
}

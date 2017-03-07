#define GREENLEDPIN 6
#define GASPIN 2
#define GASPIN_A A0
#define BODYPIN 3

void setup() {
  // put your setup code here, to run once:
pinMode(GREENLEDPIN, OUTPUT);
pinMode(GASPIN, INPUT);
pinMode(BODYPIN, INPUT);
pinMode(GASPIN_A, INPUT);

 Serial.begin(9600);
}
bool flag = true;
void loop() {
  int val1 = digitalRead(BODYPIN);//HBIPIN SOUNDPIN
  Serial.print(val1);
  int val2 = digitalRead(GASPIN);//HBIPIN SOUNDPIN
  Serial.print(",");
  Serial.print(val2);
  int val3 = analogRead(GASPIN_A);
  Serial.print(",");
  Serial.println(val3);
  

  delay(1000);
}

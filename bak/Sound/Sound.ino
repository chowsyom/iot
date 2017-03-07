#define GREENLEDPIN 6
#define SOUNDPIN 10
#define SOUNDPIN_A A3
#define SOUNDPIN_A2 A2
void setup() {
  // put your setup code here, to run once:
pinMode(GREENLEDPIN, OUTPUT);
pinMode(SOUNDPIN, INPUT);
pinMode(SOUNDPIN_A, INPUT);
pinMode(SOUNDPIN_A2, INPUT);
 Serial.begin(115200);
}
bool flag = true;
void loop() {
  //put your main code here, to run repeatedly:
  //digitalWrite(SOUNDPIN,flag?LOW:HIGH);
  //flag = !flag;
  int level = digitalRead(SOUNDPIN);//HBIPIN SOUNDPIN
  digitalWrite(GREENLEDPIN,level);
  Serial.print(level);
  /*int sound = analogRead(SOUNDPIN_A);
  Serial.print(",");
  Serial.print(sound);
  int sound2 = analogRead(SOUNDPIN_A2);
  Serial.print(",");
  Serial.println(sound2);
  */
  delay(1000);
}

#define NIGHT_LIGHT_PIN 3

void fadeIn()
{
  uint8_t i = 0;
  while(i<255)
  {
    analogWrite(NIGHT_LIGHT_PIN, i);
    i+=5;
    delay(100);
  }
}
void fadeOut()
{
  uint8_t i = 255;
  while(i>0)
  {
    analogWrite(NIGHT_LIGHT_PIN, i);
    i-=5;
    delay(100);
  }
}

void setup() {
  pinMode(NIGHT_LIGHT_PIN, OUTPUT);
  analogWrite(NIGHT_LIGHT_PIN, 0);
}

void loop() {
  fadeIn();
  delay(3000);
  fadeOut();
}

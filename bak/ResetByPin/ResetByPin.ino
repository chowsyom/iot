void setup()
{
    Serial.begin(115200);
    pinMode(2,OUTPUT);
    digitalWrite(2, HIGH);   // switch off
    Serial.println("hello!");
}
uint8_t i = 0;
void loop()
{
  i++;
  Serial.print(i);
  Serial.println(" looping ... ");
  delay(1000);
   if (Serial.available() > 0)
   {
       char val;
       val = Serial.read();     // read serial data
       if(val == 'r')
       {
          delay(5000);
          digitalWrite(2, LOW);    // 将reset管脚的电平拉低50ms，起到复位的作用
          delay(50);
          digitalWrite(2, HIGH);
      } 
  }
}

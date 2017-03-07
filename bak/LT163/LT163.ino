unsigned long serialSpeed = 4800;

void setup()
{

  //Serial.begin(9600);
  Serial1.begin(9600);  //设置波特率为 9600bps.
  Serial2.begin(serialSpeed);
}
byte requset[] = {0X01, 0X03, 0X00, 0X48, 0X00, 0X0A, 0X45, 0XDB};

void loop() {
  Serial2.flush();     // 清空串口接收缓冲区
  Serial2.write(&requset[0], 8);
  Serial1.println("data sended.");
  byte data[25];
  unsigned char n = 0;
  while (Serial2.available())
  {
    data[n++] = Serial2.read();
  }
  if (data[0] == 0X01 && data[1] == 0X03 && data[2] == 0X14) {
    if (n >= 25) {
      Serial1.print("V=");
      unsigned int v = data[3]*0X100+data[4];
      Serial1.print(v/100.0);
      Serial1.println("V");

      Serial1.print("A=");
      unsigned int a = data[5]*0X100+data[6];
      Serial1.print(a/100.0);
      Serial1.println("A");

      Serial1.print("W=");
      unsigned int w = data[7]*0X100+data[8];
      Serial1.print(w);
      Serial1.println("W");

      Serial1.print("kWh=");
      unsigned int kWh = data[9]*0X1000000+data[10]*0X10000+data[11]*0X100+data[12];
      Serial1.print(kWh/3200.0,3);
      Serial1.println("KWh");

      Serial1.print("ys=");
      unsigned int ys = data[13]*0X100+data[14];
      Serial1.println(ys/1000.0);

      Serial1.print("CO2=");
      unsigned int co2 = data[15]*0X1000000+data[16]*0X10000+data[17]*0X100+data[18];
      Serial1.print(co2/1000.0,3);
      Serial1.println("Kg");

      Serial1.print("Hz=");
      unsigned int Hz = data[21]*0X100+data[22];
      Serial1.print(Hz/100.0);
      Serial1.println("Hz");
      
    }
  }

  delay(1000);



}

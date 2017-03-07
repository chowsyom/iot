#include<SoftwareSerial.h>
#include <Wire.h>
 
#define RTC_Address   0x68 //0x32//器件地址

 
 //端口连接
 //SDA-AD4(Gadgeteer PIN8)
 //SCL-AD5(Gadgeteer PIN9)
 
 
 unsigned char   date[7];
 
 //读SD2403实时数据寄存器
 void I2CReadDate(void)
 {
      unsigned char n=0;
      
      Wire.requestFrom(RTC_Address,7); 
      while(Wire.available())
      {  
        date[n++]=Wire.read();
      }
      delayMicroseconds(1);
      Wire.endTransmission();
 }
 //写SD2403实时数据寄存器
 void I2CWriteDate(void)
 {		
      WriteTimeOn();
     
      Wire.beginTransmission(RTC_Address);
      Wire.write(byte(0));//设置写起始地址
      Wire.write(0x50);//second:1秒
      Wire.write(0x11);//minute:45分
      Wire.write(0x99);//hour:8时(二十四小时制)     	
      Wire.write(0x03);//week:星期3
      Wire.write(0x17);//day:17号
      Wire.write(0x02);//month:2月
      Wire.write(0x16);//year:2016年
      Wire.endTransmission();
 
      Wire.beginTransmission(RTC_Address);
      Wire.write(0x12);//设置写起始地址
      Wire.write(byte(0));//清零数字调整寄存器
      Wire.endTransmission(); 
 	
      WriteTimeOff();      
 }
 //SD2400写允许程序
 void WriteTimeOn(void)
 {		
      Wire.beginTransmission(RTC_Address);       
      Wire.write(0x10);//设置写地址10H      	
      Wire.write(0x80);//置WRTC1=1      
      Wire.endTransmission();
 	
      Wire.beginTransmission(RTC_Address);    
      Wire.write(0x0F);//设置写地址0FH      	
      Wire.write(0x84);//置WRTC2,WRTC3=1      
      Wire.endTransmission(); 	
 }
 //SD2400写禁止程序
 void WriteTimeOff(void)
 {		
      Wire.beginTransmission(RTC_Address);   
      Wire.write(0x0F);//设置写地址0FH      	
      Wire.write(byte(0));//置WRTC2,WRTC3=0      
      Wire.write(byte(0));//置WRTC1=0(10H地址)      
      Wire.endTransmission(); 
 }
 //数据处理
 void Data_process(void)
 {
      unsigned char i;
      
      for(i=0;i<7;i++)
      {
          if(i!=2)
          date[i]=(((date[i]&0xf0)>>4)*10)+(date[i]&0x0f);
          else
          {
             date[2]=(date[2]&0x7f);
             date[2]=(((date[2]&0xf0)>>4)*10)+(date[2]&0x0f);
          }
 
      }
      
      /*Serial.print("Sec = ");//second
      Serial.print(date[0]);
      Serial.print("   Min = ");//minute
      Serial.print(date[1]);
      Serial.print("   H = ");//hour
      Serial.print(date[2]);
      Serial.print("   W = ");//week
      Serial.print(date[3]);
      Serial.print("   D = ");//day
      Serial.print(date[4]);
      Serial.print("   M = ");//month
      Serial.print(date[5]);
      Serial.print("   Y = ");//year
      Serial.print(date[6]);*/
      Serial.print("20");
      Serial.print(date[6]);
      Serial.print("-");
      Serial.print(format(date[5]));
      Serial.print("-");
      Serial.print(format(date[4]));
      Serial.print(" ");
      Serial.print("W");
      Serial.print(date[3]);
      Serial.print(" ");
      Serial.print(format(date[2]));
      Serial.print(":");
      Serial.print(format(date[1]));
      Serial.print(":");
      Serial.print(format(date[0]));
      
      Serial.println();
 }
 String format(char c){
  return (c<10?"0":"")+String(int(c));
 }

//读SD2403实时数据寄存器
String  getDateTime(void)
{
  Wire.beginTransmission(RTC_Address);
  Wire.write(uint8_t(0x00));
  Wire.endTransmission();
  
  unsigned char n = 0;
  Wire.requestFrom(RTC_Address, 7);
  while (Wire.available())
  {
    date[n] = Wire.read();
    if (n == 2) {
      date[n] = (date[n] & 0x7f);
    }
    date[n] = (((date[n] & 0xf0) >> 4) * 10) + (date[n] & 0x0f);
    n++;
  }
  delayMicroseconds(1);
  Wire.endTransmission();
  return "20" + format(date[6]) + "-" + format(date[5]) + "-" + format(date[4]) + " " + format(date[2]) + ":" + format(date[1]) + ":" + format(date[0]);
}
 
 void setup()
 {
      Wire.begin();
      Serial.begin(115200); 
 }
 void loop()
 {
      //I2CWriteDate();//写实时时钟
       //delay(100);
      
      //while(1)
      //{  
         //I2CReadDate();//读实时时钟     
          //Data_process();//数据处理	

          Serial.println(getDateTime());
          delay(1000);//延时1S
      //}
 
 }
 

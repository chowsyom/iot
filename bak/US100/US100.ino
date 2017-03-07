
unsigned int HighLen = 0; 
unsigned int LowLen  = 0; 
unsigned int Len_mm  = 0; 

unsigned long serialSpeed = 9600;

void setup() 
{   
   
  Serial.begin(9600);  //设置波特率为 9600bps. 
  Serial1.begin(serialSpeed);
  Serial.println("Hello!"); 
}  
void loop() { 
  Serial1.flush();     // 清空串口接收缓冲区 
  Serial1.write(0X55); // 发送0X55，触发US-100开始测距        
  delay(500);          //延时500毫秒        
  if(Serial1.available() >= 2)            //当串口接收缓冲区中数据大于2字节        
  { 
    HighLen = Serial1.read();                   //距离的高字节 
    LowLen  = Serial1.read();                   //距离的低字节          
    Len_mm  = HighLen*256 + LowLen;             //计算距离值          
    if((Len_mm > 1) && (Len_mm < 10000))    //有效的测距的结果在1mm到10m之间          
    {              
      Serial.print("Present distance is: ");   //输出结果至串口监视器             
      Serial.print(Len_mm, DEC);            //输出结果至串口监视器               
      Serial.println("mm");                  //输出结果至串口监视器          
    }      
  }      
  Serial1.flush();     // 清空串口接收缓冲区            
  Serial1.write(0X50);
  delay(500);          //延时500毫秒        
  if(Serial1.available() >= 0) 
  { 
    unsigned int tmp  = Serial1.read()-0X2D;               
      Serial.print(tmp, DEC);          
      Serial.println(" *C"); 
    
  }      
  delay(100); 

  
}

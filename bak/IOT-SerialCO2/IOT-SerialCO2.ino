char sensor_info[50];
unsigned long updateStartTime = 0;
void setup ( void ) {
  Serial.begin ( 9600 );
}

void loop ( void ) {
   if (Serial.available()) {
    delay(100);
    size_t len = Serial.available();
    byte buf[len];
    //读取传感器原始数据
    Serial.readBytes(buf, len);
    if(millis() - updateStartTime > 3000)
    {
      updateStartTime = millis();
      //pm2.5传感器原始数据以42 4d开头  edit
      if(buf[0]==0x42&&buf[1]==0x4d)
      {
        //计算数值  edit
        long CO2 = buf[4]*256+buf[5];
        //格式化输出内容，赋值给变量sensor_info  edit
        sprintf(sensor_info,"CO2 = %d ppm\n",CO2);
        uint8_t sz = strlen(sensor_info);
        //设置字符结束位，勿改
        sensor_info[sz] = 0;
        Serial.write(sensor_info, sz);
      }
    }
  }
}



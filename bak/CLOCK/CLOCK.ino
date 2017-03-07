
unsigned long seconds;
int  s = 0, m = 0, h = 0, d = 0, mon = 0, y = 0;   //时间进位
int  second = 0, minute = 0, hour = 0, day = 0, month = 0, year = 0;  //当前时间
int  SECOND = 0, MINUTE = 0, HOUR = 0, DAY = 0, MONTH = 0, YEAR = 0;  //初始时间
void setup()
{ 
  set(2015,5,15,22,19,32);  //设置初始时间
}


void time()   //计算时间
{    
     second = ( SECOND + seconds ) % 60;   //计算秒
     m = ( SECOND + seconds ) / 60;        //分钟进位
     FormatDisplay(6,1,second);
     
     minute = ( MINUTE + m ) % 60;  //计算分钟
     h = ( MINUTE + m ) / 60;       //小时进位
     FormatDisplay(3,1,minute);      
    
     hour = ( HOUR + h ) % 24;   //计算小时
     d = ( HOUR + h ) / 24;      //天数进位
     FormatDisplay(0,1,hour); 
}

int Days(int year, int month)   //根据年月计算当月天数
{
    int days = 0;
    if (month != 2) 
    {
      switch (month) 
      {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: days = 31;  break;
        case 4: case 6: case 9: case 11:  days = 30;  break;
      }
    } 
    else //闰年
    {
      if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0)    days = 29;
      else    days = 28;    
    }  
   return days;   
}
 
void Day()   //计算当月天数
{     
   int days = Days(year,month);
   int days_up;
   if( month == 1 )   days_up = Days( year - 1, 12 ); 
   else  days_up = Days( year, month - 1 );
   day = ( DAY + d ) % days;
   if( day == 0 )   day = days;    
   if( ( DAY + d ) == days + 1 )
   {
     DAY -= days;
     mon++;
   }
   if(( DAY + d ) == 0)
   {
     DAY += days_up;
     mon--;
   }
   FormatDisplay(8,0,day); 
}

void Month()    //计算月份
{  
   month = ( MONTH + mon ) % 12;
   if( month == 0 )  month = 12;
   y = ( MONTH + mon - 1) / 12;
   FormatDisplay(5,0,month); 
}

void Year()    //计算年份
{ 
    year = ( YEAR + y ) % 9999;
    if( year == 0 ) year = 9999;
    lcd.setCursor(0, 0); 
    if(year < 1000)  { lcd.print("0"); }
    if(year < 100)   { lcd.print("0"); }
    if(year < 10)    { lcd.print("0"); }
    lcd.print(year);
}

void Week(int y,int m, int d)  //根据年月日计算星期几
{           
    if(m==1) m=13;
    if(m==2) m=14;
    int week=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7+1; 
    String weekstr="";
    switch(week)
      {
          case 1: weekstr="Mon. ";   break;
          case 2: weekstr="Tues. ";  break;
          case 3: weekstr="Wed. ";   break;
          case 4: weekstr="Thur. ";  break;
          case 5: weekstr="Fri. ";   break;
          case 6: weekstr="Sat. ";   break;
          case 7: weekstr="Sun. ";   break;
      }    
    lcd.setCursor(11, 0); 
    lcd.print(weekstr);
}

void set(int y, int mon, int d, int h, int m, int s)
{
  YEAR = y;
  MONTH = mon;
  DAY = d;  
  HOUR = h;
  MINUTE = m;
  SECOND = s;  
}

void Display()  //显示时间、日期、星期
{ 
  time();
  Day();  
  Month();
  Year();
  Week(year,month,day);  
}

void loop() 
{     
   seconds = millis()/1000;    
   Display();
}


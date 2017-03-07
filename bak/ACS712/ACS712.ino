//Demo for the current measurement method of the Current Sensor
//by Catalex
//catalex.taobao.com
//Demo Function: Get the voltage output of the VOUT pin with the analog
//               input pin of Arduino Boards and calculate the current.

#define ACS712_ZERO_VOL 2.505 //2.495V. Set the macro value of the voltage output 
                         //of the ACS712 when the measured current is zero.
#define ACS712_SENSITIVITY 0.185 //0.185mV is typical value
#define ADC_RESOLUTION  (float)5/1024 // 5/1024 is eaque 0.0049V per unit

#define ACS712_VOUT A0

int samplesnum = 1000;
float current;

void setup() 
{
  Serial.begin(9600);
}
 
void loop() 
{
  float current_sum = 0;
  for(int i = 0; i < samplesnum; i++) 
  {
    current_sum +=  ((float)analogRead(ACS712_VOUT)*ADC_RESOLUTION - ACS712_ZERO_VOL)/ACS712_SENSITIVITY;
    delay(1);
  }
  current = current_sum/samplesnum;
  Serial.println("The measured current is ");
  Serial.print(current,3); 
  Serial.println(" A");
  delay(1000);
}


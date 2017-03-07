#include <VirtualWire.h>


#define DELAY 5000

char* msg;
uint8_t sendedSize = 0;
void sendMsg(void)
{

  //char *msg="hellohello hello from com4";
  sendedSize = strlen(msg);
  vw_send((uint8_t *)msg, strlen(msg));
  vw_wait_tx(); // Wait until the whole message is gone
  Serial.println("message sended=" + String(msg) + "(" + String(strlen(msg)) + ")");

}

uint8_t receiveSize = 0;
uint8_t receiveBuf[VW_MAX_MESSAGE_LEN];
boolean receiveMsg(void)
{
  boolean flag = false;
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(receiveBuf, &buflen)) // Non-blocking
  {
    int i;
    receiveBuf[buflen] = 0;
    Serial.println("Got: " + String((char*) receiveBuf) + "(" + String(buflen) + ")");
    receiveSize = buflen;
    flag = true;
  }
  return flag;
}

void setup()
{
  Serial.begin(9600);    // Debugging only
  Serial.println("setup");

  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);  // Bits per sec

  vw_rx_start();       // Start the receiver PLL running

}


unsigned long startTime = 0 - 5 - DELAY;
int INPUT_SIZE = 150;
void loop()
{



  //  if (millis() - startTime > DELAY) {
  //    sendMsg();
  //    startTime = millis();
  //  }
  if (receiveMsg())
  {
    //Serial.println("receiveSize="+String(receiveSize)+","+String((int) receiveBuf[0]));
    if (receiveSize > 4 && receiveBuf[0] == 0xFE && receiveBuf[1] == 0x7C)
    {
      if (receiveBuf[2] == 0x01)  //匹配设备号
      {
        if (receiveBuf[3] == 0xFD) //匹配指令
        {
          if (receiveBuf[4] != sendedSize) sendMsg();
          else Serial.println("sended message done.");
        }
        else
        {
          char retMsg[] = {0xFE, 0x7C, 0x01, 0xFD, (char) receiveSize, 0};
          msg = retMsg;
          sendMsg();
        }
      }
    }

  }
  char input[INPUT_SIZE + 1];
  byte size = Serial.readBytes(input, INPUT_SIZE);
  if (size > 0) {
    Serial.println("inputSize=" + String(size));
    input[size] = 0;// Add the final 0 to end the C string
    char inputTmp[size + 4];
    char head[3] = {0xFE, 0x7C, 0x01};
    //strcat(inputTmp, head); 不能使用strcat，两个数组在内存中不连续
    //strcat(inputTmp, input);

    char i,j;
    for(i=0,j=0;i<3;i++)
    {
      inputTmp[i] = head[j++];
    }
    for(i=3,j=0;i<size + 4;i++)
    {
      if(j==0) {
        int temp = (int) (input[j++]-48);
        inputTmp[i] = (char) temp;
        Serial.println(inputTmp[i],HEX);
      }
      else inputTmp[i] = input[j++];
    }
    //Serial.println("inputTmp 2 size=" + String(sizeof(inputTmp) / sizeof(*inputTmp)));

    msg = inputTmp;
    sendMsg();
  }
}

#define INPUT_SIZE 119
void setup() {
  Serial.begin(9600);
}


//01:02-03:04,05:06-07:08,09:10-11:12,13:14-15:16,17:18-19:20,21:22-23:24,25:26-27:28,29:30-31:32,33:34-35:36,37:38-39:40


//更新定时列表
void updateTimeList(char* input)
{
  // Read each command pair
  char* timeStr = strtok(input, ",");
  while (timeStr != 0)
  {
    // Split the timeStr in two part
    char* colon = strchr(timeStr, ':');
    if (colon != 0)
    {
      // Actually split the string in 2: replace '-' or ':' with 0
      *colon = 0;
      int startHour = atoi(timeStr);
      Serial.println("startHour="+String(startHour));
      ++colon;
      //Serial.println("timeSeparator 1="+String(timeSeparator));
      char* hyphen = strchr(colon, '-');
      //Serial.println("timeSeparator 2="+String(timeSeparator));
      if (hyphen != 0)
      {
        *hyphen = 0;
        int startMin = atoi(colon);
        Serial.println("startMin="+String(startMin));
        ++hyphen;
        colon = strchr(hyphen, ':');
        if (colon != 0)
        {
          *colon = 0;
          int endHour = atoi(hyphen);
          Serial.println("endHour="+String(endHour));
          ++colon;
          int endMin = atoi(colon);
          Serial.println("endMin="+String(endMin));
        }
      }
    }
    // Find the next command in input string
    timeStr = strtok(0, ",");
  }
}

void loop() {
  char input[INPUT_SIZE + 1];
  byte size = Serial.readBytes(input, INPUT_SIZE);
  // Add the final 0 to end the C string
  input[size] = 0;
  updateTimeList(input);
}

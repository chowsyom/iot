
#include <VirtualWire.h>

char msg[8] = {0xFE,0x7C,0x05,0x01,0x01,0x02,0x02,0};

void setup()
{
    Serial.begin(9600);    // Debugging only
    Serial.println("setup");

    // Initialise the IO and ISR
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);  // Bits per sec
}

void loop()
{
    msg[4]++;

    digitalWrite(13, true); // Flash a light to show transmitting
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(13, false);
    delay(2000);
}

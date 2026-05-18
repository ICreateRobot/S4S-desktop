/**
 * Control two servos to swing around 0-180 degrees
 */
#include "s4s_mainBoard.h"
#include "stdio.h"

s4s_mainBoard mainBoard;

void setup() 
{
    Serial.begin(115200);
    mainBoard.begin();
}

void loop() 
{
    mainBoard.servo_set_angle(0, 0);
    mainBoard.servo_set_angle(1, 180);
    delay(1000);

    mainBoard.servo_set_angle(0, 180);
    mainBoard.servo_set_angle(1, 0);
    delay(1000);
}

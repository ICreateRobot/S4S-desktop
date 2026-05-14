/**
 * Control how many degrees the motor rotates
 */
#include "s4s_mainBoard.h"
#include "stdio.h"

s4s_mainBoard mainBoard;

void setup()
{
    mainBoard.begin();
}

void loop()
{
    // Rotate forward 200 degrees
    mainBoard.encoder_motor_set_relative_angle(0, 200);
    mainBoard.encoder_motor_set_action(0, 7);
    delay(2000);

    // Invert 200 degrees
    mainBoard.encoder_motor_set_relative_angle(0, 0);
    mainBoard.encoder_motor_set_action(0, 8);
    delay(2000);
}

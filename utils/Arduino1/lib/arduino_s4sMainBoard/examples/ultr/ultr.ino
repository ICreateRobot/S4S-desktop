/**
 * Show red when the ultrasonic distance
 * is less than 30 centimeters, 
 * otherwise show green.
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
    uint16_t distance = mainBoard.ultr_get_distance();

    if (distance < 30)
    {
        mainBoard.ultr_set_color(255, 255, 0, 0);
    }
    else
    {
        mainBoard.ultr_set_color(255, 0, 255, 0);
    }

    char print_buffer[50] = {0};
    sprintf(print_buffer, "distance: %d\n", distance);
    Serial.print(print_buffer);
}

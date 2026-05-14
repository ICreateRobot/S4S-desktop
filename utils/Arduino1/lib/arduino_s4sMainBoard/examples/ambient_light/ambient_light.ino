/**
 * Control ambient lighting
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
    mainBoard.ambient_light_set_state(255, 255, 0, 0);
    delay(1000);

    mainBoard.ambient_light_set_state(255, 0, 255, 0);
    delay(1000);

    mainBoard.ambient_light_set_state(255, 0, 0, 255);
    delay(1000);
}

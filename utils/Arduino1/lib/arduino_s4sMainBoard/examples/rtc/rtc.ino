/**
 * Set and get the RTC date and time
 */
#include "s4s_mainBoard.h"
#include "stdio.h"

s4s_mainBoard mainBoard;

void setup() 
{
  Serial.begin(115200);
  mainBoard.begin();

  /**
   * Set the RTC date and time. 
   * Disabling the functions below can prevent the settings 
   * from being reset every time the device is powered on.
   */
  mainBoard.rtc_set_data(25, 11, 22);
  mainBoard.rtc_set_time(16, 2, 21);
}

void loop() 
{
  char print_buffer[50];

  uint8_t year, month, day, week;
  uint8_t hour, minute, second;

  // Get the RTC date and time
  mainBoard.rtc_get_data(&year, &month, &day, &week);
  mainBoard.rtc_get_time(&hour, &minute, &second);

  sprintf(print_buffer, "%d - %d - %d - %d\n", year, month, day, week);
  Serial.print(print_buffer);

  sprintf(print_buffer, "%d : %d : %d\n\n", hour, minute, second);
  Serial.print(print_buffer);

  delay(250);
}

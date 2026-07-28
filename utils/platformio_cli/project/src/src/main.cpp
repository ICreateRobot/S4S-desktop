#include "TinkerCode.h"

float my_variable;

void app_setup() {
  hw_esp_oled.init();
}

void app_loop() {
  hw_esp_oled.print(0,0,"ABC");
  hw_esp_oled.refresh();
  if_uart_comm.print(if_uart_comm.read_string(),'\n');
  hw_esp_oled.clear_screen();
  hw_esp_oled.print(0,0,"123");
  hw_esp_oled.refresh();
  delay(1 * 1000);

  delay(2);
}

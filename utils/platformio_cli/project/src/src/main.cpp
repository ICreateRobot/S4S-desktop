#include "TinkerCode.h"

float my_variable;

void app_setup() {
}

void app_loop() {
  if_uart_comm.print(if_uart_comm.read_string(),'\n');
  delay(1 * 1000);

  delay(2);
}

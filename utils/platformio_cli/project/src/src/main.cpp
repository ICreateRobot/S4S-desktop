#include "TinkerCode.h"

float my_variable;

void app_setup() {
}

void app_loop() {
  delay(1 * 1000);
  if_uart_comm.send_bytes("hello\r\n");

  delay(2);
}

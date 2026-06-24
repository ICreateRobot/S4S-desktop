#include "TinkerCode.h"

float _E6_88_91_E7_9A_84_E5_8F_98_E9_87_8F;

void app_setup() {
}

void app_loop() {
  if (String("aa") == (if_uart_comm.read_bytes_until(','))) {
    if_uart_comm.send_bytes("hello");
  }

  delay(2);
}

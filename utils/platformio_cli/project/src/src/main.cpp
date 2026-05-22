#include "TinkerCode.h"

float my_variable;

void app_setup() {
}

void app_loop() {
  if (hw_gray.line_sensor_color(hw_gray.LINE_SENSOR_PROBE_L2,hw_gray.LINE_SENSOR_RED)==1) {
    hw_cultr.ultrasonic_set_color(255,0,0);
  }
  if (hw_gray.line_sensor_color(hw_gray.LINE_SENSOR_PROBE_L2,hw_gray.LINE_SENSOR_GREEN)==1) {
    hw_cultr.ultrasonic_set_color(25,255,0);
  }

  delay(2);
}

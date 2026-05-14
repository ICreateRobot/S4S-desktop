#include "TinkerCode.h"
void app_setup(){
  pinMode(A0 , OUTPUT);
}
void app_loop(){
digitalWrite(A0,HIGH);
delay(1000);
digitalWrite(A0,LOW);
delay(1000);
}

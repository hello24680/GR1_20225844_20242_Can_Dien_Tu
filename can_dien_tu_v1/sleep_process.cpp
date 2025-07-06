#include "sleep_process.h"

#define WAKEUP_PIN GPIO_NUM_33

void wakeup_Init(){
  // Setup pin mode
  pinMode(WAKEUP_PIN, INPUT_PULLDOWN);   // Nếu không có tín hiệu điện thì mức 0, có tín hiệu thì mức 1

  // Kiểm tra lý do thức dậy
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("scale is awake and ready!");
  } 

  delay(2000); // chờ một chút để in log

  // Cấu hình chân đánh thức (GPIO33, HIGH là tín hiệu đánh thức)
  esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, 1); // 1 = đánh thức khi GPIO ở mức high
}

void enterSleep(){
    Serial.println("enter sleep mode...");
    delay(1000);
    esp_deep_sleep_start();
}
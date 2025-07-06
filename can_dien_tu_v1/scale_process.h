#ifndef SCALE_PROCESS_H
#define SCALE_PROCESS_H


#include <Arduino.h>
#include <stdio.h>
#include <Wire.h>

#include <HX711.h>
#include <LCD-I2C.h>



void Scale_Init(); // Setup scale
void Scale_Close(); // Tắt đèn lcd

void tareScale(); // Hàm Tare giá trị để loại bỏ khối lượng của chính cái cân
float weightScale(); // Thực hiện cân và trả về khối lượng
void displayWeighResult(float weight); // Hiển thị kết quả lên LCD & Serial
void startScaleProcess(); // Thực hiện quy trình cân
float getWeight(); // Lấy giá trị cân nặng

#endif
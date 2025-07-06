#ifndef UART_INTERFACE_H
#define UART_INTERFACE_H

#include <Arduino.h>
#include <stdio.h>


void interface_Init(); // Setup uart

void showMainMenu(); // Giao diện chọn 1, 2, 3
void showScaleOptions(); // Giao diện chọn cân: lưu / không lưu / hủy
void showLogOptions(); // Giao diện xóa log hoặc thoát

#endif
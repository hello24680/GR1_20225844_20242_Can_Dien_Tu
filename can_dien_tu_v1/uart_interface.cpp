#include "uart_interface.h"

// Setup 
void interface_Init(){
  Serial.begin(115200);
  delay(500); // Đợi Serial ổn định
}

// Giao diện Menu chính
void showMainMenu() {
  Serial.println("========== MENU CHÍNH ==========");
  Serial.println("1. Cân ");
  Serial.println("2. Xem lịch sử cân");
  Serial.println("3. Thoát");
  Serial.println("================================");
}

// Giao diện chọn chức năng cân
void showScaleOptions() {
  Serial.println("======== TÙY CHỌN CÂN =========");
  Serial.println("1. Lưu kết quả");
  Serial.println("2. Không lưu kết quả");
  Serial.println("3. Hủy và quay lại menu chính");
  Serial.println("================================");
}

// Giao diện chọn chức năng log (lịch sử cân)
void showLogOptions() {
  Serial.println("====== TÙY CHỌN HIỂN THỊ =======");
  Serial.println("1. Xóa lần cân mới nhất ");
  Serial.println("2. Xóa toàn bộ lịch sử cân");
  Serial.println("3. Thoát và quay lại menu chính");
  Serial.println("================================");
}

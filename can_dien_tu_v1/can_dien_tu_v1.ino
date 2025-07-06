#include "uart_interface.h"
#include "scale_process.h"
#include "log_process.h"
#include "sleep_process.h"

// uart interface var
char userInput = 0;   // Biến lưu ký tự nhập từ Serial
int currentMenu = 0;  // 0 = Main, 1 = Scale Options, 2 = Log Options

// log process var
uint16_t newLogAddr;

// timeout var
unsigned long lastActivityTime = 0;
const unsigned long timeout = 60000; //  1 phút

void setup() {
 interface_Init(); //uart_interface setup
 wakeup_Init(); //sleep_process setup

 DS3231_Module_Init(); //log_process setup
 Scale_Init(); //scale_process setup

 lastActivityTime = millis(); // Khởi tạo thời gian hoạt động
 showMainMenu();
}

void loop() {
  // chạy theo lựa chọn
  if (Serial.available()) {
    userInput = Serial.read();
    lastActivityTime = millis(); // Cập nhật thời gian hoạt động gần nhất

    switch(currentMenu){
      case 0: //main menu
        switch(userInput){
          case '1': // chế độ cân
            Serial.println(">> Vào chế độ cân...");
            currentMenu = 1;
            showScaleOptions();
            break;
          case '2': // chế độ xem lịch sử
            Serial.println(">> Truy cập lịch sử cân...");
            currentMenu = 2;
            printAllLogs();
            Serial.printf("Tổng số bản ghi: %u\n", getTotalLogs());
            showLogOptions();
            break;
          case '3': // thoát
            Serial.println(">> Thoát chương trình và chuyển sang chế độ sleep...");
            DS3231_Module_Close(); //lưu địa chỉ con trỏ stack
            Scale_Close();
            enterSleep(); // thực hiện sleep cho esp32
            break;

          default:
            Serial.println("Vui lòng chọn (1-3): ");
            break;
        }
        break;

      case 1: //Scale menu
        switch (userInput) {
          case '1':
            Serial.println(">> Chọn lưu kết quả...");
            startScaleProcess();

            WeightLog log;
            getTime(&(log.timestamp));	// lấy thời gian hiện tại từ DS3231
            log.weight = getWeight(); // lấy giá trị cân nặng
            newLogAddr =  getLastLogAddress(); //lấy adrr của con trỏ stack chỉ tới
            writeWeightLog(newLogAddr, &log);

            currentMenu = 0;
            showMainMenu();
            break;
          case '2':
            Serial.println(">> Chọn không lưu kết quả...");
            startScaleProcess();

            currentMenu = 0;
            showMainMenu();
            break;
          case '3':
            Serial.println(">> Hủy, quay lại menu chính...");
            currentMenu = 0;
            showMainMenu();
            break;
          default:
            Serial.println("Vui lòng chọn (1-3): ");
            break;
        }
        break;

      case 2: //log menu
        switch (userInput) {
          case '1':
            Serial.println(">> Đã xóa lần cân mới nhất...");
            clearLastLog();
            Serial.printf("Số bản ghi còn lại: %u\n", getTotalLogs());

            currentMenu = 0;
            showMainMenu();
            break;
          case '2':
            Serial.println(">> Đã xóa toàn bộ lịch sử cân...");
            clearAllLogs();
            Serial.printf("Số bản ghi còn lại: %u\n", getTotalLogs());

            currentMenu = 0;
            showMainMenu();
            break;
          case '3':
            Serial.println(">> Quay lại menu chính.");
            currentMenu = 0;
            showMainMenu();
            break;
          default:
            Serial.println("Vui lòng chọn (1-3): ");
            break;
        }
        break;
    }
  }

  // Kiểm tra timeout
  if (millis() - lastActivityTime > timeout) {
    Serial.println("Không có thao tác trong 1 phút, tự động chuyển sang sleep...");
    DS3231_Module_Close(); //lưu địa chỉ con trỏ stack
    Scale_Close();
    enterSleep(); // thực hiện sleep cho esp32
  }
}

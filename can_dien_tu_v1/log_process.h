#ifndef LOG_PROCESS_H
#define LOG_PROCESS_H


#include <Arduino.h>
#include <Wire.h>

/*
AT24C32 trong DS3231 có tổng cộng 4.096 byte bộ nhớ EEPROM và có 3 byte đầu có ý nghĩa đăc biệt:
- byte số 0: lưu giá trị cờ (flag) để đánh dấu 1 số thứ nếu cần
- byte số 1: lưu giá trị byte cao địa chỉ con trỏ stack 
- byte số 2: lưu giá trị byte thấp địa chỉ con trỏ stack
	(vì con trỏ có 16 bit ứng với 2 byte)
*/


//*************** các kiểu dữ liệu cần thiết ***************//
// Cấu trúc thời gian
typedef struct {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t weekday; // range từ 1 -> 7
	uint8_t day; // range từ 01 -> 31
	uint8_t month;
	uint8_t year; //range từ 00 -> 99
} Time;

// Cấu trúc lưu dữ liệu cân nặng kèm thời gian
typedef struct {
	float weight;     // Khối lượng cân được
	Time timestamp;   // Thời điểm cân
} WeightLog;

// Cấu trúc lưu dữ liệu cân nặng dạng float vào eeprom
union {
		float floatVal;
		uint8_t bytes[4];
} FloatToByte;

// Cấu trúc lấy dữ liệu cân nặng dạng float từ eeprom
union {
	uint8_t bytes[4];
	float floatVal;
} ByteToFloat;

//*************** các hàm nội bộ dùng trong thư viện ***************//
//Các hàm nội bộ cho thời gian
uint8_t Decimal2BCD(uint8_t val); // Chuyển đổi từ hệ thập phân sang BCD của RTC module
uint8_t BCD2Decimal(uint8_t val); // Chuyển đổi từ BCD sang hệ thập phân


// Các hàm nội bộ cho bộ nhớ EEPROM
void eepromWriteByte(uint16_t addr, uint8_t data); // Viết dữ liệu theo từng Byte vào EEPROM
uint8_t eepromReadByte(uint16_t addr); // Đọc dữ liệu theo từng Byte vào EEPROM

void eepromWriteFloat(uint16_t addr, float val); // Viết dữ liệu cân nặng
float eepromReadFloat(uint16_t addr); // Đọc dữ liệu cân nặng
void eepromWriteTime(uint16_t addr, Time* time); // Viết dữ liệu thời gian
Time eepromReadTime(uint16_t addr); // Đọc dữ liệu thời gian



//*************** các hàm thực hiện chức năng ***************//
// Các hàm khởi tạo
void DS3231_Module_Init(); // Khởi tạo I2C hoặc thiết lập ban đầu
void DS3231_Module_Close(); // kết thúc khi vào chế độ sleep

void InitialTime(); // Khởi tạo thời gian ban đầu
void resetInitialTime(); // Reset flag thời gian nếu cần khởi tạo lại thời gian


// Các hàm quản lý thời gian
void setTime(Time* time);	// Cài đặt thời gian cho DS3231
void getTime(Time* time);	// Đọc thời gian hiện tại từ DS3231


// Các hàm quản lý log EEPROM (lịch sử cân)
void writeWeightLog(uint16_t addr, WeightLog* log); // Viết log cân nặng (4 byte)
float readWeightLog(uint16_t addr, WeightLog* log); // Đọc log cân nặng (4 byte)

void clearAllLogs(); // Xóa toàn bộ log 
void clearLastLog(); // Xóa log ghi mới nhất
uint16_t getLastLogAddress(); // Lấy địa chỉ log mới nhất


// Các hàm tiện ích
uint16_t getTotalLogs();            // Lấy số bản ghi đã lưu
bool isStorageFull();               // Kiểm tra ROM đầy chưa
void printLog(WeightLog* log);  		// In ra log qua Serial (để debug)
void printAllLogs(); 								// In tất cả log qua Serial



#endif
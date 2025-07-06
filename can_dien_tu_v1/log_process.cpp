#include "log_process.h"

//*************** các kiểu dữ liệu cần thiết ***************//
// các define
#define EEPROM_SIZE 4096 		 // eeprom có tối đa 4 Kbyte => 4096 byte
#define LOG_SIZE 11					 // 4 byte đầu lưu weight, 7 byte sau lưu thời gian cân
#define INIT_FLAG_ADDR 0     // đặt địa chỉ byte đầu EEPROM lưu cờ đánh dấu khi khởi tạo
#define INIT_FLAG_VALUE 0x01 // giá trị đánh dấu đã khởi tạo giá trị thời gian ban đầu (Initial Time) (0000 0001)

// các biến toàn cục
uint16_t nextLogAddr;				 // địa chỉ con trỏ tiếp theo trong eeprom (con trỏ stack)


//*************** các hàm nội bộ dùng trong thư viện ***************//
// Các hàm nội bộ cho thời gian
uint8_t Decimal2BCD(uint8_t val){
	return (val/10)<<4 | (val%10);
}

uint8_t BCD2Decimal(uint8_t val){
	return (val>>4)*10 + (val&0x0F);
}


// Các hàm nội bộ cho bộ nhớ EEPROM
//// 1. Các hàm nội bộ bậc 1 (lưu vào EEPROM theo từng byte)

/**
 * @brief Ghi một byte dữ liệu vào địa chỉ xác định trong bộ nhớ EEPROM của DS3231 (AT24C32).
 * 
 * Hàm sử dụng giao tiếp I2C để truyền byte dữ liệu `data` vào địa chỉ `addr` trong EEPROM.
 * Địa chỉ gồm 2 byte (16-bit), do đó được chia thành byte cao và byte thấp để gửi đi.
 * Sau khi truyền, cần delay một khoảng thời gian để EEPROM hoàn tất quá trình ghi (~5ms).
 * 
 * @param addr Địa chỉ 16-bit trong EEPROM nơi sẽ ghi dữ liệu.
 * @param data Byte dữ liệu cần ghi vào EEPROM tại địa chỉ đã cho.
 */
void eepromWriteByte(uint16_t addr, uint8_t data){
	Wire.beginTransmission(0x57);
	Wire.write((addr >> 8) & 0xFF);  // Byte cao của địa chỉ
	Wire.write(addr & 0xFF);         // Byte thấp của địa chỉ
	Wire.write(data);                // Dữ liệu cần ghi
	Wire.endTransmission();

	delay(5);  // EEPROM cần thời gian để hoàn tất ghi (5ms là an toàn)
}


/**
 * @brief Đọc một byte dữ liệu từ địa chỉ xác định trong bộ nhớ EEPROM của DS3231 (AT24C32).
 * 
 * Hàm sử dụng giao tiếp I2C để truy xuất dữ liệu từ địa chỉ `addr` trong EEPROM.
 * Đầu tiên gửi địa chỉ cần đọc (2 byte: byte cao và byte thấp), sau đó yêu cầu đọc 1 byte từ EEPROM.
 * Nếu dữ liệu sẵn sàng, sẽ trả về byte đã đọc. Nếu không có dữ liệu, trả về giá trị mặc định `0xFF`.
 * 
 * @param addr Địa chỉ 16-bit trong EEPROM nơi cần đọc dữ liệu.
 * @return uint8_t Byte dữ liệu đọc được từ EEPROM; trả về 0xFF nếu không thành công.
 */
uint8_t eepromReadByte(uint16_t addr){
	Wire.beginTransmission(0x57);
	Wire.write((addr >> 8) & 0xFF);  // Byte cao của địa chỉ
	Wire.write(addr & 0xFF);         // Byte thấp của địa chỉ
	Wire.endTransmission();

	Wire.requestFrom(0x57, (uint8_t)1);
	if (Wire.available()) {
			return Wire.read();
	}
	return 0xFF; // Trả về giá trị mặc định nếu không đọc được
}


//// 2. Các hàm nội bộ bậc 2 (Lưu theo chuỗi byte) (gọi hàm nội bộ bậc 1 để lưu)
void eepromWriteFloat(uint16_t addr, float val){
	FloatToByte.floatVal = val;

	for (uint8_t i = 0; i < 4; i++) {
			eepromWriteByte(addr + i, FloatToByte.bytes[i]);
	}
}

float eepromReadFloat(uint16_t addr){
	for (uint8_t i = 0; i < 4; i++) {
			ByteToFloat.bytes[i] = eepromReadByte(addr + i);
	}

	return ByteToFloat.floatVal;
}

void eepromWriteTime(uint16_t addr, Time* time){
	eepromWriteByte(addr + 0, time->sec);
	eepromWriteByte(addr + 1, time->min);
	eepromWriteByte(addr + 2, time->hour);
	eepromWriteByte(addr + 3, time->weekday);
	eepromWriteByte(addr + 4, time->day);
	eepromWriteByte(addr + 5, time->month);
	eepromWriteByte(addr + 6, time->year);
}

Time eepromReadTime(uint16_t addr){
	Time time;

	time.sec     = eepromReadByte(addr + 0);
	time.min     = eepromReadByte(addr + 1);
	time.hour    = eepromReadByte(addr + 2);
	time.weekday = eepromReadByte(addr + 3);
	time.day     = eepromReadByte(addr + 4);
	time.month   = eepromReadByte(addr + 5);
	time.year    = eepromReadByte(addr + 6);

	return time;
}



//*************** các hàm thực hiện chức năng ***************//
//// 1. Các hàm khởi tạo

void DS3231_Module_Init(){ // Khởi tạo I2C hoặc thiết lập ban đầu
	Wire.begin();

	// Khởi tạo thời gian
	// resetInitialTime(); // BẬT KHI MUỐN ĐẶT LẠI GIỜ
	InitialTime();

	// Đọc địa chỉ ghi tiếp theo từ 2 byte (tại vị trí 1 và 2) trong EEPROM
	uint8_t highByte = eepromReadByte(1);
	uint8_t lowByte = eepromReadByte(2);
	nextLogAddr = (highByte << 8) | lowByte;

	// Nếu EEPROM chưa từng ghi (giá trị mặc định là 0xFFFF) hoặc sắp hết bộ nhớ
	if (nextLogAddr == 0xFFFF || nextLogAddr >= 4096 - LOG_SIZE) {
			nextLogAddr = 3; // bắt đầu ghi sau 3 byte đầu tiên
	}

}


void DS3231_Module_Close(){
	// Lưu giá trị địa chỉ tiếp theo vào eeprom để sau lấy lại
	eepromWriteByte(1, (nextLogAddr >> 8) & 0xFF);  // Byte cao
	eepromWriteByte(2, nextLogAddr & 0xFF);         // Byte thấp
}


void InitialTime() {
	Time Set_IniTime;

  uint8_t flag = eepromReadByte(INIT_FLAG_ADDR);

  if (flag != INIT_FLAG_VALUE) {
    Set_IniTime.sec = 00;
    Set_IniTime.min = 57;
    Set_IniTime.hour = 19;
    Set_IniTime.day = 12;
    Set_IniTime.month = 5;
    Set_IniTime.year = 25;
    Set_IniTime.weekday = 22;
    setTime(&Set_IniTime);
    eepromWriteByte(INIT_FLAG_ADDR, INIT_FLAG_VALUE);
  }
}

void resetInitialTime() {
	Serial.printf("cờ flag trước: %u\n", eepromReadByte(INIT_FLAG_ADDR));
  eepromWriteByte(INIT_FLAG_ADDR, 0x00); // Xóa cờ, cho phép gọi lại InitialTime() khi chạy lại hàm Init
	Serial.printf("cờ flag sau: %u\n", eepromReadByte(INIT_FLAG_ADDR));
}


//// 2. Các hàm quản lý thời gian

void setTime(Time* time){
	Wire.beginTransmission(0x68); // Địa chỉ DS3231
	Wire.write(0x00); // Thanh ghi bắt đầu

	Wire.write(Decimal2BCD(time->sec));
	Wire.write(Decimal2BCD(time->min));
	Wire.write(Decimal2BCD(time->hour));
	Wire.write(Decimal2BCD(time->weekday));
	Wire.write(Decimal2BCD(time->day));
	Wire.write(Decimal2BCD(time->month));
	Wire.write(Decimal2BCD(time->year));

	Wire.endTransmission();
};


void getTime(Time* time){
	Wire.beginTransmission(0x68);	// Địa chỉ DS3231
	Wire.write(0x00); // Thanh ghi bắt đầu
	Wire.endTransmission(); // Trước khi đọc thì phải gửi tìm I2C có địa chỉ cần đọc

	Wire.requestFrom(0x68, 7); // Yêu cầu đọc 7 byte
	if (Wire.available() >= 7) {
		time->sec     = BCD2Decimal(Wire.read());
		time->min     = BCD2Decimal(Wire.read());
		time->hour    = BCD2Decimal(Wire.read());
		time->weekday = BCD2Decimal(Wire.read());
		time->day     = BCD2Decimal(Wire.read());
		time->month   = BCD2Decimal(Wire.read());
		time->year    = BCD2Decimal(Wire.read());
	}
};


//// 3. Các hàm quản lý log EEPROM (hàm bậc 3) (lưu dữ liệu thời gian và cân nặng) (gọi hàm bậc 2) 

void writeWeightLog(uint16_t addr, WeightLog* log){
	// Ghi float weight (4 byte)
	eepromWriteFloat(addr, log->weight);           // 0 -> 3

	// Ghi struct Time (7 byte)
	eepromWriteTime(addr + 4, &(log->timestamp));     // 4 -> 10

	// Dịch chuyển đến địa chỉ tiếp theo
	nextLogAddr = addr + LOG_SIZE;
}

float readWeightLog(uint16_t addr, WeightLog* log){
	if(addr >= 0){ // Kiểm tra vì addr thường truyền vào để đọc = nextLogAddr - i*LOG_SIZE (với i là vị trí log tính từ nextLogAddr đổ về trước) => có thể có giá trị âm khi addr == 3
		// Đọc float weight
		log->weight = eepromReadFloat(addr);           // 0 -> 3

		// Đọc struct Time
		log->timestamp = eepromReadTime(addr + 4);     // 4 -> 10

		return log->weight;
	}
	return -1;
}

void clearAllLogs(){
	/*
	thay vì xóa thì ta chỉ cần cập nhật vị trí con trỏ tiếp theo rồi ghi đè giá trị lưu trước đó là được 
	=> Do EEPROM thường giới hạn số lần ghi (~100,000 lần) nên làm thế để tiết kiệm số lần ghi
	*/

	// Cập nhật con trỏ địa chỉ tiếp theo 
	nextLogAddr = 3;
	eepromWriteByte(1, (nextLogAddr >> 8) & 0xFF);  // Byte cao
	eepromWriteByte(2, nextLogAddr & 0xFF);         // Byte thấp
}

void clearLastLog(){
	if(getTotalLogs() != 0){
		nextLogAddr = nextLogAddr - LOG_SIZE; //dịch con trỏ stack xuống (không xóa mà chỉ ghi đè)
		eepromWriteByte(1, (nextLogAddr >> 8) & 0xFF);  // Byte cao
		eepromWriteByte(2, nextLogAddr & 0xFF);         // Byte thấp
	}
};

uint16_t getLastLogAddress(){
	return nextLogAddr;
}


// Các hàm tiện ích
// Lấy số bản ghi đã lưu
uint16_t getTotalLogs(){
	return (nextLogAddr-3)/LOG_SIZE;
}    

// Kiểm tra ROM đầy chưa
bool isStorageFull(){
 	return (nextLogAddr + LOG_SIZE > EEPROM_SIZE); // nếu nextLogAddr tiếp theo bị vượt quá size eeprom tối đa
} 

// In ra log qua Serial (để debug)
void printLog(WeightLog* log){
	// in cân nặng
	Serial.print("Weight: ");
	Serial.print(log->weight, 2);
	Serial.print(" kg");

	// in thời gian
	Serial.print(" | Time: ");
	Serial.printf("%02d:%02d:%02d ", log->timestamp.hour, log->timestamp.min, log->timestamp.sec);
	Serial.printf("Date: %02d/%02d/20%02d", log->timestamp.day, log->timestamp.month, log->timestamp.year);

	// xuống dòng
	Serial.println();
}  		

// In tất cả log (bản ghi) trong EEPROM
void printAllLogs(){
	Serial.println("----------- Log List -----------");
	for (uint16_t addr = 3; addr + LOG_SIZE <= nextLogAddr; addr += LOG_SIZE) {
			WeightLog log;
			readWeightLog(addr, &log);
			printLog(&log);
	}
}

#include "scale_process.h"

//HX711
const int ld_cell = 19;
const int ld_sck = 18;
HX711 scale; // đối tượng scale kiểu hx711

//LCD
LCD_I2C lcd(0x27, 16, 2); // Default address PCF8574 modules

float weight;

void Scale_Init(){
  //lcd
  Wire.begin();
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();

  // Scale
  scale.begin(ld_cell, ld_sck);   //setup LCD
}

void Scale_Close(){
  lcd.backlightOff();
}


/**
 * @brief Thực hiện thao tác loại bỏ khối lượng bề mặt (tare) trên cân điện tử.
 * 
 * Hàm sẽ hiển thị thông báo hướng dẫn người dùng loại bỏ vật nặng (nếu có) khỏi bề mặt cân trong 5 giây.
 * Sau đó, tiến hành hiệu chỉnh tare để đưa cân về 0. Cuối cùng, thông báo rằng quá trình tare đã hoàn tất.
 * 
 * Các thông báo được hiển thị song song trên cả LCD và Serial Monitor để đảm bảo người dùng nhận được phản hồi.
 */
void tareScale(){ //tare weight
  lcd.print("Tare removes...");
  Serial.println("Tare removes...");
  delay(5000);
  lcd.clear();

  //tare
  scale.tare();

  lcd.print("tare done...");
  Serial.println("tare done...");
  delay(5000);
  lcd.clear();
}


/**
 * @brief Thực hiện đo khối lượng vật thể đặt lên cân điện tử.
 * 
 * Hàm sẽ hiển thị thông báo nhắc người dùng đặt vật cần cân lên bề mặt cân.
 * Sau đó tiến hành đọc giá trị từ load cell thông qua HX711 với 10 lần lấy mẫu,
 * mỗi lần cách nhau 200ms, mỗi mẫu bao gồm trung bình của 10 phép đọc liên tiếp từ HX711.
 * 
 * Giá trị cân trung bình sau đó được chuyển đổi theo hệ số hiệu chuẩn (scale factor)
 * và trả về dưới dạng khối lượng (đã lấy trị tuyệt đối).
 * 
 * @return float Giá trị khối lượng tính được sau khi cân, tính bằng đơn vị kg (hoặc phù hợp với hệ số scale).
 */
float weightScale(){ //scale weight
  lcd.print("put weight...");
  Serial.println("put weight...");
  delay(5000);
  lcd.clear();

  //scale 
  float sum = 0;
  uint8_t sample = 10;
  for (uint8_t i = 0; i < sample; i++) {
    sum += scale.get_units(10);  // lấy trung bình 10 mẫu mỗi lần đọc
    delay(200); // 0.2 giây giữa các lần lấy mẫu
  }
  float weight = abs(sum) / (19676*sample);  // chia hệ số scale tương ứng

  return weight;
}

void displayWeighResult(float weight){
  lcd.print("weight result...");
  Serial.println("weight result...");
  delay(5000);
  lcd.clear();

  //display weight in lcd
  char buff[7];
  dtostrf(weight, 2, 2, buff); // hàm chuyển float -> string
  Serial.println(buff);
  lcd.print(buff);
  delay(5000);
  lcd.clear();

  // lcd.print(scale.get_units(10));
  // Serial.println(scale.get_units(10));
  // delay(5000);
  // lcd.clear();
}

void startScaleProcess(){
  if(scale.is_ready()){ 
    //set scale
    scale.set_scale();

    //tare
    tareScale();

    //scale weight
    weight = weightScale();

    //display in uart
    displayWeighResult(weight);
  }
}

float getWeight(){
  return weight;
}

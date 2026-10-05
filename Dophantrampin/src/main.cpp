#include <Arduino.h>

#define VOLTAGE_PIN 36 // Chân GPIO kết nối với đầu S của cảm biến

// Cấu hình thông số mạch chia áp 
const float R1 = 30000.0; // 30k ohm
const float R2 = 7500.0;  // 7.5k ohm
float calibration_factor = 1.0; // Hệ số sai số (nếu đo thực tế bị lệch, hãy chỉnh số này, VD: 1.02 hoặc 0.98)

// Dải điện áp của hệ pin 2S (2 viên 18650 mắc nối tiếp)
const float MAX_BATTERY_VOLTAGE = 8.4; // 100% (Pin đầy)
const float MIN_BATTERY_VOLTAGE = 6.4; // 0% (Pin cạn)

void setup() {
  // Khởi tạo giao tiếp Serial với tốc độ baud 115200
  Serial.begin(115200);
  
  // Đợi một chút để cổng Serial ổn định
  delay(1000);
  Serial.println("Bắt đầu đọc điện áp pin...");
}

void loop() {
  // 1. Lọc nhiễu: Đọc 100 lần và lấy giá trị trung bình
  long adcSum = 0;
  int numSamples = 100;
  
  for (int i = 0; i < numSamples; i++) {
    adcSum += analogRead(VOLTAGE_PIN);
    delay(2); // Dừng 2ms giữa mỗi lần đọc để lấy mẫu đều hơn
  }
  
  float adcAverage = (float)adcSum / numSamples;

  // 2. Tính điện áp đầu ra tại chân ESP32 (Vout)
  float vOut = (adcAverage * 3.3) / 4095.0;

  // 3. Tính điện áp thực tế của pin (Vin)
  float vActual = vOut * ((R1 + R2) / R2) * calibration_factor;

  // 4. Chuyển đổi sang phần trăm pin (SOC)
  int batteryPercentage = 0;
  if (vActual >= MAX_BATTERY_VOLTAGE) {
    batteryPercentage = 100;
  } else if (vActual <= MIN_BATTERY_VOLTAGE) {
    batteryPercentage = 0;
  } else {
    // Nội suy tuyến tính
    batteryPercentage = ((vActual - MIN_BATTERY_VOLTAGE) / (MAX_BATTERY_VOLTAGE - MIN_BATTERY_VOLTAGE)) * 100;
  }

  // 5. In kết quả ra màn hình Terminal
  Serial.print("Vactual=");
  Serial.print(vActual);
  Serial.print("V, SOC=");
  Serial.print(batteryPercentage);
  Serial.println("%");

  // Dừng 1 giây trước khi đọc chu kỳ tiếp theo
  delay(1000); 
}
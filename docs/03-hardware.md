# Phần cứng và bảng GPIO chính thức

Nguồn: [đặc tả người dùng](requirements/2026-09-28-agv-firmware-request.txt). Tất cả chân active nằm duy nhất trong [Config.h](../NCKHAGV/src/Config.h). Bảng cũ chỉ còn ở archive, không dùng để đấu dây bản này.

Bảng từng chân để tra cứu: [Bảng phân chân toàn xe AGV](bang-phan-chan-agv.md).

| Tín hiệu | GPIO | Cấu hình |
| --- | --- | --- |
| LEFT ENA, IN1, IN2 | 14, 27, 26 | Hai motor trái chạy chung |
| RIGHT ENB, IN3, IN4 | 25, 33, 32 | Hai motor phải chạy chung |
| Line LEFT, CENTER, RIGHT | 39, 34, 35 | INPUT; không pull-up/down nội |
| HC-SR04 TRIG, ECHO | 12, 13 | OUTPUT trigger, INPUT echo interrupt |
| Battery B25 signal | 36 (VP) | ADC1 INPUT |
| RFID SCK, MISO, MOSI | 18, 19, 23 | SPI.begin đúng thứ tự |
| RFID SS, RST | 5, 22 | MFRC522 |
| Encoder LEFT, RIGHT | 4, 16 | Hai INPUT, interrupt RISING độc lập |
| Servo 1, 2, 3 | 15, 17, 21 | Chỉ bật servo đã xác nhận vai trò/calibration |
| Còi lùi | 0 | Ngõ điều khiển active LOW, cần mạch đệm trở kháng cao và kéo lên 3,3 V |
| IR hàng nhìn lên | 2 | INPUT active HIGH, mức tín hiệu tối đa 3,3 V |

static_assert kiểm tra 24 GPIO không trùng. Unity kiểm tra bảng chân và phân vùng timer PWM. Đây là kiểm tra source, không chứng minh dây thực tế đúng.

GPIO0 là chân BOOT: mạch còi phải có đầu vào trở kháng cao và pull-up đủ để GPIO0 luôn HIGH khi reset; còi không được cấp tải trực tiếp từ GPIO. Còi active LOW chỉ bật khi firmware đã cấp lệnh lùi cho **cả hai** motor, tắt khi PWM dừng. Nút BOOT của project thử không còn là nút bật/tắt motor trong firmware chính. GPIO2 là strapping: IR `HIGH` khi đang nạp firmware có thể cản chế độ download, vì vậy dùng jumper ngắt IR lúc nạp và kiểm tra khởi động khi IR HIGH/LOW. Không nối output 5 V của IR vào ESP32. Hai chân này là lựa chọn để giữ nguyên 22 chân cũ, chưa được thử điện trên đúng DevKit V1; nếu mạch thật không đáp ứng điều kiện boot/nạp thì phải đổi revision phần cứng, không cố đấu trực tiếp. Tham khảo [bảng chân và strapping của Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/boards/ESP32-DevKitC-1.html).

Người dùng xác nhận **ba mắt dò line lắp ở phía trước xe**. Đây là vị trí cơ khí, không thay bảng GPIO. Khi xe lùi khỏi kho, cảm biến ở phía sau theo hướng di chuyển; cần thử luật sửa lái chiều lùi riêng trước khi cho chạy trên mô hình.

## Driver và PWM

MotorModule giữ toàn bộ PWM motor: 5 kHz, 8 bit, channel 0..3. ServoModule dùng 50 Hz, 16 bit, channel 4/5/6. Theo cách ánh xạ LEDC của [Arduino ESP32 2.0.17](https://github.com/espressif/arduino-esp32/blob/2.0.17/cores/esp32/esp32-hal-ledc.c), hai nhóm dùng timer khác nhau. Vẫn phải đo waveform đồng thời.

L298N: ENA/ENB là PWM channel 0/1, IN1..IN4 là direction digital. Tháo jumper enable thích hợp để PWM có tác dụng, theo board driver thật.

BTS7960 không có giao diện IN1/IN2/ENA như L298N. Adapter đã định nghĩa rõ cho MỖI phía: ENA/ENB nối chung R_EN và L_EN của module; IN1/IN3 -> RPWM, IN2/IN4 -> LPWM. PWM dùng bốn chân IN, enable dùng digital. Chỉ chọn BTS7960 nếu dây đúng adapter và driver chịu tổng dòng hai motor cùng phía; không mặc định hai driver có thể thay trực tiếp. Nếu board driver khác, giữ Unconfigured và bổ sung adapter sau khi có sơ đồ.

motorStop đặt PWM=0 và ngõ hướng/enable thấp. Phanh/coast và quãng dừng phụ thuộc driver, tải, cơ khí; không coi đây là ngắt nguồn vật lý.

## Kiểm tra điện bắt buộc

- HC-SR04 ECHO có thể 5 V: phải dùng chuyển mức/chia áp đã kiểm tra trước GPIO13; ESP32 không được giả định chịu 5 V.
- GPIO34/35/36/39 input-only, không pull nội; cảm biến line/encoder phải có mức 3,3 V hợp lệ và điện trở ngoài nếu output cần. [GPIO reference Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html).
- GPIO5/12/15 là strapping trên ESP32 classic; kiểm tra ngoại vi không ép sai mức khi reset. GPIO12 nối TRIG cần đặc biệt đối chiếu module/nguồn và boot, không tự đổi chân để né vấn đề.
- GPIO16/17 cần board ESP32-WROOM phù hợp, không mặc định dùng được với mọi module có PSRAM. Xác nhận DevKit V1 38 chân vật lý.
- B25/chia áp phải đưa ADC36 vào dải an toàn ESP32 trước hiệu chuẩn; attenuation trong code không phải bảo vệ quá áp.
- Nguồn motor/servo đủ dòng khởi động, mass và bảo vệ đúng sơ đồ; kiểm tra RC522 3,3 V. Không cấp motor/servo tải từ GPIO.
- Nút dừng cứng, bảo vệ dòng và trạng thái driver lúc ESP32 reset/mất nguồn phải được thiết kế riêng; thêm pull-down/enable phù hợp theo sơ đồ đã duyệt.

## Hiệu chuẩn chưa có

PPR, đường kính bánh, UID, cực tính line, vai trò/góc/pulse servo, hệ số B25, driver và ngưỡng dừng đều giữ TODO. Xem [checklist](06-configuration.md). Single-channel encoder chỉ đếm cạnh, không phân biệt tiến/lùi; averageDistanceCm là trung bình travel hai phía, không phải odometry có hướng. Ngưỡng sai số và timing nghiệm thu giữ TBD tại [QA](../qa/test-plan.md).

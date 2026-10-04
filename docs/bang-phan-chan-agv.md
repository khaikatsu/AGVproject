# Bảng phân chân toàn xe AGV

Ngày xuất: 2026-10-03. Board dự kiến: ESP32 DevKit V1 38 chân, module ESP32-WROOM. Bảng lấy từ [Config.h hiện tại](../NCKHAGV/src/Config.h), driver đang chọn L298N. Đây là phân chân trong firmware, chưa phải kết quả kiểm tra dây trên xe thật.

Các số trong bảng là **số GPIO**, không phải thứ tự chân vật lý trên board. Bảng gồm 24 GPIO không trùng nhau.

## Chân tín hiệu

| Thiết bị | Chân trên thiết bị | GPIO ESP32 | Chức năng / mức tín hiệu |
| --- | --- | --- | --- |
| L298N bên trái | ENA | 14 | PWM tốc độ bánh trái |
| L298N bên trái | IN1 | 27 | Điều khiển chiều motor trái |
| L298N bên trái | IN2 | 26 | Điều khiển chiều motor trái |
| L298N bên phải | ENB | 25 | PWM tốc độ bánh phải |
| L298N bên phải | IN3 | 33 | Điều khiển chiều motor phải |
| L298N bên phải | IN4 | 32 | Điều khiển chiều motor phải |
| Mắt line trái | OUT | 39 (VN) | INPUT, line đen = HIGH |
| Mắt line giữa | OUT | 34 | INPUT, line đen = HIGH |
| Mắt line phải | OUT | 35 | INPUT, line đen = HIGH |
| HC-SR04 | TRIG | 12 | OUTPUT, phát xung đo |
| HC-SR04 | ECHO | 13 | INPUT, nhận xung qua chuyển mức/chia áp |
| Encoder trái | OUT | 4 | INPUT, đếm cạnh lên |
| Encoder phải | OUT | 16 | INPUT, đếm cạnh lên |
| RFID RC522 | SCK | 18 | SPI clock |
| RFID RC522 | MISO | 19 | Dữ liệu từ RC522 về ESP32 |
| RFID RC522 | MOSI | 23 | Dữ liệu từ ESP32 tới RC522 |
| RFID RC522 | SDA / SS | 5 | SPI chip-select; không phải SDA của I2C |
| RFID RC522 | RST | 22 | Reset RC522 |
| Servo 1 | Signal | 15 | Servo quét theo vai trò cấu hình hiện tại |
| Servo 2 | Signal | 17 | Servo nâng/hạ thứ nhất |
| Servo 3 | Signal | 21 | Servo nâng/hạ thứ hai |
| Cảm biến điện áp pin B25 | OUT | 36 (VP) | ADC1 đo điện áp sau chia áp/hiệu chuẩn |
| Còi cảnh báo lùi | Chân điều khiển | 0 | Active LOW: LOW bật, HIGH tắt, qua mạch điều khiển phù hợp |
| IR hàng hướng lên | OUT | 2 | INPUT: HIGH có hàng, LOW không có hàng |

Hai servo nâng/hạ dùng hai GPIO riêng để hiệu chuẩn góc từng con và hoạt động cùng lúc. Không thay đổi GPIO để dùng chung một chân trong lần xuất bảng này. Servo hiện còn bị tắt trong cấu hình, góc/pulse/thời gian chuyển động chưa được hiệu chuẩn.

IR hướng lên có thể nhìn thấy hàng trên kệ trước khi nâng hoặc hàng trên khay sau khi lấy; riêng HIGH ở kệ chưa chứng minh hàng đã nằm trên xe. Còi được bật khi firmware thực sự cho cả hai motor lùi và tắt khi dừng; chế độ lùi vẫn đang khóa chờ kiểm thử phần cứng.

## Nguồn và lưu ý đấu dây

- ESP32, driver, cảm biến và nguồn servo/motor phải có GND chung theo sơ đồ nguồn đã kiểm tra. Cấp nguồn motor/servo đủ dòng từ nguồn phù hợp; không cấp tải từ GPIO. RC522 dùng nguồn 3,3 V.
- Không đưa tín hiệu 5 V trực tiếp vào ESP32. Kiểm tra mức OUT line, encoder, IR và B25; ECHO HC-SR04 phải qua mạch chuyển mức hoặc chia áp phù hợp.
- GPIO34/35/36/39 chỉ làm ngõ vào, không có pull-up/down nội. Dùng điện trở ngoài nếu loại ngõ ra cảm biến yêu cầu. [Tài liệu GPIO Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html).
- GPIO0 là chân BOOT. Mạch còi active LOW cần đầu vào trở kháng cao và pull-up 3,3 V phù hợp, không kéo GPIO0 LOW lúc reset. GPIO2 là strapping: cần cách ngắt tín hiệu IR khỏi GPIO2 khi nạp nếu IR đang HIGH. Phải thử boot/nạp ở cả hai mức IR trước khi dùng. [Tài liệu boot ESP32](https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html).
- GPIO5/12/15 cũng là strapping; kiểm tra ngoại vi không ép sai mức khi reset. GPIO16/17 cần xác nhận đúng module WROOM, không mặc định dùng được trên module có PSRAM.
- Tháo jumper ENA/ENB phù hợp trên L298N khi dùng PWM từ ESP32; kiểm tra theo board driver thực tế. Công suất driver phải chịu được tổng tải motor mỗi bên.

Chi tiết nguồn, PWM, điều kiện boot và các mục chưa kiểm chứng nằm trong [tài liệu phần cứng](03-hardware.md). Bảng này không tự xác nhận điện áp cấp nguồn hoặc màu dây của từng module.

## Phạm vi kiểm chứng

Đã đối chiếu bảng với 24 hằng GPIO trong Config.h. Không sửa firmware, không nạp lên board và không chạy cơ cấu trong lần xuất này. Kiểm tra điện, boot, chiều motor, PWM, servo và cảm biến trên xe thật: **NOT_RUN**.

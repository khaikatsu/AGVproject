# Nguồn tham khảo GitHub

Đã truy cập các trang nguồn ngày 2026-09-28. Workspace vận dụng cách tổ chức và giới hạn kỹ thuật; không vendor/clone mã upstream. README nhánh hiện tại có thể khác phiên bản đang cài, vì vậy không dùng latest README để chứng minh API của bản đã resolve. Kiểm tra LICENSE đúng revision khi sao chép mã trong tương lai.

| Nguồn gốc | Phần tham khảo | Áp dụng trong workspace |
| --- | --- | --- |
| [platformio/platform-espressif32](https://github.com/platformio/platform-espressif32) | PlatformIO project, examples, khuyến nghị pin platform | Giữ project firmware hiện có; lệnh build và baseline phiên bản |
| [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) | Core Arduino chính thức | Hướng dẫn đọc API đúng core và kiểm tra GPIO/PWM |
| [ThrowTheSwitch/Unity](https://github.com/ThrowTheSwitch/Unity) | Unit testing C cho embedded | Đã dùng native Unity 2.6.1 trên FSM/parser production, không on-device upload |
| [knolleary/pubsubclient](https://github.com/knolleary/pubsubclient) | MQTT examples và giới hạn publish/buffer | Contract QoS 0, giới hạn payload và rủi ro bảo trì |
| [miguelbalboa/rfid](https://github.com/miguelbalboa/rfid) | MFRC522 SPI và trạng thái phát triển | Pin map, ca thử UID/đọc lặp; giữ version hiện tại |
| [madhephaestus/ESP32Servo](https://github.com/madhephaestus/ESP32Servo) | PWM/servo, attach và tài nguyên phần cứng | Ca thử xung motor và servo cùng lúc |
| [open-rmf/rmf_demos](https://github.com/open-rmf/rmf_demos) | Ví dụ quản lý nhiệm vụ, map, fleet, tài nguyên dùng chung | Tham khảo phân ranh robot/nhiệm vụ/bản đồ; chưa đưa ROS/Open-RMF vào stack |

## Ghi nhận quan trọng

PubSubClient README hiện thông báo dự án không còn duy trì; thư viện chỉ publish QoS 0. MFRC522 README mô tả đóng băng tính năng và bảo trì không thường xuyên. ESP32Servo README nhánh hiện tại nêu Arduino ESP32 3.0.0 trở lên; đó không phải bằng chứng cho ESP32Servo 3.2.1/core 2.0.17 đang build được tại máy này. Các thay đổi thư viện cần được đánh giá khi có yêu cầu cụ thể.

## Tài liệu chính thức bổ sung

- [PlatformIO unit testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html): phân biệt test native và on-device.
- [PlatformIO GitHub Actions](https://docs.platformio.org/en/latest/integration/ci/github-actions.html): build project qua CLI trong CI.
- [Arduino ESP32 migration 2.x to 3.0](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html): API LEDC thay đổi, cần đối chiếu trước nâng core.
- [ESP32 GPIO reference](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html): khả năng GPIO và các chân đặc biệt của ESP32 classic.

Phiếu đề tài và request firmware bổ sung là nguồn phạm vi. Active source nằm trong `NCKHAGV/src`; bản cũ trong archive. Không sao chép thông tin cá nhân từ phiếu sang mẫu GitHub.

## Nguồn cho firmware modular mới

- [Arduino ESP32 LEDC core 2.0.17](https://github.com/espressif/arduino-esp32/blob/2.0.17/cores/esp32/esp32-hal-ledc.c): đối chiếu API và ánh xạ channel/timer, ServoModule mới dùng native LEDC thay ESP32Servo.
- [ArduinoJson v6.21.5](https://github.com/bblanchon/ArduinoJson/tree/v6.21.5): parser JSON có giới hạn; phiên bản đã pin trong platformio.ini.
- [PubSubClient v2.8 source](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.cpp): connect/callback/buffer/socket timeout/publish QoS0 đúng phiên bản dùng.

Các trang này đã được kiểm tra lại ngày 2026-09-28. Cách tổ chức task/queue/FSM và adapter driver là triển khai của project; không tuyên bố upstream đã chứng nhận hardware AGV.

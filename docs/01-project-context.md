# Bối cảnh và hiện trạng dự án

## Nguồn phạm vi

Phiếu đăng ký đề tài Word ở thư mục gốc nêu hệ thống quản lý robot tự hành trong kho dựa trên IoT: hai robot, web thời gian thực, bản đồ, lộ trình, pin, lỗi và báo cáo NCKH. Workspace không sao chép thông tin liên hệ cá nhân. Lịch trong phiếu là tháng 2 đến tháng 10 năm 2026, cần xác nhận tiến độ thực tế.

Ngày 2026-09-28 người dùng bổ sung [đặc tả firmware đầy đủ](requirements/2026-09-28-agv-firmware-request.txt). Đặc tả này thay bảng GPIO cũ và yêu cầu ESP32 thực hiện path từ server, không chạy A-Star. Firmware cũ được lưu tại [baseline archive](../archive/firmware-baseline-2026-09-28/README.md); báo cáo baseline chỉ phản ánh bản cũ.

## Hiện trạng sau triển khai

| Thành phần | Đã triển khai trong code | Còn cần xác nhận |
| --- | --- | --- |
| MCU | PlatformIO esp32dev, Arduino core 2.0.17, C++17 | Board ESP32 DevKit V1 38 chân thực tế và sơ đồ nguồn |
| Motor | Hai phía LEFT/RIGHT, L298N, đảo chiều đã xác nhận trong profile, PWM=0 khi stop | Trim dưới tải, quãng dừng, chiều lùi |
| Line | Ba INPUT-only, line đen HIGH, 101 tiến và bốn pattern lệch chỉ chạy một bên | Đủ tám pattern trên xe production và bám line khi lùi |
| Siêu âm | Echo ISR, timeout/stale invalid, dừng motion | Chuyển mức ECHO, khoảng dừng và chất lượng đo |
| Encoder | Hai IRQ trái/phải, atomic snapshot/reset, cm sau calibration | PPR, đường kính bánh, nhiễu và sai số |
| RFID | 26 tên node (B1/B2 tạm), sự kiện node và reset đoạn | UID thật, tên base, bố trí tag và khả năng đọc |
| Servo | Ba chân, tách timer PWM, hai servo nâng có góc riêng, không đặt góc boot | Pulse/góc/nguồn/settling và đồng bộ cơ khí |
| IR/còi | GPIO2 IR hàng HIGH, GPIO0 còi active LOW khi hai motor lùi | Điện áp, boot/nạp, âm lượng và thử phần cứng |
| Pin | BatteryModule ADC1, unknown trả null | B25, calibration và range pin; chưa low-battery policy |
| MQTT | Task mạng riêng, JSON/session/ACK, queue, SAFE_STOP latch | Broker, topics, bảo mật và thử reconnect thật |
| Nhiệm vụ | FSM chờ NODE_ORDER cấp trước, server cho phép nâng/hạ, lùi về RFID trước, báo mất hàng | Backend/map, lùi đang khóa QA, route và thử xe |
| QA | Native Unity kiểm thử ControlLogic/CommandCodec production | GPIO ISR/LEDC/task mạng, latency và bàn thử còn NOT_RUN |
| Hệ thống kho | Tài liệu yêu cầu và mẫu QA | Backend, web, A-Star server, điều phối hai xe chưa triển khai |

Các sentinel trong Config.h là cố ý: build được nhưng không cho motion khi cấu hình chưa đầy đủ. Xem [checklist cấu hình](06-configuration.md) và [bàn giao từng file](firmware-delivery.md).

## Trạng thái yêu cầu

SOURCED: có mục tiêu trong phiếu. DRAFT: đề xuất. ACCEPTED: đã chốt nội dung. IMPLEMENTED: có triển khai, chưa đồng nghĩa nghiệm thu trên xe. VERIFIED: có bằng chứng đầy đủ cho tiêu chí. DEFERRED: đã thống nhất lùi. Kết quả thử riêng dùng PASS/FAIL/BLOCKED/NOT_RUN.

Không tự suy ra ngưỡng tải, latency hoặc sai số chưa được cung cấp. Yêu cầu bổ sung qua [biểu mẫu](requirements/README.md); những giá trị cần đo giữ TBD.

# Cấu hình trước khi chạy xe

Mọi giá trị cần điền nằm trong [Config.h](../NCKHAGV/src/Config.h), dưới comment `// TODO: USER CONFIGURATION`. Các sentinel rỗng, 0, -1 và false giúp project compile, không phải số đo thực tế. 22 GPIO cũ giữ nguyên; GPIO0 còi và GPIO2 IR là revision mới cần thử boot/nạp. Motor L298N, đảo chiều hai bên, line HIGH, PWM thử 210/trim trái -20 đã lấy từ AGV-PROFILE-001 nhưng chưa đo dưới tải thật.

| Nhóm | Bạn cần điền | Kiểm tra trước khi xác nhận |
| --- | --- | --- |
| Motor | Kiểm lại L298N, đảo chiều hai bên, PWM 210/trim trái -20 | Kê bánh, thử từng phía, đo PWM và đường thẳng dưới tải trước khi coi là calibration |
| Line | HIGH đã xác nhận cho bộ mắt thử; luật `101` đã đổi trong production | Đo đủ tám pattern trên xe thật; không dùng pull-up nội GPIO34/35/39 |
| Vật cản | OBSTACLE_DISTANCE_CM > 0 | Đo khoảng dừng dưới tải; HC-SR04 invalid/no-echo luôn chặn chạy |
| Encoder | ENCODER_PPR > 0, WHEEL_DIAMETER_CM > 0 | PPR là số cạnh RISING trên một vòng bánh, tính cả tỉ số truyền nếu đo ở trục motor; đo cả hai phía |
| RFID | RFID_NODES: UID thật cho 26 node; chốt tên hai base B1/B2 với server | Hex chữ hoa không khoảng trắng, 8/14/20 ký tự; không trùng UID; chưa có UID nào trong source |
| Servo | SERVO_ENABLED index 0/1/2 -> GPIO15 quét, GPIO17/21 nâng | Chỉ bật sau khi xác nhận nguồn, dây, tải; hai servo nâng phải cùng hoạt động mà không kẹt |
| Servo cơ khí | SERVO_MIN/MAX_PULSE_US, hai mảng SERVO_LIFT_UP/DOWN_ANGLES, MOVE_TIME_MS | Góc riêng hai bên, pulse và thời gian settling đo dưới tải; không có limit switch |
| IR/còi | GPIO2 IR HIGH, GPIO0 còi active LOW, lọc 150 ms | Chuyển mức 3,3 V, pull-up boot GPIO0, ngắt IR GPIO2 lúc nạp nếu HIGH; thử còi tắt ở boot/stop/lỗi |
| Lùi | REVERSE_STEERING_CONFIRMED = true chỉ sau TC-034 | Hiện đang false nên lệnh BACKWARD bị chặn; mắt line đầu xe chưa thử sửa lái khi lùi |
| Pin B25 | BATTERY_VOLTAGE_SCALE/OFFSET, EMPTY/FULL_VOLTAGE | Đo đồng hồ nhiều mức, scale của toàn mạch chia áp; giá trị ADC không phải điện áp pin. Phần trăm tuyến tính chỉ là ước lượng, không phải SOC đo được |
| Mạng | AGV_ID, WIFI_SSID/PASSWORD, MQTT_SERVER/PORT, USERNAME/PASSWORD nếu dùng | ID riêng cho mỗi xe; broker thật, ACL và LAN kiểm soát; không đưa credential thật lên GitHub |
| Topic | COMMAND, STATUS, POSITION, DISTANCE, EVENT, ERROR | Sáu topic cụ thể, khác nhau, không wildcard, dài tối đa 128 ký tự; thống nhất với server |
| Mất mạng | WIFI_TIMEOUT_MS, MQTT_TIMEOUT_MS | 0 nghĩa là SAFE_STOP ngay khi phát hiện mất kết nối. Dù có grace, motor vẫn dừng ngay và nhiệm vụ chuyển WAITING; không tự tiếp tục |
| Ngã rẽ | TURN_MIN_MS, TURN_TIMEOUT_MS, STRAIGHT_DEPARTURE_MS nếu dùng departure | Đo trên đường thật. LEFT/RIGHT chỉ tìm center sau khoảng tối thiểu và có timeout; STRAIGHT cho phép qua vạch giao nhau trong cửa sổ đã đo nhưng không bỏ qua mất line 000 |

## Thông số phần mềm đã có

Các giá trị PWM, retry, khoảng lấy mẫu, giới hạn payload/path/queue là mặc định kỹ thuật, không phải hiệu chuẩn phần cứng. Timeout echo 25.000 us, trigger 10 us, poll RFID 30 ms; cần đo ngân sách vòng điều khiển với RC522, Serial và ADC thật. Task mạng có heartbeat stale 2 s; mất broker còn phụ thuộc cơ chế phát hiện TCP/keepalive, không thể suy ra thời gian dừng vật lý chỉ từ MQTT_TIMEOUT_MS.

## Quy trình vận hành bàn thử

1. Đối chiếu [phần cứng](03-hardware.md), mạch nguồn, chuyển mức, nút dừng cứng và đồ gá. Điền cấu hình đã có bằng chứng; build lại.
2. Boot không cấp tải motor; firmware không tự đặt góc servo và không tự chạy. Thiếu mạng với timeout 0 sẽ vào SAFE_STOP.
3. Quét node đầu tiên của path; đọc status và session MQTT hiện tại. Nếu đang SAFE_STOP, xử lý nguyên nhân rồi gửi RESTART hợp lệ. RESTART chỉ xóa latch, không chạy.
4. Gửi TASK đã tính trên server, chờ AGV_COMMAND_RESULT accepted. Cấp NODE_ORDER cho node hiện tại/tiếp theo trước khi xe tới; gửi START riêng. Node đầu path phải khớp RFID hiện tại; mọi node trên route phải có mapping thật.
5. Tại pickup motor dừng, gửi AGV_PICKUP_IR; server chỉ gửi LIFT_UP khi có hàng. Sau nâng cần lệnh rời kho, lùi chỉ sau khi QA chiều lùi. Tại drop chờ LIFT_DOWN từ server; IR LOW ổn định mới COMPLETED.
6. STOP chuyển WAITING; nối lại mạng không tự chạy. SAFE_STOP cần RESTART rồi START. Nếu dừng giữa pha quay: CANCEL_TASK, định vị lại bằng RFID, RESTART và TASK/START mới; không đoán hướng còn dang dở.

Đọc [giao thức hiện hành](04-mqtt-contract-v11.md) trước khi gửi lệnh. Không upload hoặc bật motor chỉ để kiểm tra tài liệu. Chưa có nút local start hoặc serial command bypass, không có A-Star trên ESP32.

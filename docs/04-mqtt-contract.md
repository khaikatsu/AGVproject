# Giao thức MQTT firmware v1 (lưu lịch sử)

**Không dùng trang này để tích hợp bản firmware hiện tại.** Giao thức hiện hành sau thay đổi 2026-10-03 nằm ở [MQTT v1.1](04-mqtt-contract-v11.md). Các đoạn O1..O10, servo tự nâng/hạ và chỉ distanceCm bên dưới mô tả bản cũ.

Đây là schema mới triển khai theo yêu cầu 2026-09-28, không tương thích lệnh chuỗi RUN/CONTINUE của baseline. Không có broker/server thật trong workspace. Topic, ID, SSID và broker còn rỗng trong Config.h; không tự đặt tên cuối cùng.

## Topic

| Cấu hình | Chiều | Nội dung |
| --- | --- | --- |
| MQTT_TOPIC_COMMAND | Server -> AGV | JSON v1: TASK, START, STOP, EMERGENCY_STOP, RESTART, CANCEL_TASK, LIFT_UP, LIFT_DOWN |
| MQTT_TOPIC_STATUS | AGV -> server | State/error, node/distance, task/progress, motor/duty, Wi-Fi/MQTT, ultrasonic và pin |
| MQTT_TOPIC_POSITION | AGV -> server | Node RFID gần nhất và distanceCm từ node đó |
| MQTT_TOPIC_DISTANCE | AGV -> server | left/rightPulses, left/rightDistanceCm, average distanceCm |
| MQTT_TOPIC_EVENT | AGV -> server | AGV_REACHED_NODE, AGV_TASK_COMPLETED, AGV_COMMAND_RESULT |
| MQTT_TOPIC_ERROR | AGV -> server | Mã lỗi và LWT AGV_CONNECTION_LOST |

Sáu topic phải khác nhau, cụ thể, không wildcard, tối đa 128 ký tự. MQTT publish QoS 0, không retained; subscribe QoS 0. [PubSubClient v2.8 source](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.cpp) là cơ sở API. Không coi thành công publish là server đã lưu hoặc người dùng đã nhận.

## Command envelope

Mọi lệnh có version=1, id khác nhau, session (uint32), command viết hoa. id/task.id chỉ gồm A-Z, a-z, 0-9, dấu - _ ., dài 1..47 ký tự. Payload tối đa 1024 byte, path 1..32 node. Parser kiểm tra kiểu, node O1..O10, pickup có trong path, target là node cuối, không lặp liên tiếp. JSON lỗi/trailing data/NUL/lệnh không biết bị từ chối, không điều khiển motor.

Mỗi lần MQTT connect thành công tạo session nonce khác và báo trên status/event. Server phải đọc session mới trước TASK/START/RESTART/LIFT/CANCEL_TASK. Ví dụ 123 bên dưới chỉ minh họa, phải thay bằng session thật:

```json
{"version":1,"id":"cmd-001","session":123,"command":"TASK","task":{"id":"task-001","startNode":"O3","targetNode":"O10","path":["O1","O3","O5","O8","O10"]}}
```

startNode là ĐIỂM LẤY HÀNG, không bắt buộc node đầu path. Xe phải được RFID xác nhận đang ở node đầu path (O1 trong ví dụ). Nhận TASK chỉ WAITING, không tự chạy. Mọi node route phải có UID đã cấu hình.

```json
{"version":1,"id":"cmd-002","session":123,"command":"START"}
```

Server có thể dùng step object để chỉ hướng rời node:

```json
{"node":"O3","departure":"LEFT"}
```

departure: FOLLOW (mặc định), LEFT, RIGHT, STRAIGHT. Hướng được áp dụng khi rời node đó. Server chịu trách nhiệm chọn hướng theo map và heading thật; firmware không tính A-Star hay suy ra hướng từ nhãn node. LEFT/RIGHT/STRAIGHT yêu cầu timing đã hiệu chuẩn; firmware chính cho `010/101/111` tiến và chỉ `000` gây LINE_LOST, kể cả lúc quay. Bản đồ 26 node và quãng đường mm giữa node đã được chốt làm yêu cầu tiếp theo nhưng schema MQTT v1 ở đây vẫn dùng `O1..O10` và cm.

Yêu cầu [REQ-021/022](requirements/2026-10-03-node-prefetch-reverse.md) chọn lệnh hướng cho node kế tiếp gửi **trước khi xe tới**, và đoạn lùi `K1 -> G1` xác nhận bằng RFID. Đây không phải command/schema v1 đã triển khai: v1 vẫn lấy `departure` từ TASK, không có lệnh `BACKWARD` theo node hay trạng thái chờ lệnh mới. Trước khi đổi contract cần định nghĩa node áp dụng, next node, task/sequence/session, thời hạn, ACK, xử lý lệnh đến muộn và đường lùi an toàn.

Yêu cầu [IR hàng/lệnh nâng-hạ server](requirements/2026-10-03-cargo-ir-server-lift.md) cũng **chưa thuộc schema v1**: xe cần gửi có/không có hàng ở pickup theo giai đoạn, server cấp quyền nâng; tại drop server cấp quyền hạ; khi đang chở mà IR mất hàng ổn định thì firmware dừng motor/SAFE_STOP và phát lỗi. `LIFT_UP`/`LIFT_DOWN` v1 là lệnh thủ công chỉ khi không có task active, không được dùng nguyên xi trong task. Tên event/command, ACK, timeout và lọc IR còn TBD.

## Dừng và phục hồi

STOP và EMERGENCY_STOP vẫn có envelope đúng schema/id, nhưng cho phép session=0 và ưu tiên an toàn, không phụ thuộc dedup/session hiện tại. STOP -> WAITING/IDLE, motor dừng; EMERGENCY_STOP -> SAFE_STOP latch. Cờ stop được đặt ngay khi callback nhận lệnh, kể cả queue command đầy. Các lệnh thường còn trong queue cùng lượt stop bị từ chối.

Mất Wi-Fi/MQTT: motor dừng ngay khi phát hiện, WAITING trong grace; quá WIFI_TIMEOUT_MS/MQTT_TIMEOUT_MS chuyển SAFE_STOP ở mọi state. Timeout=0 latch ngay. Nối lại không tự tiếp tục. RESTART hợp lệ cần mạng, calibration motion, ultrasonic valid, không vật cản và line đọc được; chỉ về WAITING/IDLE. START là bước riêng. Quay bị gián đoạn yêu cầu CANCEL_TASK và định vị lại; CANCEL_TASK không xóa latch.

LIFT_UP/DOWN thủ công chỉ khi motor dừng, không task active, không SAFE_STOP/ERROR, servo không busy, calibration motion/mạng/sensor hợp lệ và góc LIFT đã cấu hình. START không được phép khi servo đang busy. SAFE_STOP giữ target servo đã phát; không cắt nguồn servo.

## Telemetry và ACK

Payload có version, agvId, session, uptimeMs. uptimeMs là millis ESP32, có wrap-around, không phải Unix/UTC. Giá trị chưa hiệu chuẩn/invalid là JSON null, không phải 0/999. Node rỗng là chưa xác nhận. batteryAdcMv là điện áp ADC, không phải điện áp pin; batteryPercentage tuyến tính sau calibration chỉ là ước lượng.

AGV_COMMAND_RESULT có id, accepted và error. accepted nghĩa là controller chấp nhận thao tác, không phải task đã hoàn thành; chờ AGV_TASK_COMPLETED và status/progress. Duplicate ID trong cửa sổ 16 lệnh gần nhất bị từ chối INVALID_COMMAND, không thực thi lại. Sau reconnect queue cũ bị từ chối bởi session; lệnh thường quá 2 s kể từ enqueue bị từ chối STALE_COMMAND. Đây không phải deadline do server ký hay cơ chế xác thực.

AGV_REACHED_NODE có agvId, node, distanceCm, left/rightDistanceCm, left/rightPulses, uptimeMs: khoảng cách đoạn vừa hoàn tất được chụp và reset hai counter atomically trước enqueue. Telemetry mới đo từ node vừa đến; đọc lặp cùng node không tạo event/reset. Unknown UID khi moving làm SAFE_STOP và vô hiệu vị trí cho đến RFID hợp lệ mới. Node ngoài thứ tự route làm SAFE_STOP.

Event/ACK/error nằm FIFO RAM 16 bản ghi; front publish fail được giữ lại cho reconnect. Telemetry là mailbox latest-only một snapshot. Queue overflow -> SAFE_STOP, không im lặng bỏ qua điều kiện lỗi. FIFO không bền qua reset, QoS0 vẫn có thể mất sau socket write thành công; muốn đảm bảo server lưu cần event ID/ACK application và persistence trong yêu cầu tiếp theo.

## Giới hạn tích hợp

PubSubClient/connect chạy trong task riêng nhưng phát hiện lỗi TCP/keepalive không tức thì; heartbeat stale 2 s không phải chứng nhận latency dừng. MQTT dùng WiFiClient plaintext, chưa TLS; username/password chỉ là tùy chọn broker auth. Session nonce không chống kẻ tấn công. Dùng LAN kiểm soát, ACL và nút dừng vật lý; MQTT emergency stop không thay thế mạch dừng an toàn. Mỗi xe cần topic/ID riêng và server kiểm tra độ mới dữ liệu.

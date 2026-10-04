# Giao thức MQTT firmware v1.1

Firmware dùng JSON `version:1` trên sáu topic trong `Config.h`: COMMAND (server -> xe), STATUS, POSITION, DISTANCE, EVENT và ERROR (xe -> server). Tên topic, broker và credential còn trống; mỗi xe cần ID/topic riêng. Publish/subscribe hiện QoS 0; ACK ứng dụng `AGV_COMMAND_RESULT` chỉ nói lệnh được chấp nhận, không chứng minh motor/servo đã hoàn thành hoặc server đã lưu dữ liệu.

## Envelope và nhiệm vụ

Mọi command có `version`, `id` duy nhất, `session` hiện tại và `command`. `STOP`/`EMERGENCY_STOP` được phép có session 0 và được ưu tiên. Payload tối đa 1024 byte, task path tối đa 32 node. ID dài 1..47 ký tự và chỉ gồm chữ ASCII, số, `-_.`. Parser từ chối JSON sai kiểu/NUL/dữ liệu thừa. 26 tên node hiện có trong `Config.h`: `K1..K6`, `C1..C4`, `M1..M6`, `G1..G6`, `GS1..GS2`, `B1/B2`; **B1/B2 là tên tạm** cho hai ô base, phải thống nhất server trước khi chạy. UID đều chưa có, vì vậy TASK chứa các node đó vẫn bị chặn tại controller cho đến khi điền mapping thật.

```json
{"version":1,"id":"c1","session":123,"command":"TASK","task":{"id":"t1","startNode":"K1","targetNode":"K2","path":["G1","K1","G1","M5","G2","K2"]}}
```

`startNode` là nơi lấy hàng, không bắt buộc là node đầu path. Ví dụ chỉ minh họa schema, **không xác nhận route hợp lệ theo mũi tên bản đồ**; server phải kiểm topology/heading và điều phối hai xe. Xe phải thấy RFID node đầu path, nhận TASK rồi START riêng:

```json
{"version":1,"id":"c2","session":123,"command":"START"}
```

## Lệnh theo node

Server gửi `NODE_ORDER` cho node sắp tới trước khi xe đến. `pathIndex` là chỉ số 0-based của node rời; `node` và `nextNode` phải khớp hai phần tử liên tiếp của TASK, `taskId` và MQTT session phải đúng. Một lệnh đang chờ được cache; sau khi RFID xác nhận node, lệnh hợp lệ được dùng ngay. Thiếu lệnh thì motor dừng ở WAITING; nếu lệnh đến sau và không có STOP/lỗi, xe mới tiếp tục. Duplicate ID hoặc cùng index bị từ chối; lệnh cũ sau reconnect hoặc quá 2 s trong queue bị từ chối. Đây là chống lệnh cũ, **không phải xác thực bảo mật**.

```json
{"version":1,"id":"c3","session":123,"command":"NODE_ORDER","taskId":"t1","node":"K1","nextNode":"G1","pathIndex":1,"departure":"BACKWARD"}
```

`departure` là LEFT, RIGHT, STRAIGHT hoặc BACKWARD. BACKWARD chỉ cho phép từ `K*` về đúng `G*` ngay trước kho trong path. Đến G1, motor dừng ít nhất `REVERSE_STOP_MS` rồi mới xét lệnh tiếp. Firmware production không dùng `departure` cũ nhúng trong TASK; parser vẫn đọc để tương thích dữ liệu. `REVERSE_STEERING_CONFIRMED=false` đang khóa chiều lùi do ba mắt line nằm đầu xe và chưa có thử bám line khi lùi. Nếu bật sau QA, bản hiện tại lùi thẳng khi line khác `000`, dừng với `000`; chưa có thuật toán sửa lái lùi đã kiểm chứng. Server làm A-Star, ESP32 không làm A-Star.

## IR và nâng/hạ

Ở pickup, motor dừng và IR hàng HIGH/LOW ổn định được gửi bằng event `AGV_PICKUP_IR` (`node`, `taskId`, `cargoPresent`). HIGH chỉ nói thấy vật đen trong vùng nhìn, không tự nâng. Server gửi lệnh đúng nhiệm vụ, node, index, session:

```json
{"version":1,"id":"c4","session":123,"command":"LIFT_UP","taskId":"t1","node":"K1","pathIndex":1}
```

`LIFT_DOWN` dùng cùng schema tại node giao. Firmware kiểm tra giai đoạn, RFID, IR HIGH trước lệnh, motor dừng, mạng, siêu âm và hai servo đã cấu hình. Sau nâng, HIGH có thể vẫn là hàng trên kệ; phải thử khi rời vùng kệ để chứng minh hàng trên khay. Trong pha chở, IR LOW ổn định `CARGO_IR_STABLE_MS` làm motor stop + `SAFE_STOP/CARGO_LOST`, phát ERROR. Sau hạ, chỉ COMPLETED khi IR LOW ổn định; còn HIGH thì đứng tại UNLOADING. IR đứt dây LOW không phân biệt với hàng rơi. Không có limit switch nên ACK và hết timer servo không chứng minh cơ cấu/hàng đã di chuyển đúng.

## Dừng, dữ liệu và giới hạn

STOP đưa PWM về 0 và WAITING/IDLE; EMERGENCY_STOP, vật cản, siêu âm invalid, line `000`, RFID sai hoặc mất mạng quá hạn chốt SAFE_STOP. Nối mạng lại không tự chạy. RESTART xóa latch khi interlock tốt, START là bước riêng. Quay hoặc servo bị gián đoạn yêu cầu CANCEL_TASK và định vị lại; servo đã phát xung không bị cắt nguồn khi SAFE_STOP.

STATUS có state/error, `node`, `nextNode`, `distanceMm`, `pathIndex`, motor/duty, cargoStable/cargoPresent, buzzerOn, kết nối, siêu âm và pin. POSITION có node/nextNode/distanceMm; DISTANCE có xung và cm/mm từ RFID trước. `AGV_REACHED_NODE` chứa `fromNode`, `node`, `reverse`, `distanceMm`, xung/quãng hai bên rồi reset counter atomically. `distanceMm` âm khi đang lùi, xung vẫn dương vì encoder một kênh không biết chiều. Mẫu còn tính xung pivot/trượt bánh, chỉ dùng để nội suy tương đối; RFID mới xác nhận node. Server phải áp dụng chiều dài cạnh cm thực, giới hạn hiển thị và đánh dấu telemetry cũ.

Event/ACK/error là FIFO RAM 16 phần tử; telemetry chỉ giữ snapshot mới nhất. QoS0, FIFO không bền qua reset, kết nối TCP/Wi-Fi có độ trễ phát hiện, MQTT hiện plaintext. Nút dừng vật lý và người giám sát khu lùi vẫn cần cho mô hình demo; còi không phát hiện vật cản sau xe.

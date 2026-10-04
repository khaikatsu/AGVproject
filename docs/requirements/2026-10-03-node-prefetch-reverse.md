# Lệnh hướng nhận trước node và lùi khỏi nhánh kho

Ngày nhận: 2026-10-03. Nguồn: người dùng xác nhận trong chat. Các đoạn bên dưới giữ nguyên bối cảnh intake; trạng thái code mới và giới hạn QA ở [giao thức v1.1](../04-mqtt-contract-v11.md). Liên quan [bản đồ 26 node](2026-10-03-map-position-line.md) và [ADR-0002](../decisions/0002-prefetched-node-orders-and-warehouse-reverse.md).

## Quyết định đã chốt

1. Server quyết định hướng rời từng node và gửi lệnh cho **node kế tiếp trước khi xe tới**. Khi RFID xác nhận node đúng dự kiến, firmware báo `AGV_REACHED_NODE`, kiểm tra lệnh đã lưu cho chính node đó rồi đi tiếp mà không phải chờ một vòng MQTT mới. Xe chỉ dừng *vì thiếu lệnh hướng* khi lệnh chưa có hoặc không hợp lệ.
2. Lệnh tại node có thể chỉ thị rẽ trái, rẽ phải hoặc đi thẳng. Lệnh phải gắn với đúng robot/nhiệm vụ, node áp dụng, node sẽ tới và thứ tự/phiên để lệnh cũ, trùng hoặc đến muộn không được dùng ở node khác. Tên trường và schema MQTT cụ thể còn TBD.
3. Các dừng khác vẫn có hiệu lực: pickup/drop, obstacle, siêu âm lỗi, `000`, RFID sai thứ tự, STOP/ESTOP, lỗi mạng hoặc cấu hình. Không có lệnh hợp lệ thì PWM motor bằng 0 tại vị trí chờ phù hợp; trạng thái chờ, deadline và cách server cấp lại lệnh còn TBD. Không suy ra servo nâng xong chỉ từ lệnh server.
4. Khi xe vào nhánh kho theo `G1 -> K1`, sau thao tác lấy hàng tại `K1`, xe **lùi trên chính đoạn vừa đi** cho đến khi RFID xác nhận lại `G1`. Chỉ sau mốc RFID `G1` mới xử lý chỉ thị tiếp theo; encoder một mình không được xác nhận đã về G1. Đây là thao tác lùi có chủ đích, không mặc nhiên cho phép đi ngược mọi cạnh một chiều trong map.
5. Tại mốc đổi từ lùi sang tiến/rẽ, bộ điều khiển phải đưa motor về mức dừng trước khi đảo chiều. Nếu lệnh rời G1 đã được cấp trước và an toàn, xe không cần chờ mạng; nếu chưa có, xe đứng chờ. Việc giảm tốc, thời gian dừng cơ khí và vị trí RFID trước ngã rẽ cần đo trên xe.
6. Người dùng muốn xe **phát âm thanh cảnh báo khi lùi**. Cảnh báo phải gắn với trạng thái motor thật sự đang lùi; không kêu khi boot/đứng yên/chạy tiến và tắt khi dừng, lỗi hoặc RFID G1 kết thúc đoạn lùi. Kiểu còi, GPIO, nguồn, nhịp/âm lượng và giới hạn môi trường còn TBD; chưa có buzzer trong cấu hình firmware.
7. Phạm vi được người dùng xác nhận là **mô hình trình diễn hệ thống**; theo tiêu chí người hướng dẫn, chức năng âm báo khi lùi là đủ cho phần cảnh báo của mô hình. Người dùng chọn **không lắp cảm biến vật cản phía sau** ở phiên bản này. Đây là tiêu chí demo, **không phải bằng chứng xe phát hiện hoặc tránh được người phía sau** hay đạt an toàn vận hành kho thực tế. Khi thử/demo cần người giám sát, giữ người xem ngoài đường lùi và có khả năng ngắt nguồn.

## Chưa chốt, cần giải quyết trước khi cho lùi tự động

- Xe hiện chỉ có HC-SR04 phía trước theo thiết kế đã ghi; không thêm cảm biến sau trong phạm vi mô hình demo đã chốt. **Âm báo chỉ cảnh báo người, không phát hiện hay tự tránh vật cản**; siêu âm trước cũng không quan sát đường lùi. Cần xác nhận bố trí khu demo, người giám sát/cách ngắt nguồn và thử quãng dừng; ứng dụng ngoài phạm vi mô hình phải đánh giá an toàn lại.
- Người dùng xác nhận ba mắt line nằm **phía trước AGV**. Khi lùi, bộ mắt nằm phía sau theo hướng chuyển động; cần thử chiều sửa lái và độ ổn định bám line riêng, hành vi `000`, không suy ra có thể dùng nguyên thuật toán tiến.
- Encoder một kênh chỉ đếm xung, không biết dấu chiều. Telemetry quãng đường trên đoạn lùi cần ghi rõ `K1 -> G1` hoặc hướng chuyển động; không dùng số xung dương như vị trí tiến từ G1.
- Cần chốt vị trí RFID để xe đọc đủ sớm trước điểm rẽ/dừng, khoảng dừng và deadline lệnh đã nhận; MQTT QoS0 và kết nối lại không bảo đảm lệnh đã đến chỉ nhờ publish thành công.
- Quy tắc này giải quyết trường hợp rời `K1` qua `G1`; đường vào/ra các kho khác và quyền dùng đường chung của hai xe cần xác nhận theo map thật.

## Hiện trạng code so với yêu cầu

Firmware v1 nhận `TASK` chứa toàn bộ path và `departure` từng node, chuyển đoạn ngay khi RFID đúng, chưa có hàng đợi lệnh rời node riêng hoặc trạng thái chờ thiếu lệnh tại node. `MotorAction::Backward` chỉ là primitive điều khiển bánh; FSM chưa có đoạn lùi `K1 -> G1`. Firmware vẫn nhận `O1..O10`, đo cm và chỉ điều khiển một servo nâng; tài liệu này không tự thay đổi những phần đó.

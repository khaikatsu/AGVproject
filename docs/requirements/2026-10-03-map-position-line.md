# Bản đồ 26 node, vị trí giữa node và quy tắc line

Ngày nhận: 2026-10-03. Nguồn: mô tả và sơ đồ người dùng gửi trong chat; người dùng xác nhận các số `20`, `35`, `50`, `70`, `100` trên cạnh là **cm đo ngoài đời**. Đính chính cùng ngày: tổng cộng **26 thẻ/node**; hai ô màu xanh dương/tím là node base. Firmware nay có 26 tên node (B1/B2 tạm) và telemetry mm, nhưng chưa có UID/map/backend và chưa thử phần cứng; xem [giao thức v1.1](../04-mqtt-contract-v11.md).

## Bản đồ vật lý

- Bản đồ có **26 node gắn thẻ RFID**: 6 kho `K1..K6`; 18 node đường/ngã rẽ/cua gồm `C1..C4`, `M1..M6`, `G1..G6`, `GS1..GS2`; và **2 node base** là hai ô màu xanh dương (phía trên `GS1`) và tím (phía dưới `GS2`) trong ảnh. `GS1/GS2` là node đường trước base, **không phải chính node base**. Tên mã/UID RFID của hai ô màu chưa được cung cấp, giữ TBD; không lấy chữ `01 ảo`/`02 ảo` trên giao diện làm mã RFID khi chưa xác nhận.
- Cạnh có chiều theo mũi tên trong sơ đồ. Server phải lưu topology, chiều đi và chiều dài vật lý từng cạnh; không suy ra chiều ngược được phép chỉ vì hai node nối nhau.
- Các nhãn chiều dài trong sơ đồ có đơn vị cm. Tọa độ hiển thị, kích thước lắp đặt thực tế từng cạnh, vị trí thẻ RFID và UID của 26 node cần đưa vào dữ liệu map trước khi chạy.
- Firmware production hiện chỉ biết `O1..O10`; server/backend, bản đồ 26 node và firmware chưa được đồng bộ. Hai ô màu được người dùng xác nhận là node base, nhưng UID và tên node trong giao thức vẫn chưa chốt.

## Vị trí giữa hai lần đọc RFID

- RFID xác nhận node vừa đi qua `fromNode`; path server giao xác định node kế tiếp `toNode`. ESP32 gửi quãng đã đi kể từ `fromNode` theo **mm**, kèm cặp node/đoạn và thời điểm hoặc số thứ tự đủ để server không ghép nhầm mẫu cũ sang đoạn mới.
- Server lấy chiều dài vật lý cạnh `L_cm`, đổi thành `L_mm = 10 * L_cm`, rồi hiển thị tiến độ ước lượng `distanceFromNodeMm / L_mm` trên cạnh có hướng `fromNode -> toNode`. Giới hạn tiến độ hiển thị trong `[0, 1]` và đánh dấu dữ liệu cũ/mất kết nối; không coi tiến độ encoder là RFID xác nhận đã tới `toNode`.
- Khi RFID xác nhận đúng `toNode`, phát event quãng đoạn hoàn tất, reset cả hai counter encoder nguyên tử và chuyển mốc sang đoạn tiếp theo. RFID sai thứ tự vẫn phải dừng an toàn.
- Encoder một kênh hiện chỉ đếm xung, không biết chiều quay; xung khi pivot/ngã rẽ và trượt bánh có thể làm sai quãng tiến dọc cạnh. Cần quy tắc tách xung quay khỏi quãng tiến, hiệu chuẩn PPR/đường kính bánh và phép đo sai số trên cạnh thật. Schema MQTT, nhịp cập nhật và chính sách vượt chiều dài cạnh chưa chốt.

## Quy tắc line mới

- Người dùng yêu cầu **chỉ pattern `000` mới làm xe dừng vì mất line**. Điều này không bỏ qua dừng do vật cản, siêu âm lỗi, dừng khẩn, mạng, RFID sai thứ tự hay lỗi cấu hình.
- Người dùng đã chốt `101` **chạy thẳng**, như `010/111`, và áp dụng quy tắc line mới vào firmware chính. `100/110` và `001/011` vẫn theo luật sửa lái hiện tại của firmware chính; chưa nhập thuật toán chỉ chạy một bên bánh của project test.
- Trong firmware chính, `000` gây LINE_LOST/SAFE_STOP kể cả lúc đang quay theo directive. `101/111` không được dừng chỉ vì line; RESTART cũng phải chấp nhận hai pattern này nếu các interlock khác tốt.
- Project test từng được xác nhận `010/111` đi thẳng, `100/110` chỉ chạy bánh trái, `001/011` chỉ chạy bánh phải và `000/101` dừng. Đây là lịch sử của bản test cũ; chưa sửa project test theo quyết định mới. Các điều kiện obstacle, mạng, RFID và dừng khẩn vẫn có quyền dừng firmware chính.

## Tiêu chí kiểm tra trước nghiệm thu

1. Đếm đủ 26 thẻ/node (2 ô base màu, 6 kho, 18 node đường gồm cả GS1/GS2); xác nhận tên/UID hai base, cạnh nối base xanh dương–GS1 và base tím–GS2, chiều dài/chiều đi thực tế của từng cạnh, UID hợp lệ không trùng; đối chiếu đường đi server với mũi tên trên sơ đồ.
2. Trên cạnh chuẩn, so quãng encoder mm với thước đo ngoài đời; kiểm tra khi vừa qua node, quay tại ngã rẽ, mất RFID, mất MQTT và mẫu telemetry đến muộn. Sai số/nhịp cập nhật chốt sau đo.
3. Thử đủ tám pattern line trên xe đã kê bánh và dưới tải; `000` dừng do mất line, `010/101/111` tiến, bốn pattern lệch sửa lái. Ghi PWM trái/phải, thử `000` khi đang quay và xác nhận các interlock độc lập vẫn dừng.

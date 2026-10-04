# IR trên cụm nâng, lệnh nâng/hạ từ server và giám sát mất hàng

Ngày nhận: 2026-10-03. Nguồn: người dùng mô tả trong chat, bổ sung [AGV-PROFILE-001](2026-09-30-line-servo-sonic-profile.md). Các đoạn dưới giữ bối cảnh intake; trạng thái code mới và giới hạn QA ở [giao thức v1.1](../04-mqtt-contract-v11.md). Logic đã code không tự trở thành kết quả đo phần cứng.

## Hành vi người dùng yêu cầu

1. Một cảm biến IR trên cụm nâng **nhìn hướng lên trên** và nhận bề mặt **màu đen**. Theo mô tả của người dùng, có hàng trong vùng nhìn thì ngõ ra `HIGH` và chỉ **một đèn** trên module sáng; không thấy hàng thì `LOW`. Cần đo lại điện áp ngõ ra, cực tính và trạng thái đèn trên đúng module IR hàng, không dùng bằng chứng của ba cảm biến line thay thế.
2. Khi RFID xác nhận xe đã tới node kho lấy hàng và motor dừng, firmware đọc IR nhìn lên. `HIGH` ổn định báo server rằng có hàng ở vùng nhìn tại kho, kèm robot/task/node/giai đoạn; `LOW` báo kho không có hàng trong vùng nhìn và **không nâng**. **Server quyết định gửi lệnh nâng**; firmware chỉ nâng khi lệnh đúng phiên, đúng nhiệm vụ/node, IR vẫn hợp lệ và các interlock local tốt. Không nâng chỉ vì `HIGH` hoặc chỉ vì tới RFID.
3. Sau khi đã lấy hàng và chuyển sang giai đoạn **đang chở hàng**, cùng IR nhìn lên được dùng để theo dõi hàng trên khay nâng. Khi tín hiệu có hàng chuyển `LOW` ổn định sau lọc nhiễu, firmware **dừng motor thật, chốt `SAFE_STOP` và gửi lỗi mất hàng về server**. Một mẫu `LOW` tức thời trong lúc cơ cấu chuyển động/chạy qua rung động không tự kết luận hàng rơi. Xe không tự chạy tiếp chỉ vì IR trở lại `HIGH`; cần quy trình phục hồi riêng.
4. Tới đúng node giao, xe dừng và **chờ lệnh hạ từ server**, giống cơ chế server cho phép nâng. Firmware chỉ hạ nếu lệnh đúng phiên/task/node/giai đoạn, servo sẵn sàng và interlock local tốt. Chưa coi hạ hàng hoàn tất chỉ vì đã gửi lệnh hoặc hết timer nếu chưa có bằng chứng cơ cấu/hàng thực tế.
5. `HIGH` chỉ có nghĩa có vật đen trong **vùng quan sát hiện tại**. Server phân biệt “hàng ở kho” trước pickup với “hàng trên khay” khi đang chở bằng giai đoạn/task/node, không bằng mức điện đơn lẻ. Hướng nhìn lên đã được người dùng chốt; cần thử trên mô hình để chứng minh một IR nhìn được cả hai mục tiêu ở hai giai đoạn.
6. Người dùng xác nhận ba thời điểm: **trước nâng ở kho**, khay trống nhưng IR nhìn lên thấy hàng trên kệ nên `HIGH`; **sau khi hàng đã lên xe/khay**, IR vẫn `HIGH` vì thấy hàng trên khay; **khi khay không có hàng**, IR `LOW`. Sau khi hạ và hàng rời khay, `LOW` là tín hiệu mong đợi trong giai đoạn kiểm tra khay, nhưng phép đo thực với vị trí hàng/kệ phía trên vẫn cần thử. `HIGH` liên tục ngay tại kho không tự chứng minh hàng đã chuyển từ kệ lên xe; cần kiểm tra tín hiệu khi kệ ra khỏi vùng nhìn.

## Chưa chốt trước khi sửa firmware

- Model IR, GPIO, nguồn và mức ngõ ra so với 3,3 V ESP32; màu/vật liệu/ánh sáng nền, khoảng cách và ngưỡng chống rung.
- Hướng nhìn đã chốt là lên trên và mức trước nâng là `HIGH` nếu kệ có hàng; cần đo khoảng cách/vùng nhìn thực trước nâng, khi kệ ra khỏi vùng nhìn, sau khi hàng lên khay và sau hạ, kể cả khoảng chuyển động servo có thể làm `LOW` tạm thời.
- Tín hiệu `LOW` khi kệ trống, khi khay trống và khi rút dây/IR lỗi; cách phân biệt mất hàng với false negative và thời gian xác nhận lỗi.
- Schema lệnh server cho nâng/hạ, ACK và timeout khi server không trả lời. Lệnh nâng/hạ không được vượt qua STOP/ESTOP, vật cản, sai RFID hoặc servo chưa sẵn sàng.
- Hai servo nâng cùng khay vẫn theo REQ-017; góc/xung/nguồn và cách xác nhận cơ cấu/hàng đã nâng hoặc hạ xong chưa chốt.

## So với code hiện tại

Firmware hiện chuyển vào `AGV_LOADING`/`AGV_UNLOADING` khi tới node và **tự phát `LiftUp`/`LiftDown` theo timer**, không có IR hàng, không gửi event xác nhận hàng ở kho, không chờ lệnh nâng/hạ từ server và không giám sát hàng rơi khi MOVING. `LIFT_UP`/`LIFT_DOWN` MQTT v1 chỉ là lệnh thủ công khi không có task active, nên không dùng lại nguyên xi trong nhiệm vụ. Không đổi code hoặc gán GPIO theo tài liệu này.

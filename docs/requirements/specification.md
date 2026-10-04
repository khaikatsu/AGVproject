# Sổ yêu cầu ban đầu

Phiên bản 0.15, ngày 2026-10-03. Nguồn: phiếu đề tài, [đặc tả firmware bổ sung của người dùng](2026-09-28-agv-firmware-request.txt), [hồ sơ AGV-PROFILE-001](2026-09-30-line-servo-sonic-profile.md), [bản đồ/vị trí/line](2026-10-03-map-position-line.md), [lệnh trước node/lùi khỏi kho](2026-10-03-node-prefetch-reverse.md) và [IR hàng/lệnh nâng-hạ server](2026-10-03-cargo-ir-server-lift.md). IMPLEMENTED nghĩa đã có logic firmware, không phải nghiệm thu hardware hoặc backend. Giao thức hiện hành ở [MQTT v1.1](../04-mqtt-contract-v11.md). ID và trạng thái phải khớp [traceability](../../qa/traceability.json).

| ID | Trạng thái | Nội dung | Tiêu chí còn cần chốt |
| --- | --- | --- | --- |
| REQ-001 | SOURCED | Hai robot tự hành chạy lộ trình lập trình trước | Lộ trình, tải, tốc độ, số lượt, quy tắc dùng chung đường |
| REQ-002 | SOURCED | Web quản lý robot theo thời gian thực | Người dùng, quyền điều khiển, độ trễ, online/stale |
| REQ-003 | SOURCED | Quản lý bản đồ kho | Node/edge/trạm, tọa độ, cách sửa và lưu map |
| REQ-004 | SOURCED | Quản lý lộ trình di chuyển | Tạo/giao nhiệm vụ, điểm đầu/cuối, hoàn thành và hủy |
| REQ-005 | SOURCED | Quản lý lượng pin robot | Mạch đo, độ chính xác, phần trăm/điện áp, ngưỡng |
| REQ-006 | SOURCED | Báo lỗi robot | Danh mục lỗi, severity, timestamp, phục hồi |
| REQ-007 | SOURCED | Báo cáo NCKH theo quy định học viện | Template, cấu trúc, minh chứng và ngày nộp |
| REQ-008 | IMPLEMENTED | SAFE_STOP với obstacle, line `000` hoặc siêu âm invalid; áp dụng motorStop | Ngưỡng obstacle, latency và quãng dừng phải đo; RESTART có interlock |
| REQ-009 | IMPLEMENTED | Mất Wi-Fi/MQTT dừng ngay, timeout latch, reconnect không tự chạy; RESTART/START riêng | Timeout thật, TCP detection latency, packet delivery và bảo mật |
| REQ-010 | IMPLEMENTED | UID -> 26 tên node (B1/B2 tạm); RFID xác nhận node, snapshot/reset hai encoder, AGV_REACHED_NODE | UID, tên base chính thức, PPR/diameter và sai số thực tế |
| REQ-011 | IMPLEMENTED | Hai servo nâng GPIO17/21, góc riêng, không góc boot, motor dừng khi nâng/hạ | Pulse/góc/tải/nguồn, đồng bộ và giới hạn hành trình |
| REQ-012 | IMPLEMENTED | Đủ module .h/.cpp, main ngắn, GPIO chính thức tập trung, pin/libs cố định | Đối chiếu dây và PWM trên board thật |
| REQ-013 | IMPLEMENTED | ESP32 nhận path/NODE_ORDER từ server, không A-Star; pickup/drop cần lệnh servo; ACK/session | Map/heading/departure server, calibration junction và thử route |
| REQ-014 | IMPLEMENTED | Không bịa calibration/UID/network; Battery ADC36 riêng, unknown NaN/null; Config TODO | Điền B25, range pin, các TODO có bằng chứng trước vận hành |
| REQ-015 | ACCEPTED | Lưu AGV-PROFILE-001 cho test line/motor/servo/HC-SR04: motor boot-off, BOOT bật/tắt, mapping line đã yêu cầu và cấu hình bench hiện tại | Chạy TC-027 trên xe thật, chốt trim/timing rồi mới tích hợp firmware production |
| REQ-016 | IMPLEMENTED | IR nhìn lên GPIO2 active HIGH, lọc 150 ms; HIGH/LOW được diễn giải theo giai đoạn | Thử vùng nhìn khi rời kệ, điện áp/nhiễu; HIGH tại kệ chưa chứng minh chuyển hàng |
| REQ-017 | IMPLEMENTED | Hai servo nâng dùng GPIO17/21 và góc riêng, cùng nhận lệnh nâng/hạ; không dùng chung GPIO | Cơ cấu, tải/nguồn, góc/xung từng bên, đồng bộ và chống kẹt |
| REQ-018 | ACCEPTED | Bản đồ có 26 thẻ/node: 6 kho `K1..K6`, 18 node đường `C/M/G/GS`, 2 node base là ô xanh dương và tím; `GS1/GS2` không phải base; cạnh có hướng và dài cm | Tên/UID hai base, chiều/độ dài cạnh base–GS, tọa độ và UID/vị trí đủ 26 thẻ cần chốt trước khi chạy |
| REQ-019 | ACCEPTED | Firmware gửi node/nextNode/distanceMm; server còn phải hiển thị tiến độ trên cạnh; RFID vẫn xác nhận node | Loại xung pivot, hiệu chuẩn/sai số, stale/overshoot, frontend/backend |
| REQ-020 | IMPLEMENTED | Chỉ `000` dừng vì line; `010/101/111` tiến, bốn pattern lệch chỉ chạy một bên bánh theo profile | Đo đủ tám pattern, PWM/trim và quay dưới tải thật |
| REQ-021 | IMPLEMENTED | NODE_ORDER gắn session/task/pathIndex/node/nextNode được cache trước RFID; thiếu/sai thì motor dừng WAITING | Thử server gửi/ACK/retry, RFID và latency/quãng dừng thật |
| REQ-022 | ACCEPTED | Sau pickup tại nhánh kho như `G1 -> K1`, xe lùi theo đoạn cũ tới RFID `G1`; chỉ khi G1 xác nhận mới nhận/thực hiện hướng tiếp, dừng motor trước đảo chiều | Mô hình demo không lắp cảm biến sau; cần bố trí giám sát/ngắt nguồn, bám line chiều lùi, telemetry có hướng, UID và quãng dừng; không suy ra an toàn xe kho thực tế |
| REQ-023 | IMPLEMENTED | GPIO0 còi active LOW khi cả hai motor thực nhận duty lùi, tắt lúc dừng; không thay cảm biến sau | Mạch boot-safe/nguồn, âm lượng và thử trên xe |
| REQ-024 | IMPLEMENTED | Pickup báo AGV_PICKUP_IR; LIFT_UP/DOWN chỉ theo lệnh đúng session/task/node/index và interlock; LOW không nâng | Test server/ACK/retry, hai servo, xác nhận nâng/hạ cơ khí |
| REQ-025 | IMPLEMENTED | Khi chở mà IR LOW ổn định 150 ms, motorStop + SAFE_STOP/CARGO_LOST + ERROR; không tự chạy lại | Đo chống nhiễu, sensor lỗi/hàng rơi và quãng dừng thật |

## Mỗi yêu cầu sau khi cụ thể hóa

Ghi nguồn, phiên bản, Given/When/Then, đơn vị, giao diện/module, TC và bằng chứng. Phần định tính đã có request rõ được triển khai; ngưỡng đo chưa có vẫn TBD và chưa VERIFIED. Không nâng trạng thái yêu cầu hệ thống kho chỉ vì firmware một xe đã compile.

## Khoảng trống hệ thống

Chưa frontend/backend/map/A-Star server, điều phối đường chung, dữ liệu pin hiệu chuẩn hoặc lịch sử lỗi. Giao thức firmware v1 và TODO có ở [MQTT](../04-mqtt-contract.md) và [cấu hình](../06-configuration.md).

## Nhật ký thay đổi

| Ngày | Phiên bản | Thay đổi | Người chốt |
| --- | --- | --- | --- |
| 2026-09-28 | 0.1 | Tách bảy mục tiêu từ phiếu và bốn mục kỹ thuật đề xuất | Chưa chốt tiêu chí chi tiết |
| 2026-09-28 | 0.2 | Nhận đặc tả 26 mục, triển khai framework modular, thêm REQ-012..014 | Người dùng chốt định tính; calibration/ngưỡng physical TBD |
| 2026-09-30 | 0.3 | Thêm REQ-015 và AGV-PROFILE-001 để giữ cấu hình test đã chốt cho lần tích hợp production sau | Người dùng chốt hành vi; phần cứng sau thay đổi BOOT còn NOT_RUN |
| 2026-10-03 | 0.4 | Ghi bổ sung IR phát hiện hàng và hai servo nâng khay vào AGV-PROFILE-001; chưa code | Người dùng nêu chức năng; chi tiết điện/cơ khí và vị trí cảm biến TBD |
| 2026-10-03 | 0.5 | Chốt 24 node, cạnh đo cm, telemetry vị trí giữa node theo mm; `101` chạy thẳng, chỉ `000` dừng vì line trong firmware chính | Người dùng xác nhận trong chat; đã sửa logic line, chưa tích hợp bản đồ/mm |
| 2026-10-03 | 0.6 | Chốt cấp lệnh trước node để không dừng chờ nếu hợp lệ; lùi từ K1 về RFID G1 rồi mới đi tiếp | Người dùng xác nhận trong chat; chưa code, an toàn chiều lùi TBD |
| 2026-10-03 | 0.7 | Bổ sung âm báo khi lùi; giữ riêng yêu cầu phát hiện/bảo vệ vật cản phía sau | Người dùng đề xuất trong chat; chưa có còi/GPIO/code |
| 2026-10-03 | 0.8 | Ghi lựa chọn không lắp cảm biến sau phiên bản này, dùng còi và kiểm soát khu vực lùi; không coi còi là phát hiện người | Người dùng xác nhận trong chat; phạm vi vận hành/QA chưa chốt |
| 2026-10-03 | 0.9 | Chốt phạm vi là mô hình trình diễn, âm báo đáp ứng phần cảnh báo theo tiêu chí người hướng dẫn; vẫn cần QA khu demo | Người dùng xác nhận trong chat; chưa code hoặc thử phần cứng |
| 2026-10-03 | 0.10 | Đính chính tổng số từ 24 thành 26 thẻ/node: 2 base, 6 kho, 18 node đường | Người dùng xác nhận trong chat; 2 nhãn đường chưa có trên ảnh giữ TBD |
| 2026-10-03 | 0.11 | Đính chính phân loại: hai ô màu là base, GS1/GS2 là node đường; ảnh đã hiện đủ 26 vị trí | Người dùng chỉ rõ trên ảnh; tên/UID hai base vẫn TBD |
| 2026-10-03 | 0.12 | Chốt IR hàng đen HIGH trên cụm nâng, server cho phép nâng tại kho và báo lỗi nếu mất hàng khi chạy | Người dùng mô tả trong chat; chưa code, hướng nhìn và phản ứng motor khi lỗi TBD |
| 2026-10-03 | 0.13 | Chốt IR nhìn lên, LOW là không thấy hàng; mất hàng khi chạy phải SAFE_STOP và báo server; hạ hàng cũng cần lệnh server | Người dùng xác nhận trong chat; chưa code, vùng nhìn/độ ổn định cần thử |
| 2026-10-03 | 0.14 | Chốt IR HIGH ngay trước nâng nếu thấy hàng trên kệ, HIGH sau nâng nếu hàng trên khay | Người dùng xác nhận trong chat; cần thử lúc rời kệ để phân biệt hai mục tiêu |
| 2026-10-03 | 0.15 | Chọn GPIO0 còi/GPIO2 IR, triển khai logic NODE_ORDER, lùi có chặn QA, IR/lệnh hai servo, báo mất hàng và telemetry mm | Người dùng yêu cầu code; compile/native test không phải nghiệm thu xe |

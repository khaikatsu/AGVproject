# Roadmap và backlog ban đầu

Các đợt là đề xuất thứ tự phụ thuộc; ngày và người phụ trách chưa chốt. Firmware cũ đã archive, framework modular mới đã implement/build/native-test theo request. Hardware/backend/web vẫn cần các đợt riêng.

| Đợt | Kết quả cần đạt | Điều kiện kết thúc |
| --- | --- | --- |
| 0 Workspace | Tài liệu, mẫu yêu cầu, skills, Validate/Build, baseline | Workspace kiểm tra được; không nhầm kế hoạch với kết quả thử |
| 1 Bàn thử một robot | Xác nhận BOM/pin/nguồn, line, encoder, RFID, PWM/servo | Có log đo và ca thử thành phần; chốt vấn đề ưu tiên |
| 2 Firmware điều khiển | Chính sách dừng, dữ liệu invalid, offline và phục hồi | Yêu cầu đã chốt có test và bằng chứng tương ứng |
| 3 Giao thức và backend | ID hai robot, telemetry/ACK, broker, dữ liệu trạm/bản đồ | Tích hợp broker/backend theo contract đã duyệt |
| 4 Web quản lý | Bản đồ, lộ trình, pin, lỗi, online/stale | Ca thử UI và tích hợp trên dữ liệu thật hoặc replay được gắn nhãn |
| 5 Hai robot và báo cáo | Điều phối dùng chung đường, kịch bản demo, báo cáo | Đạt tiêu chí nghiệm thu hai robot và có bằng chứng NCKH |

## Backlog cần chốt khi nhận mô tả

- Chọn board, driver, cảm biến, sơ đồ nguồn và mạch đo pin.
- Điền calibration/UID/mạng theo Config.h; đo latency dừng, tải/tốc độ và ngân sách loop cho SAFE_STOP đã triển khai.
- Thử TASK/START/STOP/RESTART và nâng hạ trên board/broker thật; không còn giao thức RUN cũ.
- RFID xác nhận node, encoder chỉ đo travel từng đoạn; server chốt map/heading/departure và A-Star.
- Phân biệt ID/topic hai robot; chốt chống tranh chấp đường/trạm.
- Chọn backend/web/database theo yêu cầu truy cập, triển khai và lưu dữ liệu.
- Chốt tiêu chí pin, thông tin lỗi, thời gian cập nhật và quyền điều khiển.
- Thu thập template báo cáo, tiến độ và kịch bản nghiệm thu của học viện.

## Definition of Done

Một tính năng xong khi yêu cầu và tiêu chí rõ, triển khai đúng, kiểm tra phù hợp đạt, tài liệu/traceability cập nhật, lỗi còn mở được ghi nhận. Chỉ ghi `VERIFIED` khi có bằng chứng. Một build thành công không đủ kết luận chức năng robot đã đạt.

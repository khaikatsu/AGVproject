# Sổ tay workspace AGV IoT

Workspace phục vụ đề tài xây dựng hệ thống quản lý robot tự hành trong kho dựa trên IoT. Mục tiêu trong phiếu đề tài là hai robot chạy lộ trình lập trình trước, web quản lý thời gian thực, bản đồ, lộ trình, pin, lỗi và báo cáo nghiên cứu. Bộ tài liệu giúp nhóm xác định hiện trạng và bổ sung yêu cầu có cách kiểm tra rõ.

## Hiện trạng dự án

Firmware ESP32 trong NCKHAGV đã được viết lại modular theo đặc tả ngày 2026-09-28: GPIO chính thức, hai encoder, ba chân servo, RFID node, ADC pin, path từ server và SAFE_STOP. Bản cũ đã archive. Chưa có backend, web, A-Star server hoặc điều phối robot thứ hai.

Firmware mới build esp32dev và có Unity native trên FSM/parser production. Kết quả ở qa/reports/2026-09-28-modular-firmware.md. Calibration và mạng còn TODO; chưa upload hoặc thử xe. Build/native PASS không nghiệm thu phần cứng.

## Tài liệu cần đọc

| Nội dung | File trong workspace |
| --- | --- |
| Hiện trạng và mục tiêu | docs/01-project-context.md |
| Kiến trúc và trạng thái | docs/02-architecture.md |
| GPIO nguồn linh kiện | docs/03-hardware.md |
| Topic và payload MQTT | docs/04-mqtt-contract.md |
| Build và phát triển | docs/05-development.md |
| Rủi ro và kế hoạch | docs/08-risks.md và docs/09-roadmap.md |
| Yêu cầu và nghiệm thu | docs/requirements/specification.md |
| Kế hoạch và ca thử | qa/test-plan.md và qa/test-cases.md |
| Nguồn tham khảo | docs/10-references.md |

SOURCED nghĩa là mục tiêu có trong phiếu đề tài; DRAFT là đề xuất. Chỉ chuyển ACCEPTED khi nội dung và tiêu chí đã chốt. VERIFIED cần kết quả thử và bằng chứng.

<!-- pagebreak -->

## Làm việc với firmware

Mở NCKH_2025.code-workspace trong VS Code. Lệnh kiểm tra và build từ thư mục gốc:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action Validate
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action Build
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action Test
```

Validate kiểm tra tài liệu, skills, REQ/TC và firmware guards. Test chạy Unity native, không upload. Build chỉ biên dịch; chạy phần cứng là bước riêng. docs/firmware-delivery.md có code từng file; docs/06-configuration.md chỉ rõ tham số cần điền.

## Kỹ năng cục bộ

| Skill | Dùng khi |
| --- | --- |
| esp32-agv-firmware | Sửa firmware cảm biến motor servo ISR MQTT |
| agv-qa | Review lập ca thử và ghi bằng chứng |
| agv-requirements | Chuyển mô tả mới thành yêu cầu nghiệm thu |

Skill nằm trong skills và được AGENTS.md định tuyến. Khi yêu cầu trợ lý làm việc, có thể nêu đường dẫn SKILL.md tương ứng. Các skill chưa được cài toàn cục.

## Những điểm ưu tiên kiểm tra

Firmware mới chặn motion khi cấu hình thiếu; sensor invalid không xem là đường trống. Mất mạng dừng motor, timeout latch SAFE_STOP; reconnect không tự chạy. RFID xác nhận node, encoder đo đoạn; hai counter reset atomic. Cần đo latency, PWM, RC522 và encoder đồng thời trên board thật.

Danh mục có 26 ca thử, phân biệt native và hardware. Ghi PASS, FAIL, BLOCKED hoặc NOT_RUN cùng robot ID, firmware, dụng cụ và số đo. Không đánh dấu hardware PASS chỉ từ native.

<!-- pagebreak -->

## Bổ sung yêu cầu tiếp theo

Thêm mô tả tại docs/requirements hoặc gửi trong chat. Phiếu mô tả yêu cầu Word có sẵn để ghi mục tiêu, trigger, điều kiện chạy, thông số, ca lỗi và cách nghiệm thu.

Sau khi tiếp nhận, gán REQ ID, ghi nguồn và trạng thái trong specification.md, thêm TC tương ứng trong qa/test-cases.md và cập nhật qa/traceability.json. Các ngưỡng chưa có quyết định giữ TBD.

Thông tin cần điền gồm driver/chiều motor, cực tính line, PPR/đường kính bánh, UID thật, vai trò/góc/pulse servo, B25, mạng/topic và ngưỡng dừng. GPIO đã được chốt, không tự đổi. Server còn cần map/heading/departure và chính sách hai xe dùng chung đường.

## Thứ tự công việc đề xuất

Hoàn thiện bàn thử một robot và chốt chính sách điều khiển; thống nhất giao thức hai ID và backend; xây web theo yêu cầu bản đồ lộ trình pin lỗi; thử hai robot và tập hợp bằng chứng báo cáo. Lịch và người phụ trách sẽ được xác nhận theo tiến độ thực tế.

## Nguồn gốc và cách tham khảo

Phạm vi lấy từ phiếu đề tài gốc; hiện trạng lấy từ source firmware. Workspace đã tham khảo repo chính thức PlatformIO Espressif32, Arduino ESP32, Unity, PubSubClient, MFRC522, ESP32Servo và Open-RMF demos. Open-RMF chỉ là nguồn tham khảo tổ chức nhiệm vụ và tài nguyên dùng chung; chưa chọn làm nền tảng dự án.

Một số nguồn chính:

- [PlatformIO Espressif32](https://github.com/platformio/platform-espressif32)
- [Unity embedded testing](https://github.com/ThrowTheSwitch/Unity)
- [Open RMF demos](https://github.com/open-rmf/rmf_demos)

Danh mục đầy đủ và lưu ý phiên bản ở docs/10-references.md. Khi thay nội dung nguồn Markdown, xuất lại Word bằng ExportDocs và kiểm tra bố cục. Nếu điền trực tiếp phiếu Word, lưu thành file tên mới rồi cập nhật yêu cầu đã chốt trở lại Markdown.

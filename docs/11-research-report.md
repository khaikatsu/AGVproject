# Khung báo cáo nghiên cứu khoa học

Phiếu đề tài yêu cầu báo cáo theo quy định học viện. Chưa có mẫu trình bày, độ dài, biểu mẫu nghiệm thu hoặc yêu cầu trích dẫn chính thức. Khung này dùng để tập hợp nội dung kỹ thuật và kết quả trước khi áp dụng mẫu đó.

Báo cáo phần nhúng đã được viết thành nội dung năm chương tại [báo cáo ESP32](12-bao-cao-phan-nhung.md), với bản xuất [Word](word/bao-cao-phan-nhung.docx).

## Nội dung cần tập hợp

1. Mục tiêu và phạm vi: vấn đề quản lý robot trong kho, hai robot, lộ trình và web.
2. Cơ sở kỹ thuật: ESP32, cảm biến, motor/servo, RFID, encoder, IoT/MQTT; trích nguồn thực sự đã dùng.
3. Phân tích yêu cầu: chức năng, giới hạn môi trường, tiêu chí nghiệm thu và giả định.
4. Thiết kế: sơ đồ khối, sơ đồ nguồn/pin, thuật toán trạng thái, dữ liệu bản đồ và giao thức.
5. Triển khai: hardware thực tế, firmware, backend/web và phiên bản phụ thuộc.
6. Thử nghiệm: cấu hình, phương pháp, số lần đo, dữ liệu thô, sai số và ca lỗi.
7. Kết quả: đối chiếu mục tiêu với bằng chứng; giới hạn, lỗi còn tồn và công việc tiếp theo.
8. Tài liệu tham khảo và phụ lục: BOM, sơ đồ, test report, ảnh demo và cách tái hiện.

## Kỷ luật bằng chứng

Không ghi số liệu độ trễ, tải, quãng đường, dung lượng pin hoặc độ chính xác nếu chưa đo. Replay/simulation phải được ghi nhãn. Khi trình bày biểu đồ, giữ dữ liệu thô, đơn vị, điều kiện và số lần thử; phân biệt giá trị trung bình với worst case.

Nguồn tham khảo trong [references](10-references.md) là điểm bắt đầu, không tự động trở thành tài liệu đã dùng trong mọi chương. Báo cáo test theo [mẫu test run](../qa/templates/test-run.md).

# Tiếp nhận mô tả yêu cầu

Bạn có thể thêm mô tả dạng Markdown/Word hoặc gửi trực tiếp trong chat. Dùng [request template](request-template.md) khi muốn ghi có cấu trúc. Lưu mô tả mới cạnh các file này bằng tên `YYYY-MM-DD-ten-yeu-cau.md`; với tài liệu ngoài, ghi tên nguồn và ngày nhận.

## Quy trình

1. Giữ nguyên nguồn ban đầu; tách mong muốn, thông tin đo được, giả định và câu hỏi.
2. Gán REQ ID tiếp theo trong [sổ yêu cầu](specification.md), không tái sử dụng ID đã bỏ.
3. Nêu hành vi quan sát được, tiền điều kiện, dữ liệu/đơn vị, ca lỗi và cách nghiệm thu.
4. Ghi tác động đến firmware, pin, giao thức, backend/web và test.
5. Chuyển DRAFT/SOURCED sang ACCEPTED khi nội dung được chốt; lời yêu cầu triển khai rõ ràng của người dùng có thể là nguồn chốt.
6. Cập nhật [traceability JSON](../../qa/traceability.json), [ca thử](../../qa/test-cases.md) và chạy Validate.

`SOURCED` chỉ xác nhận có mục tiêu trong phiếu đề tài; không xác nhận một ngưỡng chưa được nêu. Không tự chọn tải, tốc độ, latency, stack web hoặc chân đo pin rồi ghi thành yêu cầu đã duyệt.

## Thông tin hữu ích khi bổ sung

Robot cần làm gì và khi nào; model phần cứng; bối cảnh chạy và tải; hành vi khi lỗi/mất mạng; dữ liệu web cần hiển thị; điều kiện nghiệm thu và người kiểm tra. Mô tả chưa đủ vẫn được ghi nhận DRAFT; hỏi các chi tiết đang chặn công việc cụ thể.

## Đồng bộ Word

[Mẫu yêu cầu Markdown](request-template.md) là nguồn của tài liệu Word. `ExportDocs` ghi lại bản Word trong `docs/word/`. Nếu điền trực tiếp trên Word, lưu thành file tên mới để tránh bị lần export sau ghi đè, rồi nhập nội dung đã chốt trở lại sổ yêu cầu Markdown.

# Tài liệu Word

Các bản Word được xuất từ Markdown để tiện đọc và điền yêu cầu:

- [Sổ tay workspace AGV IoT](workspace-guide.docx), nguồn [workspace guide](workspace-guide.md).
- [Phiếu mô tả yêu cầu](requirements-form.docx), nguồn [request template](../requirements/request-template.md).
- [Báo cáo phần nhúng robot tự hành](bao-cao-phan-nhung.docx), nguồn [báo cáo Markdown](../12-bao-cao-phan-nhung.md).
- [Báo cáo phần nhúng độc lập dành cho nhóm trưởng dạng DOC](phan-nhung-agv-cho-nhom-truong.doc), nguồn duy trì [bản năm chương cập nhật](../13-phan-nhung-cho-nhom-truong.md). File DOC chứa đầy đủ nội dung và không có hyperlink hoặc nội dung cần tải từ file khác.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action ExportDocs
```

Manifest [export-manifest.json](export-manifest.json) quy định nguồn và file đích. Export ghi đè các file đích trong manifest, không sửa phiếu đề tài gốc. Markdown là bản duy trì; nếu điền Word, lưu thành file mới rồi nhập yêu cầu đã chốt vào sổ yêu cầu để giữ đồng bộ.

Bản dành cho nhóm trưởng dùng profile `team-brief`: wrapper gọi [export_team_brief.py](../../tools/export_team_brief.py) bằng Python/Node và thư viện đi kèm runtime Codex trên máy này. Profile kiểm tra đúng năm chương và đặt khổ A4; khi chuyển máy cần runtime tương ứng hoặc điều chỉnh exporter có chủ đích.

Manifest sinh DOCX làm bản trung gian. Để xuất Word 97–2003 binary `.doc` thật, chạy lệnh sau trên máy có Microsoft Word; script kiểm tra năm chương, không có hyperlink và chữ ký OLE. Không đổi đuôi DOCX thành DOC bằng rename.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/convert_team_brief_to_doc.ps1
```

Thêm `-RenderPages` để tạo ảnh QA trực tiếp từ Page.EnhMetaFileBits của Word vào `.qa-artifacts/standalone-doc/pages-final/`, không đi qua bước PDF. Script dùng instance riêng, chỉ mở báo cáo đã tạo và không sửa tài liệu người dùng đang mở.

`ExportDocs` cần `python-docx` và `markdown-it-py`, chỉ tạo Word; không tự kiểm tra bố cục. Khi thay nội dung cần render và xem từng trang bằng công cụ tài liệu sẵn có. Kết quả lần tạo đầu ghi trong [baseline](../../qa/reports/2026-09-28-baseline.md).

Trên Windows có Microsoft Word, [render_word.ps1](../../tools/render_word.ps1) mở riêng các bản Word được tạo ở chế độ read-only, xuất PDF và ảnh vào `.qa-artifacts/word/` để kiểm tra bố cục. Script cần Word COM và Poppler, không chỉnh tài liệu đang mở của người dùng. Nếu không lấy được một instance Word trống, script dừng. Chạy `powershell -NoProfile -ExecutionPolicy Bypass -File tools/render_word.ps1`, sau đó xem từng trang; render thành công chưa tự xác nhận bố cục đạt.

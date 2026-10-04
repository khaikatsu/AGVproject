# ADR-0003: Server cho phép nâng/hạ, IR giám sát hàng

Ngày 2026-10-03. Trạng thái: **ACCEPTED về luồng, chưa triển khai**. Nguồn: [mô tả IR hàng](../requirements/2026-10-03-cargo-ir-server-lift.md). REQ-016/024/025.

## Tình huống

Firmware v1 tự nâng/hạ một servo khi RFID tới pickup/drop, dựa vào timer và không có tín hiệu hàng. Người dùng muốn IR nhìn lên ở cụm nâng phát hiện hàng đen tại kho, báo server, rồi server quyết định cho nâng và hạ; sau pickup, IR tiếp tục giám sát hàng đang chở.

## Các phương án

- Tự nâng ngay khi RFID pickup: có sẵn trong v1 nhưng không xác nhận hàng và không cho server quyết định.
- Xe báo IR + giai đoạn, server cấp lệnh nâng, xe kiểm tra lại interlock trước khi actuate: đáp ứng luồng người dùng nhưng cần schema/timeout và không được phụ thuộc MQTT để dừng an toàn.
- Chỉ server tin IR rồi điều khiển servo trực tiếp: thiếu kiểm tra local khi lệnh cũ/đến muộn hoặc cảm biến đã đổi trạng thái.

## Quyết định

Chọn báo IR theo node/giai đoạn và **server cho phép nâng/hạ**, song firmware vẫn là nơi chặn hành động nếu lệnh sai phiên/task/node/giai đoạn, xe chưa dừng, IR không còn hợp lệ ở pickup, servo chưa sẵn sàng hoặc điều kiện an toàn lỗi. Tại pickup, `HIGH` thấy hàng trên kệ, `LOW` báo kho không có hàng và không nâng. Sau khi hàng lên khay, IR cũng `HIGH`; vì vậy `HIGH` liên tục tại kệ không tự xác nhận chuyển hàng thành công, phải kiểm tra vùng nhìn khi rời kệ. Khi đã chở hàng, `LOW` ổn định sau lọc nhiễu phải **dừng motor, vào `SAFE_STOP` và báo lỗi server**. Không tự tiếp tục chỉ vì IR trở lại `HIGH`.

## Hệ quả và xác minh

Cần đổi FSM `AGV_LOADING/AGV_UNLOADING`, codec/MQTT, hai servo nâng, cảm biến IR, trạng thái cargo và telemetry/error. Một IR chỉ phân biệt kho/khay theo giai đoạn nếu vùng nhìn vật lý hướng lên đã được chứng minh. TC-028/036..038 kiểm tra; không gắn nhãn PASS trước khi test module thật và luồng server.

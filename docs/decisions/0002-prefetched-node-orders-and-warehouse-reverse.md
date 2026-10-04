# ADR-0002: Cấp lệnh trước node và lùi khỏi nhánh kho

Ngày 2026-10-03. Trạng thái: **ACCEPTED về nguyên tắc, chưa triển khai**. Nguồn: [mô tả người dùng](../requirements/2026-10-03-node-prefetch-reverse.md). REQ-021/022/023.

## Tình huống

Firmware v1 nhận path và hướng rời node trong TASK, không đợi lệnh mới ở từng RFID. Người dùng muốn server quyết định theo node nhưng xe không phải dừng-chạy do trễ MQTT. Sơ đồ kho có nhánh `G1 -> K1`; sau pickup xe cần lùi về RFID `G1` mới tiếp tục.

## Các phương án

- Gửi lệnh chỉ sau sự kiện tới node: đơn giản nhưng bắt xe dừng chờ tại mọi node, phụ thuộc vòng MQTT và có thể chặn ngã rẽ.
- Cấp lệnh hướng cho node kế tiếp khi xe còn đang trên đoạn trước: node là mốc xác nhận, xe dùng lệnh đã kiểm tra ngay; thiếu/sai lệnh thì dừng chờ.
- Cho ESP32 tự chọn hướng: giảm phụ thuộc server nhưng trái yêu cầu server quản lý map/đường chung và làm trùng thuật toán tuyến.

## Quyết định

Chọn cấp lệnh cho node kế tiếp **trước khi xe tới**. Lệnh phải ràng buộc robot, task, node áp dụng, node đích, thứ tự/phiên và được kiểm tra tại RFID; publish sự kiện tới node không được chặn loop để đợi phản hồi. Nếu lệnh thiếu/sai, dừng motor và chờ an toàn. Server vẫn chịu trách nhiệm route/đường chung; firmware không tính A-Star.

Đoạn ra khỏi kho `K1` được mô hình hóa như thao tác **lùi từ K1 về RFID G1**, không phải đảo chiều toàn bộ graph. Khi đọc lại G1, kết thúc đoạn lùi và chỉ sau đó dùng lệnh rời G1. Motor phải về mức dừng trước chuyển từ lùi sang tiến/rẽ, kể cả khi lệnh tiếp đã có.

Trong khi motor lùi, xe phát âm báo và tắt âm báo khi hết lùi. Phạm vi là mô hình trình diễn; người dùng cho biết người hướng dẫn chấp nhận chức năng cảnh báo âm thanh cho phần demo, không lắp cảm biến sau. Còi không thay thế phát hiện vật cản phía sau: xe sẽ không thể tự dừng vì một người mới bước vào đường lùi. Demo có người giám sát, người xem đứng ngoài đường lùi và có khả năng ngắt nguồn; không coi lựa chọn còi là nghiệm thu an toàn cho xe kho thực tế. [HSE về lùi xe](https://www.hse.gov.uk/workplacetransport/information/reversing.htm) khuyến nghị kiểm soát khu vực lùi và lưu ý một biện pháp đơn lẻ khó bảo đảm an toàn; [HSE về âm báo](https://www.hse.gov.uk/workplacetransport/vehicles/safemanoeuvring.htm) nêu nguy cơ còi bị lẫn vào tiếng ồn hoặc bị bỏ qua.

## Hệ quả và xác minh

Cần mở rộng FSM, codec/contract MQTT, telemetry đoạn có hướng, node mapping 26 vị trí, âm báo và kiểm thử với server thật. Trạng thái chờ/deadline, xác nhận lệnh, vị trí RFID, bố trí giám sát khu demo, bám line chiều lùi, quãng dừng, còi/GPIO và điều phối hai xe còn TBD. Hai ô màu base đã có trên ảnh cần chốt tên/UID và cạnh nối với GS1/GS2 trước khi chốt map. TC-033..035 là ca kiểm tra; native test không thay thử phần cứng hoặc biến mô hình thành xe kho được chứng nhận an toàn.

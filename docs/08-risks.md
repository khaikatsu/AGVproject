# Rủi ro sau triển khai firmware modular

Baseline cũ được giữ riêng trong archive. Các biện pháp dưới đây đã có trong source nhưng không đồng nghĩa đã kiểm nghiệm bàn thử.

| ID | Mức | Hiện trạng và biện pháp | Cần làm / ca thử |
| --- | --- | --- | --- |
| RISK-001 | P1 | Controller dừng driver cùng update, mạng chạy task riêng | Đo latency vật lý với RC522/Serial/ADC/scheduler; TC-004/005/009 |
| RISK-002 | P1 | HC-SR04 invalid/no-echo chặn motion, không dùng 999 cm | Chuyển mức ECHO, đo range/deadzone và tần suất false stop; TC-006 |
| RISK-003 | P1 | Servo không đặt góc boot, task interlock, timer tách motor | Chưa có limit switch; timer settling không chứng minh nâng xong. SAFE_STOP giữ target servo; TC-012/013 |
| RISK-004 | P1 | GPIO cố định theo yêu cầu, compile chống collision | Kiểm tra strapping GPIO5/12/15, module WROOM/PSRAM, nguồn/mass, ADC an toàn; TC-002 |
| RISK-005 | P1 | L298N/line HIGH lấy từ profile; PPR/diameter/threshold/servo vẫn chặn START | Cần calibration có số đo dưới tải và kiểm lại trim, không chỉ đổi confirmed=true; TC-014/023 |
| RISK-006 | P1 | SAFE_STOP latch; quay bị gián đoạn không tự resume | Định vị/heading tại node và departure phải do server/nhóm xác nhận; TC-022/024 |
| RISK-007 | P2 | UID mapping + event đoạn, queue RAM + reset atomic | Đọc tag ở tốc độ thật, encoder noise/slip, event QoS0/reset có thể mất; TC-007/008 |
| RISK-008 | P1 | Session/command ID/ACK, stale queue và ưu tiên stop | Không TLS/auth ứng dụng; nonce không chống tấn công, cửa sổ dedup hữu hạn; TC-010/011/026 |
| RISK-009 | P2 | Đã pin platform/libs, native test production | PubSub/MFRC upstream hạn chế maintenance, core2; CI chưa chạy cloud; TC-001/021 |
| RISK-010 | P2 | Battery ADC có calibration sentinel và null | Chưa low-battery policy, % tuyến tính không SOC thật; TC-018/023 |
| RISK-011 | P2 | Một firmware, ID/topic mỗi xe tự điền | Chưa backend/web/A-Star/điều phối tránh xung đột hai xe; TC-015..019 |
| RISK-012 | P1 | MotorStop PWM=0, không tự chạy boot/reconnect | Không phải safety-rated stop, reset MCU không ngắt driver tự động. Cần nút dừng cứng, bảo vệ nguồn và đo quãng dừng; TC-002/004 |
| RISK-013 | P1 | FSM lùi `K1 -> G1` có code nhưng khóa bởi `REVERSE_STEERING_CONFIRMED=false`; không lắp cảm biến sau, chỉ dùng còi | Còi không phát hiện người trong đường lùi. Demo có giám sát/ngắt nguồn; đo bám line khi lùi, RFID và quãng dừng; TC-034/035 |
| RISK-014 | P1 | IR nhìn lên lọc 150 ms, server cho lệnh nâng/hạ; LOW khi chở làm SAFE_STOP/CARGO_LOST | HIGH tại kệ không chứng minh hàng trên khay, đứt dây LOW không phân biệt rơi hàng; đo vùng nhìn/3,3 V/nhiễu và cơ khí; TC-028/036..038 |
| RISK-015 | P1 | GPIO0 còi active LOW và GPIO2 IR là hai chân strapping còn trống, chưa thử trên DevKit V1 | Mạch còi trở kháng cao/pull-up, IR không vượt 3,3 V, ngắt IR HIGH khi nạp; kiểm boot/nạp HIGH/LOW và nguồn còi; TC-002/028/035 |

P1 ưu tiên trước chạy tự do hoặc có tải, P2 xử lý theo đợt. Ngân sách control loop, braking distance, range pin và cơ khí chưa được cung cấp, giữ TBD; không biến build/native PASS thành nghiệm thu robot. Theo dõi quyết định tại [ADR](decisions/0001-modular-firmware.md), lỗi thực nghiệm tại [bug report](../qa/templates/bug-report.md).

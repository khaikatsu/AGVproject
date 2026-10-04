# ADR-0001: Firmware modular theo đặc tả người dùng

Ngày 2026-09-28. Trạng thái: implemented framework, hardware verification pending. Nguồn quyết định: [request đầy đủ](../requirements/2026-09-28-agv-firmware-request.txt).

## Bối cảnh

Request thay GPIO baseline, thêm hai encoder/ba servo/battery/path và SAFE_STOP. Chưa có UID, calibration, network contract cuối cùng hay hình học junction. Không được giữ các giá trị đo giả từ baseline.

## Quyết định

- Giữ PlatformIO Arduino, pin Espressif32/core2 và thư viện hiện có; thêm ArduinoJson 6.21.5 để parse JSON có giới hạn.
- Lưu firmware cũ ở archive trước khi đổi. Active code dùng Config.h và đủ module .h/.cpp theo request.
- Tách ControlLogic/CommandCodec production khỏi Arduino để Unity native kiểm thử FSM/parser thực tế.
- Task mạng riêng core0 giao tiếp controller bằng queue/atomic stop flag, không gọi motor. Vòng điều khiển không chờ reconnect.
- Dùng LEDC native cho servo, channel/timer tách motor; không attach hoặc phát góc chưa hiệu chuẩn.
- Sentinel unknown compile được nhưng chặn motion/servo khi cần; telemetry không hiệu chuẩn là null.
- TASK/START tách biệt, SAFE_STOP latch + RESTART không tự chạy; session và ID để giảm replay/dup có giới hạn.
- Server chọn path/heading/departure. ESP32 không tính A-Star, không đoán hướng từ tên node.

## Hệ quả và giới hạn

Schema không tương thích RUN/CONTINUE cũ, server cần cập nhật. Cần điền Config.h và thử hardware trước chạy. Native tests không bao phủ GPIO/scheduler/driver. QoS0+queue RAM không đảm bảo bền event; PWM stop không thay nút dừng cứng. Chi tiết tại [MQTT](../04-mqtt-contract.md), [cấu hình](../06-configuration.md) và [rủi ro](../08-risks.md).

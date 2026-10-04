# Hướng dẫn phát triển và kiểm tra

Mở [workspace VS Code](../NCKH_2025.code-workspace). Firmware nằm trong NCKHAGV. Config.h là nơi duy nhất điền tham số và giữ GPIO chính thức. Bản code và giải thích từng file được sinh tại [firmware-delivery](firmware-delivery.md).

## Thư mục firmware

`main.cpp` chỉ là điểm vào. Các cặp header/source được gom theo chức năng, không đặt chung với `main.cpp`. Xem [sơ đồ và danh sách module](../NCKHAGV/README.md).

| Thư mục | Nội dung |
| --- | --- |
| `src/core/` | Controller, FSM và giải mã lệnh |
| `src/motion/` | Motor, chuyển động và bám line |
| `src/sensors/` | Siêu âm, encoder, RFID, IR hàng và pin |
| `src/actuators/` | Servo và còi |
| `src/network/` | Wi-Fi và MQTT |

`src/Config.h` và `src/Types.h` là hai header dùng chung giữ ở vị trí cũ để không đổi nguồn cấu hình GPIO hoặc đường dẫn của các project test riêng. PlatformIO có `-Isrc` để module đọc header chung và include qua đường dẫn nhóm. Native test chỉ biên dịch `core/ControlLogic.cpp` và `core/CommandCodec.cpp`, không kéo các driver Arduino vào test.

## Lệnh từ thư mục gốc

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action Validate
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action Test
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action Build
powershell -NoProfile -ExecutionPolicy Bypass -File tools/workspace.ps1 -Action ExportFirmware
```

Validate kiểm tra docs/skills/traceability và các guard firmware. Test chạy Unity native trên ControlLogic/CommandCodec production, không upload ESP32. Windows cần g++/gcc trên PATH (máy này dùng MinGW CodeBlocks); Linux CI dùng GCC có sẵn. Build chỉ tạo ELF/BIN. Upload là bước riêng sau khi kiểm tra điện/cơ khí/cấu hình, không tự chạy trong tooling.

## Phiên bản đã khóa

platform espressif32 6.13.0, Arduino package 3.20017.241212+sha.dcc1105b (core 2.0.17), MFRC522 1.4.12, PubSubClient 2.8, ArduinoJson 6.21.5, native platform 1.2.1. Unity do PlatformIO test runner resolve, phiên đã chạy là 2.6.1. WiFi/SPI/LEDC thuộc Arduino core. Không dùng ESP32Servo trong bản mới: ServoModule quản lý LEDC để tách timer rõ ràng. Không cần thư viện encoder.

Core 3.x đổi LEDC API, không tự nâng platform/library khi chưa cập nhật code và đo PWM. Các lựa chọn được ghi tại [ADR](decisions/0001-modular-firmware.md). Thư viện JSON dùng parser thật, không tách chuỗi thủ công để giải mã command.

```powershell
Set-Location NCKHAGV
pio run -e esp32dev -t clean
pio run -e esp32dev
pio test -e native
pio pkg list -e esp32dev
pio device list
```

Khi đã chuẩn bị bàn thử, chọn COM thật cho `pio run -e esp32dev -t upload --upload-port COMx`; serial monitor `pio device monitor -b 115200`. Không tự suy ra đúng board từ tên esp32dev. Chưa upload hoặc actuate hardware trong đợt triển khai này.

## Tooling tài liệu

Wrapper tìm .venv/Python đi kèm máy/PATH và pio/PATH hoặc cache PlatformIO người dùng; có thể chỉ rõ PythonPath/PlatformIOPath. Máy mới cần Python 3.12 và [tools requirements](../tools/requirements.txt):

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r tools/requirements.txt
```

ExportDocs tạo lại Word từ Markdown, renderer dùng Word/Poppler trên Windows theo [hướng dẫn Word](word/README.md). ExportFirmware sinh bản source theo thứ tự người dùng; kiểm tra bản sao không bị cũ bằng `python tools/export_firmware.py --check`. Không sửa bản code trong tài liệu sinh.

## Quy trình thay đổi

Gắn REQ/TC, sửa đúng module, giữ unknown là TODO, chạy Validate/Test/Build và ExportFirmware, cập nhật QA evidence. Nếu sửa source Word thì ExportDocs và kiểm tra layout. Native test không bao phủ GPIO/ISR/scheduler trên silicon, driver thật hoặc packet delivery broker. CI đã có validate/native/build nhưng chưa chạy GitHub vì workspace chưa có git/remote.

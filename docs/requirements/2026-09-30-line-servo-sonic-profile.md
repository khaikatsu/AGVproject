# AGV-PROFILE-001: Line, motor, servo quét và HC-SR04

Ngày nhận: 2026-09-30. Nguồn: các yêu cầu và kết quả thử trực tiếp của người dùng trong phiên test phần cứng. Đây là hồ sơ để tiếp tục chỉnh project test và tích hợp có chủ đích vào firmware chính sau này; nó không tự thay đổi `NCKHAGV/`.

## Cách gọi lại

Dùng một trong hai câu:

- `Cập nhật AGV-PROFILE-001` để thay đổi hồ sơ và project test này.
- `Áp dụng AGV-PROFILE-001 vào firmware chính` để rà lại bằng chứng, sửa các module production liên quan và chạy toàn bộ kiểm tra bắt buộc.

Project tham chiếu: `experiments/line-servo-sonic-motor/`.

## Quyết định đã chốt cho profile test

- Chân lấy từ `NCKHAGV/src/Config.h`; driver test hiện chọn L298N.
- Ba cảm biến line đọc trực tiếp, không đảo. Line đen là mức `1`, nền trắng là mức `0`. Người dùng xác nhận cách nhận sensor hiện tại hoạt động đúng.
- Mỗi module sensor có một đèn nguồn và một đèn tín hiệu hoạt động theo mức ngược: chỉ sáng một đèn nguồn tương ứng GPIO `1`; sáng cả hai đèn tương ứng GPIO `0`. Đây là quan sát người dùng đã xác nhận trên bộ sensor hiện tại, không dùng quy tắc này cho module khác nếu chưa đo lại.
- Khi cấp nguồn, motor phải dừng; servo vẫn lập tức về 90°, sau đó quét 0–180–0 và HC-SR04 đo ở từng góc.
- Nút BOOT/GPIO0 là nút bật/tắt quyền chạy motor: lần nhấn thứ nhất bật, lần thứ hai tắt, các lần tiếp theo luân phiên. Không giữ nút trong lúc reset/cấp nguồn.
- Khi motor được bật: `010` và `111` chạy thẳng; `100` hoặc `110` chỉ chạy cụm bánh trái để cua phải về line; `001` hoặc `011` chỉ chạy cụm bánh phải để cua trái về line; `000` và `101` dừng. Mapping dựa trên hướng lắp thực tế: người dùng quan sát chạy bánh phải ở `100/110` làm xe cua trái và đi ra khỏi line.
- Chiều tiến hiện dùng `LEFT_MOTOR_INVERTED=true` và `RIGHT_MOTOR_INVERTED=true`; người dùng xác nhận xe đã hết chạy ngược.
- PWM hiện tại: base 210, correction 210, trim trái -20, trim phải 0. Người dùng quan sát cụm trái mạnh hơn; mức trim cuối cùng vẫn cần chạy xe thật để chốt.
- HC-SR04 dùng đo không chặn dựa trên interrupt; timeout không được làm treo cập nhật line/motor.
- Lệnh Serial `s` dừng motor, đưa servo về 90° rồi tắt xung; `g` khởi động lại quét servo và không tự bật motor.

## Tiêu chí nghiệm thu trước khi đưa vào firmware chính

1. Cold boot: đo PWM hai bên bằng 0; servo bắt đầu chu trình 90° rồi 0–180–0.
2. Một lần nhấn BOOT chỉ tạo một lần đổi trạng thái sau debounce 30 ms. Nhấn lần một cho phép chạy nếu line hợp lệ; nhấn lần hai làm PWM hai bên về 0; nhấn lần ba cho phép chạy lại.
3. Thử đủ tám pattern line và ghi PWM thực hai bên. `100/110` phải chỉ chạy bên trái và cua phải, `001/011` phải chỉ chạy bên phải và cua trái; các pattern khác phải đúng bảng quyết định phía trên.
4. Với `000` hoặc `101`, motor phải dừng dù BOOT đang ở trạng thái bật.
5. HC-SR04 no-echo liên tục không được ngăn nút BOOT hoặc line làm motor dừng.
6. Đo độ lệch xe trên đoạn thẳng để chốt trim trái/phải; không coi giá trị -20 là calibration production trước phép đo.

## Trạng thái bằng chứng

- Nhận line đen/trắng theo mức hiện tại: người dùng xác nhận hoạt động đúng trong test thủ công.
- Quan hệ đèn/GPIO của sensor hiện tại: một đèn = `1`, hai đèn = `0`; người dùng xác nhận từ hành vi xe đang chạy ổn.
- Chiều tiến motor sau đảo hai phía: người dùng xác nhận đã hết chạy ngược.
- Mapping theo hướng lắp thực tế (`100/110` chạy trái để cua phải, `001/011` chạy phải để cua trái) đã được người dùng thử trên xe và xác nhận hoạt động đúng.
- Servo, HC-SR04, PWM balance và toàn bộ tổ hợp line: cần ghi lại PASS/FAIL bằng log hoặc quan sát phần cứng trước khi tích hợp production.

## Tác động khi tích hợp production

Cần rà `Config.h`, `SensorModule`, `MotorModule`, `ServoModule`, `ControlLogic` và điểm đọc nút trong vòng điều khiển. Việc tích hợp phải giữ các interlock SAFE_STOP, obstacle, network và task hiện có; profile test không tự thay thế các hành vi an toàn production.

## Bổ sung tạm ngày 2026-10-03: nhận biết hàng và hai servo nâng

Người dùng muốn thêm một cảm biến hồng ngoại để nhận biết có hàng trên kệ hoặc trong khu vực chứa hàng trên xe (tạm gọi là **khay hàng/khay nâng**). Theo mô tả hiện tại, tín hiệu `HIGH` nghĩa là phát hiện hàng; khi xe ở bước lấy hàng và cảm biến xác nhận kệ có hàng thì hai servo nâng cùng hoạt động để lấy hàng. Đây là yêu cầu chức năng mới, **chưa được code và chưa thử trên xe**. Không suy ra GPIO, kiểu module IR, khoảng cách phát hiện, góc nâng hoặc thời gian chờ.

Một cảm biến ở một vị trí chỉ cho biết trạng thái trong vùng quan sát của nó. Chưa rõ cảm biến sẽ gắn trên xe để nhìn kệ khi tiếp cận, nhìn khay hàng trên xe, hay chuyển vùng quan sát theo cơ cấu nâng. Vì vậy `HIGH` chưa tự chứng minh được *hàng đang ở kệ* hay *hàng đã nằm trên xe*. Cần chốt vị trí và ý nghĩa theo từng giai đoạn; nếu cần phân biệt đồng thời hai vị trí có thể cần hai cảm biến hoặc một cách xác nhận khác. Mức `LOW`, tín hiệu đứt dây, nhiễu, chống rung và phản ứng khi không thấy hàng đều TBD. Không dùng `HIGH` đơn lẻ để bỏ qua điều kiện motor dừng, đúng node RFID, task pickup, mạng, vật cản và SAFE_STOP.

Hai servo nâng cùng một khay có thể nhận chung một tín hiệu điều khiển **chỉ khi** cùng loại tín hiệu/xung, cùng góc đích và chuyển động cơ khí đã được căn chỉnh. Khi đó không thể chỉnh riêng từng bên hay bù lệch, và vẫn phải cấp nguồn servo riêng đủ dòng, nối chung GND với ESP32; chân GPIO chỉ xuất tín hiệu, không cấp nguồn motor servo. Khuyến nghị giữ hai chân điều khiển riêng trong cấu hình để đồng bộ lệnh phần mềm nhưng hiệu chỉnh độc lập, bảo vệ khay khỏi kẹt/nghiêng. GPIO15 hiện dùng cho servo quét trong project test; GPIO17 và GPIO21 là hai tín hiệu servo còn lại theo bảng chân chính thức, **chưa tự gán** cho hai servo nâng trước khi xác nhận dây và cơ cấu.

Cần xác nhận: vị trí/model IR và điện áp ngõ ra có tương thích 3,3 V ESP32; chân GPIO còn trống; hai servo có thật sự là loại góc, chiều cơ khí, góc nâng/hạ từng bên, tải và nguồn; có công tắc hành trình/giới hạn hoặc cách xác nhận lấy hàng thành công không. Trước khi tích hợp production, chạy [TC-028 và TC-029](../../qa/test-cases.md) rồi cập nhật `REQ-016/017`. `AGV-PROFILE-001` cho line/motor/servo quét không mặc nhiên chứng minh phần nâng hàng này hoạt động.

Quyết định bổ sung sau đó: IR trên cụm nâng báo phát hiện hàng ở kho **về server trước**, chỉ nâng khi server gửi lệnh hợp lệ và interlock local còn tốt; khi đã chở hàng, IR giám sát mất hàng. Xem [hồ sơ IR hàng và lệnh nâng server](2026-10-03-cargo-ir-server-lift.md). Câu mô tả `HIGH` rồi nâng ở trên là ý tưởng ban đầu, không phải firmware v1 đã code hoặc lệnh tự nâng được giữ cho thiết kế mới.

## Quyết định line mới ngày 2026-10-03

Bảng `000/101` dừng ở trên vẫn mô tả **phiên bản project test đã thử**, không bị viết lại thành kết quả thử chưa xảy ra. Người dùng chốt firmware chính chỉ dừng vì line ở `000`, còn `101` chạy thẳng như `010/111`; xem [bản đồ/vị trí/line mới](2026-10-03-map-position-line.md). Project test chưa được sửa hoặc nghiệm thu lại theo bảng mới.


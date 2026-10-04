# Báo cáo phần nhúng xe AGV dành cho nhóm trưởng

Người soạn: Nhóm phát triển phần nhúng AGV. Người đọc: Nhóm trưởng và thành viên tích hợp server, web, cơ khí, điện. Ngày cập nhật: 04 tháng 10 năm 2026.

Đây là báo cáo độc lập. Toàn bộ phần giải thích, bảng GPIO, trạng thái, quy trình vận hành và kế hoạch kiểm thử nằm trong tài liệu này. Tài liệu không chứa liên kết sang file khác và không yêu cầu người đọc mở workspace hoặc mã nguồn để hiểu nội dung. Tên module và tên file được nhắc đến chỉ nhằm giải thích cấu trúc firmware.

Báo cáo trình bày phần nhúng đang có trong workspace để nhóm thống nhất cách vận hành và kế hoạch tích hợp. Firmware đã biên dịch cho ESP32 và đạt 49 ca kiểm thử logic native ở phiên ngày 03 tháng 10. Tuy nhiên, xe chưa được xác nhận chạy đúng trên phần cứng; nhiều thông số hiệu chuẩn, UID thẻ và cấu hình mạng còn trống. Kết luận hiện tại là có nền tảng phần mềm để tích hợp, chưa đủ bằng chứng nghiệm thu mô hình.

Tài liệu gồm năm chương. Nhóm trưởng cần đặc biệt nắm ranh giới giữa server và xe, quy tắc dừng, giới hạn của IR hàng và các điều kiện cần hoàn thành trước demo. Nội dung lấy từ source hiện tại và biên bản QA, không suy diễn số liệu chạy xe từ kết quả build.

## Chương 1 Tổng quan và phạm vi phần nhúng

### 1.1 Mục tiêu của xe AGV

Xe di chuyển trên đường line, nhận biết các vị trí bằng RFID, lấy hàng bằng cơ cấu nâng và giao hàng theo nhiệm vụ do server cấp. Encoder cung cấp quãng đường tương đối giữa các node để server hiển thị vị trí xe trên bản đồ. Xe đồng thời báo trạng thái nhiệm vụ, kết nối, cảm biến, pin và lỗi.

Hệ thống tổng thể dự kiến có hai xe và giao diện quản lý kho. Workspace hiện có firmware cho một xe theo cấu hình chung; mỗi xe phải có AGV_ID và topic riêng. Backend, web, thuật toán tìm đường và điều phối tránh xung đột giữa hai xe chưa có mã trong workspace này. Việc một firmware build được không chứng minh hệ thống hai xe đã hoạt động.

### 1.2 Phân chia trách nhiệm giữa server và ESP32

| Thành phần | Trách nhiệm |
| --- | --- |
| Server | Quản lý bản đồ, tìm đường, kiểm tra hướng đi hợp lệ, điều phối hai xe, giao nhiệm vụ, cấp lệnh rời node và cho phép nâng/hạ |
| ESP32 | Đọc cảm biến, xác nhận node bằng RFID, thi hành lệnh hợp lệ, bám line, điều khiển motor/servo/còi và dừng khi interlock không đạt |
| Web | Hiển thị vị trí, trạng thái, cảnh báo và thao tác vận hành thông qua server |

ESP32 không chạy A-Star và không tự chọn tuyến đến kho. Firmware vẫn kiểm tra nhiệm vụ, tên node, session, chỉ số đường đi và điều kiện vận hành; server không được bỏ qua các kiểm tra này khi gửi lệnh.

### 1.3 Bản đồ đã thống nhất

Bản đồ có 26 thẻ RFID: hai base, sáu kho và 18 node đường đi. Sáu kho là K1 đến K6. Các node đường đi gồm C1 đến C4, M1 đến M6, G1 đến G6 và GS1, GS2. Firmware đang gọi hai base là B1 và B2; đây là tên tạm cần thống nhất với server.

Các số 20, 35, 50, 70 và 100 trên sơ đồ là khoảng cách cm đo ngoài đời, không chỉ là trọng số tìm đường. Một cạnh 50 cm tương ứng 500 mm. Server cần lưu cả chiều dài thật và chiều đi của từng cạnh; tên node hợp lệ không có nghĩa một cặp node bất kỳ nối được với nhau.

### 1.4 Điều đã chốt và điều còn mở

Đã chốt: line đen trả HIGH; theo riêng luật line chỉ 000 gây mất line; 101 và 111 đi thẳng; sửa lái bằng một bên bánh; server cấp lệnh node trước khi xe tới; nâng và hạ đều cần lệnh server; lùi từ kho về RFID trước đó có còi; IR hàng HIGH là thấy hàng, LOW là không thấy.

Chưa hoàn tất: UID thật của 26 thẻ, định danh base, thông số encoder, ngưỡng siêu âm, thời gian quay, góc và nguồn servo, hiệu chuẩn pin, cấu hình broker/topic và kiểm tra điện. Chiều lùi đang bị khóa trong Config.h vì chưa thử với ba mắt line nằm trước xe.

## Chương 2 Phần cứng và phân chân GPIO

### 2.1 Các khối phần cứng

ESP32 DevKit V1 38 chân là bộ điều khiển dự kiến. Cụm truyền động được điều khiển theo hai phía trái/phải bằng driver L298N. Ba mắt line lắp trước xe; HC-SR04 đo vật cản trong vùng quan sát; hai encoder đếm xung độc lập. RC522 đọc RFID. Cơ cấu nâng sử dụng hai servo có tín hiệu riêng; một servo khác được dành cho vai trò quét theo cấu hình hiện tại. IR hàng nhìn lên từ cơ cấu nâng và còi cảnh báo phục vụ quá trình lấy, chở và lùi.

Nguồn motor, servo và logic cần được thiết kế theo tải thực, chung mass đúng sơ đồ và có khả năng ngắt nguồn. Chưa có BOM và số đo dòng để xác nhận nguồn hiện tại đủ tải.

### 2.2 Bảng phân chân hiện tại

| Nhóm tín hiệu | Chân thiết bị theo thứ tự | GPIO theo thứ tự |
| --- | --- | --- |
| Motor trái | ENA, IN1, IN2 | 14, 27, 26 |
| Motor phải | ENB, IN3, IN4 | 25, 33, 32 |
| Line | Trái, giữa, phải | 39, 34, 35 |
| HC-SR04 | TRIG, ECHO | 12, 13 |
| Encoder | Trái, phải | 4, 16 |
| RFID RC522 SPI | SCK, MISO, MOSI | 18, 19, 23 |
| RFID RC522 điều khiển | SDA/SS, RST | 5, 22 |
| Servo | Quét, nâng 1, nâng 2 | 15, 17, 21 |
| Đo điện áp pin B25 | OUT vào ADC1 | 36 |
| Còi lùi | Điều khiển active LOW | 0 |
| IR hàng hướng lên | OUT active HIGH | 2 |

Bảng dùng số GPIO, không phải thứ tự chân vật lý trên board. Tổng cộng 24 GPIO không trùng. Config.h là nguồn cấu hình duy nhất; không tự thay chân ở từng module. Hai servo nâng/hạ đang dùng GPIO17 và GPIO21, vì cơ cấu có thể cần góc hoặc chiều quay khác nhau. Các servo vẫn đang tắt trong cấu hình, chờ xác nhận cơ khí và hiệu chuẩn.

### 2.3 Điều kiện điện cần kiểm tra trước khi cấp tải

GPIO34, 35, 36 và 39 chỉ làm ngõ vào và không có pull-up/down nội. Ngõ ra cảm biến phải phù hợp với logic 3,3 V. Không đưa ECHO 5 V của HC-SR04 hoặc OUT 5 V của cảm biến khác trực tiếp vào ESP32; cần chuyển mức/chia áp đã kiểm tra. RC522 dùng nguồn 3,3 V; servo và motor không lấy nguồn tải từ GPIO.

GPIO0 là chân BOOT: mạch còi active LOW phải có đầu vào phù hợp, giữ GPIO0 HIGH khi reset. GPIO2 nối IR là chân strapping; tín hiệu HIGH có thể cản chế độ nạp, nên cần cách ngắt IR khi nạp và thử boot ở HIGH/LOW. GPIO5, 12 và 15 cũng cần kiểm mức khi reset. Phải xác nhận board dùng module WROOM phù hợp trước khi sử dụng GPIO16/17.

Việc dùng GPIO0/2 là revision để giữ các chân cũ, chưa phải lựa chọn đã được kiểm chứng bằng đo điện. Nếu không đáp ứng boot/nạp thì nhóm cần duyệt lại phân chân. Còi là cảnh báo, không phải cảm biến tránh vật cản sau xe.

### 2.4 PWM và tham số truyền động

Motor dùng PWM 5 kHz, độ phân giải 8 bit. Giá trị thử là 210; trim trái -20 và trim phải 0, nên khi chạy tiến thẳng lệnh tốc độ danh định là trái 190, phải 210. Hai phía đã bật đảo chiều theo profile thử. Đây là giá trị thử, chưa phải tốc độ hoặc sai số được đo dưới tải thật.

Servo dùng PWM 50 Hz, 16 bit, các channel 4 đến 6 tách khỏi nhóm motor 0 đến 3 trên core Arduino đang khóa. Cần đo waveform đồng thời khi motor và servo hoạt động. Không có cảm biến hành trình cơ cấu nâng, nên hết thời gian servo không tự chứng minh hàng đã được nâng đúng.

## Chương 3 Kiến trúc firmware và thuật toán

### 3.1 Cấu trúc mã nguồn

Firmware nằm trong NCKHAGV, xây dựng bằng PlatformIO với environment esp32dev, Arduino core 2.0.17 và C++17. main.cpp chỉ khởi tạo và gọi cập nhật controller. Config.h chứa GPIO, cấu hình và giá trị hiệu chuẩn; Types.h chứa trạng thái, lệnh, dữ liệu cảm biến và telemetry.

| Thư mục | Module chính | Vai trò |
| --- | --- | --- |
| src/core | AGVController, ControlLogic, CommandCodec | Điều phối thiết bị, FSM và kiểm tra JSON/lệnh |
| src/motion | MotorModule, MovementModule, LineFollowModule | Xuất PWM, thi hành hành động chuyển động và bám line |
| src/sensors | SensorModule, EncoderModule, RFIDModule, CargoModule, BatteryModule | Đọc cảm biến, đếm xung, xác nhận node, lọc IR hàng và đo pin |
| src/actuators | ServoModule, BuzzerModule | Cơ cấu nâng/quét và còi cảnh báo |
| src/network | WiFiModule, MQTTModule | Kết nối, hàng đợi lệnh/sự kiện và gửi dữ liệu |

ControlLogic là module production độc lập GPIO và được gọi trực tiếp trong native test; không phải bản thuật toán viết riêng để test. MQTT chạy trong task mạng riêng, trao đổi qua queue. RFID, encoder và MQTT không tự gọi motor; controller quyết định rồi MovementModule và MotorModule xuất hành động thực.

### 3.2 Máy trạng thái của xe

| Trạng thái | Ý nghĩa và hành vi |
| --- | --- |
| AGV_IDLE | Chưa có nhiệm vụ đang chạy, motor dừng |
| AGV_WAITING | Có nhiệm vụ nhưng chờ START, lệnh node hoặc phục hồi sau STOP; motor dừng |
| AGV_MOVING | Thi hành đoạn đường hiện tại, bám line hoặc quay/lùi theo lệnh hợp lệ |
| AGV_LOADING | Ở điểm lấy hàng, motor dừng, chờ server cho nâng và kiểm tra IR |
| AGV_UNLOADING | Ở điểm giao hàng, motor dừng, chờ server cho hạ và IR xác nhận không còn hàng |
| AGV_COMPLETED | Phát hoàn thành nhiệm vụ, sau đó chuyển về IDLE |
| AGV_SAFE_STOP | Dừng do interlock hoặc dừng khẩn, có latch; không tự chạy lại |
| AGV_ERROR | Lỗi triển khai/cấu hình hoặc nhiệm vụ khiến xe không thể tiếp tục; motor dừng |

AGV_WAITING không đồng nghĩa mọi lệnh đến đều làm xe chạy. Nếu xe bị STOP, cần START riêng; nếu chỉ thiếu NODE_ORDER trong luồng vận hành bình thường, lệnh phù hợp đến sau có thể cho xe tiếp tục. SAFE_STOP/ERROR cần phục hồi có kiểm tra điều kiện.

### 3.3 Bám line hiện tại không dùng PID

Firmware dùng luật rời rạc theo ba bit trái, giữa, phải. HIGH được chuẩn hóa thành 1. Không có PID tốc độ bánh hay PID bám line trong bản hiện tại. Encoder đang phục vụ quãng đường, không đóng vòng điều khiển tốc độ motor.

| Mẫu trái giữa phải | Hành động hiện có trong code |
| --- | --- |
| 000 | Dừng motor, báo LINE_LOST và vào SAFE_STOP |
| 010, 101, 111 | Chạy thẳng hai phía |
| 100, 110 | Gọi CorrectLeft; chỉ cấp tốc độ bên trái, bên phải dừng |
| 001, 011 | Gọi CorrectRight; bên trái dừng, chỉ cấp tốc độ bên phải |

Tên CorrectLeft/CorrectRight là tên hàm hiện tại. Hướng xe thực sự lệch như thế nào khi chỉ một bên bánh chạy phải đối chiếu lắp motor và đo trên xe, không kết luận chỉ từ tên hàm. Quy tắc chỉ dừng với 000 áp dụng cho xử lý line; vật cản, lỗi mạng hoặc mất hàng vẫn có quyền dừng.

Quay LEFT/RIGHT tại node là pivot bằng hai phía ngược chiều, khác với sửa lái bám line. Quay cần thời gian tối thiểu và timeout đã hiệu chuẩn; chỉ kết thúc khi đạt thời gian tối thiểu và đọc lại 010. Nếu gặp 000 hoặc quá timeout thì dừng. STRAIGHT có khoảng thời gian vượt vùng giao line trước khi trở về luật bám line, nhưng không bỏ qua 000.

### 3.4 RFID và encoder xác định vị trí

RFID ánh xạ UID thật sang tên node, chống lặp khi vẫn đang đứng trên cùng thẻ. Node chỉ được xác nhận bằng RFID; encoder không tự suy ra xe đã tới node. UID chưa có mapping hoặc node khác node kế tiếp trong nhiệm vụ có thể gây dừng và lỗi.

Hai encoder đếm cạnh lên bằng ISR ngắn, không xử lý mạng trong ngắt. Counter dùng uint64_t; snapshot/reset hai bên được bảo vệ trong cùng critical section. Khi sang node mới, controller giữ dữ liệu đoạn vừa đi để gửi event rồi reset đồng thời hai counter cho đoạn sau.

Quãng đường mỗi phía được tính bằng số xung chia cho PPR, nhân chu vi bánh. Trung bình hai phía là quãng đường tương đối; distanceMm bằng distanceCm nhân 10. PPR và đường kính bánh chưa điền thì dữ liệu khoảng cách là unknown/null, không giả lập bằng 0. Xung encoder một kênh không xác định được chiều; dấu âm khi lùi được gán theo hành động motor, không phải phép đo chiều độc lập.

### 3.5 IR hàng servo siêu âm và pin

CargoModule chỉ chấp nhận tín hiệu IR sau khi ổn định 150 ms. HIGH ở kho nghĩa là thấy hàng trong vùng nhìn, chưa tự kích hoạt servo. Hai servo nâng/hạ có góc riêng và chỉ chạy khi server cho phép đúng nhiệm vụ/node/giai đoạn. Sau nâng cần IR HIGH ổn định; khi đang chở, LOW ổn định gây CARGO_LOST và dừng. Sau hạ chỉ hoàn thành khi LOW ổn định.

Một IR không phân biệt hàng còn trên kệ với hàng đã lên khay nếu cả hai nằm trong vùng nhìn. Cần kiểm khi rời kệ. Đứt dây trả LOW có thể giống mất hàng; chưa có cảm biến hành trình hoặc lực nâng để phân biệt các trường hợp này.

SensorModule đo ECHO bằng interrupt; không echo, timeout hoặc dữ liệu quá cũ trở thành invalid và ngăn chuyển động, không coi là đường trống. BatteryModule đo ADC1 và quy đổi điện áp/phần trăm sau hiệu chuẩn. Chưa có chính sách pin yếu tự về base hoặc tự dừng trong firmware hiện tại.

## Chương 4 Quy trình vận hành và giao tiếp server

### 4.1 Từ khởi động đến bắt đầu nhiệm vụ

Khi khởi động, firmware đưa motor về trạng thái dừng, khởi tạo cảm biến và mạng; không tự đặt góc servo boot khi chưa được cấu hình. Xe cần nhận diện RFID vị trí hiện tại. Nếu xuất phát từ base 2, cần xác nhận B2 trước khi rời base; GS2 chỉ được xác nhận sau khi xe thực sự quét thẻ GS2.

Server gửi TASK chứa taskId, điểm lấy, điểm giao và path gồm các node theo thứ tự. startNode trong schema là điểm lấy hàng, không nhất thiết là phần tử đầu của path. Xe kiểm UID mapping toàn tuyến và kiểm nhiệm vụ trước khi nhận. START là lệnh riêng, chỉ được chấp nhận khi mạng, cảm biến và cấu hình bắt buộc đều đạt. Thiếu lệnh rời node đầu thì xe đứng chờ; cần NODE_ORDER hợp lệ cho B2 để bắt đầu đoạn B2 đến GS2.

### 4.2 Qua từng node với lệnh cấp trước

Trong lúc xe chạy đến GS2, server có thể cấp NODE_ORDER cho việc rời GS2 sang node tiếp theo. Lệnh chứa taskId, node, nextNode, pathIndex và departure; departure là LEFT, RIGHT, STRAIGHT hoặc BACKWARD. Firmware cache một lệnh chờ, không lưu cả loạt lệnh tự do cho nhiều node xa.

Khi RFID đọc GS2, xe cập nhật currentNode, phát AGV_REACHED_NODE với dữ liệu đoạn vừa đi, reset encoder và xét lệnh đã cache. Nếu lệnh phù hợp và không cần lấy/giao hàng hoặc dừng đổi chiều, xe có thể tiếp tục mà không chờ một vòng server mới. Nếu chưa có lệnh hợp lệ, motor dừng ở AGV_WAITING. Thiết kế này giảm dừng do độ trễ mạng nhưng chưa có số đo để kết luận chạy mượt.

Lệnh phải khớp nhiệm vụ, node rời, node đến và chỉ số path. Lệnh sai, trùng hoặc session cũ không được dùng để chạy. Chính server phải kiểm topology và hướng của xe trước khi chọn LEFT/RIGHT; ESP32 không có đồ thị đầy đủ để kiểm mọi cạnh hình học.

### 4.3 Ví dụ base 2 lấy ở K1 giao ở K2

1. Xe quét B2. Server cấp nhiệm vụ lấy K1, giao K2 và tuyến hợp lệ bắt đầu từ B2. Xe nhận START và lệnh rời B2 đến GS2.
2. Xe chạy đến GS2, báo đã tới GS2 và quãng đường đoạn B2 đến GS2. Server cấp trước các lệnh rời node tiếp theo; xe đi qua những node do server chọn đến G1 rồi K1.
3. Tại K1, xe dừng motor và vào LOADING. IR hướng lên được lọc rồi gửi AGV_PICKUP_IR. HIGH nghĩa thấy hàng trên kệ; LOW nghĩa chưa thấy hàng, firmware không chấp nhận nâng.
4. Server gửi LIFT_UP khớp taskId, K1 và pathIndex. Khi các interlock đạt, hai servo nâng cùng thao tác với góc đã hiệu chuẩn riêng. Xe chờ cơ cấu ổn định và kiểm IR.
5. Xe nhận BACKWARD từ K1 về đúng G1 vừa đi qua. Trong chế độ này, còi bật liên tục khi cả hai motor thực sự lùi. Lùi hiện vẫn bị khóa chờ thử phần cứng; mô tả bước này là hành vi dự kiến sau khi điều kiện QA đạt.
6. Khi quét lại G1, xe dừng ít nhất 150 ms để đổi pha, báo node và đoạn lùi, rồi xét lệnh đi tiếp. Chặng từ G1 tới K2 do server xác định theo chiều cạnh; báo cáo không tự đặt một tuyến chưa được xác nhận.
7. Trong quá trình chở, IR LOW ổn định gây dừng và CARGO_LOST. Nếu vẫn có hàng, xe tiếp tục đến node kho K2 rồi vào UNLOADING, motor dừng.
8. Server gửi LIFT_DOWN tại K2. Sau hạ, IR LOW ổn định cho phép phát AGV_TASK_COMPLETED. Nếu vẫn HIGH, xe đứng ở UNLOADING. Không tự có bước về base khi nhiệm vụ giao hàng đã kết thúc; cần nhiệm vụ/lệnh tiếp theo được nhóm thống nhất.

### 4.4 Telemetry cho bản đồ

POSITION/STATUS có node, nextNode, pathIndex và distanceMm; DISTANCE có số xung và quãng hai phía. Server dùng node RFID trước làm mốc, chiều dài cạnh cm thật và distanceMm để nội suy vị trí giữa hai node. Telemetry định kỳ hiện là 1.000 ms; firmware cũng cập nhật khi có node/trạng thái mới. Đây là hiển thị gần thời gian thực, không phải cập nhật liên tục từng mili giây.

Server cần giới hạn phần trăm tiến độ trên cạnh, xử lý đoạn lùi và báo dữ liệu cũ khi mất kết nối. Encoder hiện còn đếm xung pivot và có thể sai do trượt bánh, nên chưa phải odometry chính xác. Dấu distanceMm của snapshot phụ thuộc hành động motor tại thời điểm gửi; event kết thúc đoạn có cờ reverse riêng để diễn giải đoạn lùi.

### 4.5 Lệnh MQTT và ý nghĩa xác nhận

| Nhóm lệnh | Ý nghĩa |
| --- | --- |
| TASK và START | Nhận nhiệm vụ rồi yêu cầu bắt đầu riêng |
| NODE_ORDER | Cấp hành động rời node hiện tại hoặc node kế tiếp được phép cache |
| LIFT_UP và LIFT_DOWN | Cho phép nâng/hạ tại đúng node và pha nhiệm vụ |
| STOP và EMERGENCY_STOP | Dừng thường hoặc chốt dừng khẩn |
| RESTART và CANCEL_TASK | Kiểm điều kiện để gỡ latch hoặc hủy nhiệm vụ cần lập lại |

Giao thức dùng JSON version 1 trên sáu nhóm topic COMMAND, STATUS, POSITION, DISTANCE, EVENT và ERROR. Tên broker/topic và AGV_ID còn phải điền. Lệnh có ID để chống lặp và session của phiên kết nối; tuổi lệnh trong queue tối đa 2 giây. STOP/EMERGENCY_STOP có cơ chế ưu tiên riêng, không phụ thuộc queue còn chỗ để xe nhận yêu cầu dừng.

AGV_COMMAND_RESULT chỉ xác nhận lệnh được chấp nhận hay từ chối, không chứng minh servo đã nâng xong, xe đã tới đích hoặc server đã lưu dữ liệu. Hoàn thành phải dựa trên event và trạng thái phù hợp. MQTT hiện QoS 0, plaintext; queue sự kiện ở RAM có 16 phần tử, không bền qua reset và không bảo đảm giao tận nơi. Lỗi mạng có thể làm việc báo lỗi về server bị trễ dù motor đã dừng tại chỗ.

### 4.6 Dừng và phục hồi

Yêu cầu dừng ưu tiên được đọc trước khi phát hành động motor. Sau đó controller kiểm các điều kiện an toàn, xử lý RFID/lệnh và xuất hành động. Bất kỳ lệnh chạy thường nào cũng không được vượt SAFE_STOP hoặc interlock. Việc đổi trạng thái hay gửi MQTT không tự làm xe dừng: controller phải gọi MovementModule.stop và MotorModule.motorStop để đưa PWM về 0.

Các nguyên nhân dừng gồm vật cản, siêu âm invalid, line 000, mất mạng quá hạn, thẻ/node sai, timeout quay, mất hàng và lỗi queue. Mạng kết nối lại không tự làm xe chạy. RESTART chỉ gỡ latch khi đủ điều kiện; START vẫn là bước riêng. Quay hoặc servo bị gián đoạn cần CANCEL_TASK và kiểm tra/định vị lại, không tiếp tục mù giữa thao tác.

SAFE_STOP dừng motor nhưng không ngắt nguồn servo đã được phát lệnh. Phải có phương án ngắt điện và người giám sát khi thử cơ cấu. Không có cảm biến vật cản phía sau; còi không bảo đảm người hoặc vật không đi vào đường lùi.

## Chương 5 Kiểm thử hiện trạng và kế hoạch hoàn thiện

### 5.1 Bằng chứng phần mềm hiện có

| Hạng mục | Kết quả ghi nhận ngày 03 tháng 10 | Giới hạn |
| --- | --- | --- |
| Workspace Validate | PASS; liên kết, skill, REQ/TC và guard 15 module | Không kiểm dây hoặc hành vi vật lý |
| Native Unity | PASS 49 trên 49 test production | Không chạy GPIO, ISR trên chip, PWM hoặc broker thật |
| Build ESP32 | PASS environment esp32dev | Chỉ biên dịch, chưa upload |
| Kích thước firmware | RAM 49.336 byte; flash 798.761 byte | Không phải số đo độ trễ hoặc hiệu suất chạy xe |
| Tổ chức source | 29 file module chuyển vào 5 nhóm; export đủ 33 source/config files | Không đổi thuật toán hoặc GPIO |
| Hardware và tích hợp MQTT | NOT_RUN trong các phiên ghi nhận | Chưa xác nhận vận hành xe, mất mạng thực hoặc lấy/giao hàng |

Các test logic bao gồm cấu hình chân, phép tính encoder, mẫu line, nhiệm vụ sai, interlock mạng/siêu âm, lệnh node, ràng buộc lùi, điều kiện IR/nâng hạ và phục hồi sau STOP. Số 49 là tổng ca native; không có nghĩa 49 ca thực nghiệm trên robot đều đã đạt.

### 5.2 Những mục đang chặn vận hành

1. Hoàn thành kiểm tra nguồn và tín hiệu, đặc biệt GPIO0/2, ECHO siêu âm, ADC pin và dòng motor/servo. Thử cold boot và nạp với ngoại vi kết nối.
2. Điền UID đúng cho 26 thẻ và xác nhận B1/B2 với server. Đảm bảo xe quét được tag ở tốc độ và chiều cao lắp thực.
3. Đo PPR/đường kính bánh, ngưỡng vật cản, thời gian quay, khoảng vượt giao line, góc/pulse/settling cặp servo và hệ số pin. Không thay các giá trị 0/-1 bằng thông số phỏng đoán để ép START.
4. Kiểm tám mẫu line trên production và chiều motor dưới tải. Kiểm tra hướng sửa lái một bên bánh trước khi chạy tuyến.
5. Thử lùi K1 về G1 có giám sát; chỉ bật REVERSE_STEERING_CONFIRMED sau khi có bằng chứng phù hợp. Bản hiện tại lùi thẳng với pattern khác 000, chưa sửa lái lùi bằng thuật toán đã kiểm chứng.
6. Kiểm vị trí IR để HIGH sau khi rời kệ thực sự là hàng trên khay; thử hàng rơi, kho trống và LOW sau giao. Kiểm hai servo không làm kẹt hoặc nghiêng cơ cấu.
7. Hoàn thành broker, topic/ID từng xe, route có hướng và cơ chế server theo dõi ACK/event. Thử mất Wi-Fi, mất broker, lệnh trùng/cũ và reconnect không tự chạy.

### 5.3 Lộ trình tích hợp đề xuất cho nhóm

Giai đoạn đầu kiểm từng thiết bị trên bàn thử với bánh kê khỏi mặt đất và nguồn có thể ngắt. Sau đó chạy một đoạn thẳng, xác nhận RFID và encoder; tiếp theo mới thử node rẽ, lấy/giao hàng, lùi và lỗi. Mỗi ca ghi cấu hình, input, output thực, số lần lặp và tiêu chí trước khi kết luận PASS.

Khi một xe đủ bằng chứng, nhóm tích hợp server/web cho một nhiệm vụ đầy đủ. Chỉ chuyển sang hai xe sau khi thống nhất quyền chiếm node/cạnh, cách dừng khi xung đột, định danh topic và xử lý telemetry cũ. Không cho hai xe tự do dùng chung đường chỉ vì mỗi xe chạy riêng thành công.

### 5.4 Các quyết định nhóm trưởng cần chốt

Nhóm cần thống nhất tên base/UID, bản đồ có hướng và chiều dài từng cạnh; giao người phụ trách server phát lệnh cấp trước; giao điện/cơ khí kiểm boot, nguồn và cơ cấu nâng; giao phần nhúng hiệu chuẩn và ghi QA. Cần chốt tiêu chí dừng, sai số quãng đường, độ trễ hiển thị, số lần lấy/giao thành công và phạm vi an toàn cho demo. Các ngưỡng này hiện chưa có số đo hoặc quyết định đủ để ghi nghiệm thu.

PID không phải điều kiện để tích hợp giao thức và máy trạng thái hiện tại. Nếu thử dưới tải cho thấy tốc độ hai bên hoặc độ bám line chưa đạt tiêu chí, nhóm có thể lập yêu cầu điều khiển vòng kín/PID với phép đo và ca thử riêng. Báo cáo này không tự thêm PID hoặc thay thuật toán đã chốt.

### 5.5 Kết luận

Phần nhúng đã có cấu trúc module, FSM, giao tiếp MQTT, lệnh node cấp trước, kiểm hàng, nâng/hạ do server cho phép và các đường dừng motor. Kết quả build/native là cơ sở tiếp tục tích hợp, không thay bằng chứng thực nghiệm. Ưu tiên tiếp theo là kiểm điện, hiệu chuẩn và chạy một nhiệm vụ trọn vẹn trên một xe trước khi mở rộng hai xe.

Nội dung đã được đối chiếu với firmware và biên bản kiểm thử phần mềm ngày 03 tháng 10 năm 2026. Các bảng và mô tả cần thiết đã được trình bày trực tiếp trong năm chương; người đọc không cần truy cập các file khác. Những kết quả chưa thử trên xe thật vẫn giữ trạng thái chưa kiểm chứng và cần thực nghiệm trước khi nghiệm thu.

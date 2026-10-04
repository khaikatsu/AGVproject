# Bàn giao firmware AGV theo thứ tự yêu cầu

Được sinh từ source thực tế bằng `tools/workspace.ps1 -Action ExportFirmware`. Không sửa bản code trong tài liệu này. Cấu hình chưa biết được giữ TODO và chặn chuyển động khi thiếu điều kiện bắt buộc. Build/test phần mềm không xác nhận xe đủ an toàn để chạy.

## 1. Cấu trúc project

```text
NCKHAGV/
  platformio.ini
  src/
    Config.h
    Types.h
    main.cpp
    actuators/
      ServoModule.h
      ServoModule.cpp
      BuzzerModule.h
    core/
      AGVController.h
      AGVController.cpp
      ControlLogic.h
      ControlLogic.cpp
      CommandCodec.h
      CommandCodec.cpp
    motion/
      MotorModule.h
      MotorModule.cpp
      LineFollowModule.h
      LineFollowModule.cpp
      MovementModule.h
      MovementModule.cpp
    network/
      WiFiModule.h
      WiFiModule.cpp
      MQTTModule.h
      MQTTModule.cpp
    sensors/
      SensorModule.h
      SensorModule.cpp
      EncoderModule.h
      EncoderModule.cpp
      RFIDModule.h
      RFIDModule.cpp
      BatteryModule.h
      BatteryModule.cpp
      CargoModule.h
      CargoModule.cpp
  test/test_control/test_main.cpp
```

[Checklist cấu hình](06-configuration.md), [MQTT contract](04-mqtt-contract-v11.md), [QA](../qa/reports/2026-09-28-modular-firmware.md).

## 2. platformio.ini

Source: [NCKHAGV/platformio.ini](../NCKHAGV/platformio.ini).

```ini
[platformio]
default_envs = esp32dev

[env:esp32dev]
platform = espressif32 @ 6.13.0
board = esp32dev
framework = arduino
monitor_speed = 115200
build_unflags = -std=gnu++11
build_flags = -std=gnu++17 -Isrc
lib_deps =
    miguelbalboa/MFRC522 @ 1.4.12
    knolleary/PubSubClient @ 2.8
    bblanchon/ArduinoJson @ 6.21.5
test_ignore = test_control

[env:native]
platform = native @ 1.2.1
test_framework = unity
test_build_src = yes
build_flags = -std=c++17 -Isrc
build_src_filter = +<core/ControlLogic.cpp> +<core/CommandCodec.cpp>
lib_deps = bblanchon/ArduinoJson @ 6.21.5
```

Vai trò: Khóa nền tảng/thư viện Arduino và môi trường Unity native.

Giao tiếp: Toàn project PlatformIO.

Cần chỉnh: Upload port chỉ sau khi xác nhận board; không tự nâng core.

SHA-256 source: `eb6e0e5c1fb8c4bc4579585df371f159d55465104f8b11b43e7ef717af733c58`.

## 3. Config.h

Source: [NCKHAGV/src/Config.h](../NCKHAGV/src/Config.h).

```cpp
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace Config {
// Official wiring. Do not change these pins without a hardware revision.
constexpr int LEFT_ENA = 14;
constexpr int LEFT_IN1 = 27;
constexpr int LEFT_IN2 = 26;
constexpr int RIGHT_ENB = 25;
constexpr int RIGHT_IN3 = 33;
constexpr int RIGHT_IN4 = 32;
constexpr int LINE_LEFT_PIN = 39;
constexpr int LINE_CENTER_PIN = 34;
constexpr int LINE_RIGHT_PIN = 35;
constexpr int ULTRASONIC_TRIG_PIN = 12;
constexpr int ULTRASONIC_ECHO_PIN = 13;
constexpr int BATTERY_ADC_PIN = 36;
constexpr int RFID_SCK_PIN = 18;
constexpr int RFID_MISO_PIN = 19;
constexpr int RFID_MOSI_PIN = 23;
constexpr int RFID_SS_PIN = 5;
constexpr int RFID_RST_PIN = 22;
constexpr int ENCODER_LEFT_PIN = 4;
constexpr int ENCODER_RIGHT_PIN = 16;
constexpr int SERVO_1_PIN = 15;
constexpr int SERVO_2_PIN = 17;
constexpr int SERVO_3_PIN = 21;
// New wiring revision: both are strapping pins; see docs/03-hardware.md.
constexpr int BUZZER_PIN = 0;
constexpr int CARGO_IR_PIN = 2;
constexpr int BUZZER_ACTIVE_STATE = 0;
constexpr int CARGO_IR_ACTIVE_STATE = 1;
constexpr uint32_t CARGO_IR_STABLE_MS = 150;
constexpr uint32_t REVERSE_STOP_MS = 150;
// Front-mounted line sensors have not been validated for backward steering.
constexpr bool REVERSE_STEERING_CONFIRMED = false;

enum class MotorDriver { Unconfigured, L298N, BTS7960 };
// Verified in AGV-PROFILE-001 for forward motion; loaded-vehicle trim still needs measurement.
constexpr MotorDriver MOTOR_DRIVER = MotorDriver::L298N;
constexpr bool MOTOR_DIRECTION_CONFIRMED = true;
constexpr bool LEFT_FORWARD_INVERTED = true;
constexpr bool RIGHT_FORWARD_INVERTED = true;
constexpr int DEFAULT_MOTOR_SPEED = 210;
constexpr int LEFT_SPEED_TRIM = -20;
constexpr int RIGHT_SPEED_TRIM = 0;
constexpr int BASE_SPEED = DEFAULT_MOTOR_SPEED;
constexpr int TURN_SPEED = DEFAULT_MOTOR_SPEED;
constexpr int CORRECTION_SPEED = DEFAULT_MOTOR_SPEED;
constexpr uint32_t MOTOR_PWM_HZ = 5000;
constexpr int MOTOR_PWM_BITS = 8;
constexpr std::array<int, 4> MOTOR_PWM_CHANNELS = {0, 1, 2, 3};

// TODO: USER CONFIGURATION: 0 = active LOW, 1 = active HIGH, -1 = unknown.
constexpr int LINE_SENSOR_ACTIVE_STATE = 1;
// TODO: USER CONFIGURATION: measured safety and steering thresholds.
constexpr float OBSTACLE_DISTANCE_CM = 0.0f;
constexpr uint32_t TURN_MIN_MS = 0;
constexpr uint32_t TURN_TIMEOUT_MS = 0;
constexpr uint32_t STRAIGHT_DEPARTURE_MS = 0;
constexpr uint32_t ULTRASONIC_INTERVAL_MS = 60;
constexpr uint32_t ULTRASONIC_ECHO_TIMEOUT_US = 25000;
constexpr uint32_t ULTRASONIC_STALE_MS = 200;
constexpr uint32_t ULTRASONIC_TRIGGER_US = 10;
constexpr float ULTRASONIC_SOUND_CM_PER_US = 0.0343f;

// TODO: USER CONFIGURATION: measured pulses at the encoder and loaded wheel.
constexpr uint32_t ENCODER_PPR = 0;
constexpr float WHEEL_DIAMETER_CM = 0.0f;
constexpr float WHEEL_CIRCUMFERENCE_CM = 3.14159265358979323846f * WHEEL_DIAMETER_CM;

struct NodeMapping { const char* node; const char* uid; };
// TODO: USER CONFIGURATION: uppercase UID hex, no spaces/colons, never invent UIDs.
constexpr std::array<NodeMapping, 26> RFID_NODES = {{
    {"B1", ""}, {"B2", ""},
    {"K1", ""}, {"K2", ""}, {"K3", ""}, {"K4", ""}, {"K5", ""}, {"K6", ""},
    {"C1", ""}, {"C2", ""}, {"C3", ""}, {"C4", ""},
    {"M1", ""}, {"M2", ""}, {"M3", ""}, {"M4", ""}, {"M5", ""}, {"M6", ""},
    {"G1", ""}, {"G2", ""}, {"G3", ""}, {"G4", ""}, {"G5", ""}, {"G6", ""},
    {"GS1", ""}, {"GS2", ""}
}};
constexpr uint32_t RFID_POLL_INTERVAL_MS = 30;

constexpr std::array<int, 3> SERVO_PINS = {SERVO_1_PIN, SERVO_2_PIN, SERVO_3_PIN};
// Channels 4/5/6 use LEDC timers 2/3; motor channels 0..3 use timers 0/1.
constexpr std::array<int, 3> SERVO_PWM_CHANNELS = {4, 5, 6};
constexpr uint32_t SERVO_PWM_HZ = 50;
constexpr int SERVO_PWM_BITS = 16;
// TODO: USER CONFIGURATION: enable only verified servos and assign roles (0..2).
constexpr std::array<bool, 3> SERVO_ENABLED = {false, false, false};
constexpr int SERVO_SCAN = 0;
constexpr int SERVO_LIFT = 1;
constexpr int SERVO_OTHER = 2;
// SERVO_LIFT and SERVO_OTHER are the independently calibrated lift pair.
constexpr std::array<int, 2> SERVO_LIFT_UP_ANGLES = {-1, -1};
constexpr std::array<int, 2> SERVO_LIFT_DOWN_ANGLES = {-1, -1};
// TODO: USER CONFIGURATION: verified pulse limits, mechanical angles and settling.
constexpr int SERVO_MIN_PULSE_US = 0;
constexpr int SERVO_MAX_PULSE_US = 0;
constexpr uint32_t SERVO_MOVE_TIME_MS = 0;

// TODO: USER CONFIGURATION: B25/divider calibration and chemistry-dependent range.
// 0 means unconfigured, not a claim about the physical battery voltage.
constexpr float BATTERY_VOLTAGE_SCALE = 0.0f;
constexpr float BATTERY_VOLTAGE_OFFSET = 0.0f;
constexpr float BATTERY_EMPTY_VOLTAGE = 0.0f;
constexpr float BATTERY_FULL_VOLTAGE = 0.0f;
constexpr uint32_t BATTERY_SAMPLE_INTERVAL_MS = 500;

// TODO: USER CONFIGURATION: network identity, credentials and final topics.
constexpr const char* AGV_ID = "";
constexpr const char* WIFI_SSID = "";
constexpr const char* WIFI_PASSWORD = "";
constexpr const char* MQTT_SERVER = "";
constexpr uint16_t MQTT_PORT = 1883;
constexpr const char* MQTT_USERNAME = "";
constexpr const char* MQTT_PASSWORD = "";
constexpr const char* MQTT_TOPIC_COMMAND = "";
constexpr const char* MQTT_TOPIC_STATUS = "";
constexpr const char* MQTT_TOPIC_POSITION = "";
constexpr const char* MQTT_TOPIC_DISTANCE = "";
constexpr const char* MQTT_TOPIC_EVENT = "";
constexpr const char* MQTT_TOPIC_ERROR = "";
// TODO: USER CONFIGURATION: maximum loss intervals; 0 stops on first loss.
constexpr uint32_t WIFI_TIMEOUT_MS = 0;
constexpr uint32_t MQTT_TIMEOUT_MS = 0;

// Software limits/tuning, not measured hardware calibration.
constexpr uint32_t WIFI_RETRY_MS = 5000;
constexpr uint32_t MQTT_RETRY_MS = 3000;
constexpr uint16_t MQTT_SOCKET_TIMEOUT_SECONDS = 1;
constexpr uint16_t MQTT_KEEPALIVE_SECONDS = 5;
constexpr uint32_t NETWORK_WORKER_STALE_MS = 2000;
constexpr uint32_t COMMAND_MAX_AGE_MS = 2000;
constexpr uint32_t TELEMETRY_INTERVAL_MS = 1000;
constexpr uint32_t NETWORK_TASK_STACK_BYTES = 12288;
constexpr size_t MAX_PATH_NODES = 32;
constexpr size_t MAX_COMMAND_BYTES = 1024;
constexpr size_t MAX_PUBLISH_BYTES = 1536;
constexpr size_t COMMAND_QUEUE_LENGTH = 8;
constexpr size_t EVENT_QUEUE_LENGTH = 16;
constexpr size_t RECENT_COMMAND_IDS = 16;
constexpr bool DEBUG_ENABLED = true;
constexpr uint32_t SERIAL_BAUD = 115200;

constexpr std::array<int, 24> ALL_PINS = {
    LEFT_ENA, LEFT_IN1, LEFT_IN2, RIGHT_ENB, RIGHT_IN3, RIGHT_IN4,
    LINE_LEFT_PIN, LINE_CENTER_PIN, LINE_RIGHT_PIN,
    ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN, BATTERY_ADC_PIN,
    RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN, RFID_RST_PIN,
    ENCODER_LEFT_PIN, ENCODER_RIGHT_PIN, SERVO_1_PIN, SERVO_2_PIN, SERVO_3_PIN,
    BUZZER_PIN, CARGO_IR_PIN
};
constexpr bool pinsUnique() {
    for (size_t i = 0; i < ALL_PINS.size(); ++i)
        for (size_t j = i + 1; j < ALL_PINS.size(); ++j)
            if (ALL_PINS[i] == ALL_PINS[j]) return false;
    return true;
}
static_assert(pinsUnique(), "GPIO collision");
static_assert(LINE_LEFT_PIN >= 34 && LINE_CENTER_PIN >= 34 && LINE_RIGHT_PIN >= 34, "Line inputs must remain input-only pins");
static_assert(BATTERY_ADC_PIN == 36, "Battery must remain on ADC1 GPIO36");
static_assert(BUZZER_ACTIVE_STATE == 0 && CARGO_IR_ACTIVE_STATE == 1, "Unexpected buzzer/IR polarity");
static_assert(CARGO_IR_STABLE_MS > 0 && REVERSE_STOP_MS > 0, "Safety debounce/hold must be nonzero");
static_assert(BASE_SPEED >= 0 && BASE_SPEED <= 255 && TURN_SPEED >= 0 && TURN_SPEED <= 255 && CORRECTION_SPEED >= 0 && CORRECTION_SPEED <= 255, "Invalid PWM duty");
}
```

Vai trò: Nguồn duy nhất của GPIO và cấu hình, có static_assert chống trùng chân.

Giao tiếp: Tất cả module.

Cần chỉnh: Mọi nhóm TODO: USER CONFIGURATION; xem docs/06-configuration.md.

SHA-256 source: `dc13b72604dbdbbb59e1a4917af4b2725c10847ebd09ff722a31969c73ee3421`.

## 4. Types.h

Source: [NCKHAGV/src/Types.h](../NCKHAGV/src/Types.h).

```cpp
#pragma once

#include "Config.h"
#include <cstdint>
#include <limits>

constexpr size_t NODE_TEXT_SIZE = 8;
constexpr size_t UID_TEXT_SIZE = 21;
constexpr size_t COMMAND_ID_SIZE = 48;
constexpr float UNKNOWN_VALUE = std::numeric_limits<float>::quiet_NaN();

enum AGVState { AGV_IDLE, AGV_MOVING, AGV_LOADING, AGV_UNLOADING, AGV_WAITING,
                AGV_COMPLETED, AGV_SAFE_STOP, AGV_ERROR };
enum class Departure { FollowLine, Left, Right, Straight, Backward };
enum class MotorAction { Stop, Forward, Backward, CorrectLeft, CorrectRight, PivotLeft, PivotRight };
enum class ServoAction { None, LiftUp, LiftDown };
enum class CommandType { Task, Start, Stop, EmergencyStop, Restart, CancelTask, LiftUp, LiftDown, NodeOrder };
enum class ErrorCode { None, Configuration, WifiTimeout, MqttTimeout, EmergencyStop,
    Obstacle, SensorInvalid, LineLost, AmbiguousLine, UnknownTag, UnexpectedNode,
    TurnTimeout, TurnConfiguration, TurnInterrupted, ServoConfiguration, InvalidTask,
    TaskBusy, NoTask, NotConnected, NotAtRouteStart, RestartDenied, QueueOverflow,
    InvalidCommand, StaleCommand, CargoMissing, CargoLost,
    NodeOrderMismatch, ServoInterrupted };

struct PathStep { char node[NODE_TEXT_SIZE]{}; Departure departure = Departure::FollowLine; };
struct AGVTask {
    char id[COMMAND_ID_SIZE]{};
    char startNode[NODE_TEXT_SIZE]{};
    char targetNode[NODE_TEXT_SIZE]{};
    PathStep path[Config::MAX_PATH_NODES]{};
    uint8_t pathLength = 0;
    bool active = false;
};
struct AGVPosition { char node[NODE_TEXT_SIZE]{}; float distanceCm = UNKNOWN_VALUE; };
struct EncoderData {
    uint64_t leftPulses = 0, rightPulses = 0;
    float leftDistanceCm = UNKNOWN_VALUE, rightDistanceCm = UNKNOWN_VALUE;
    float averageDistanceCm = UNKNOWN_VALUE;
};
struct RFIDEvent { char uid[UID_TEXT_SIZE]{}; char node[NODE_TEXT_SIZE]{}; bool newNode = false; };
struct LineReadings {
    int left = 0, center = 0, right = 0;
    uint8_t pattern = 0;
    bool configured = false;
};
struct AGVCommand {
    CommandType type = CommandType::Stop;
    char id[COMMAND_ID_SIZE]{};
    uint32_t session = 0;
    uint32_t receivedAtMs = 0;
    AGVTask task{};
    char taskId[COMMAND_ID_SIZE]{};
    char node[NODE_TEXT_SIZE]{};
    char nextNode[NODE_TEXT_SIZE]{};
    uint8_t pathIndex = 0;
    Departure departure = Departure::FollowLine;
};
struct ControlInput {
    uint32_t now = 0;
    bool wifi = false, mqtt = false;
    bool motionConfigured = false, liftConfigured = false;
    bool distanceValid = false, obstacle = false, servoBusy = false;
    bool lineConfigured = false;
    uint8_t linePattern = 0;
    bool cargoStable = false, cargoPresent = false;
};
struct ControlOutput { MotorAction motor = MotorAction::Stop; ServoAction servo = ServoAction::None; };
struct SafetySettings {
    uint32_t wifiTimeoutMs = 0, mqttTimeoutMs = 0;
    uint32_t turnMinMs = 0, turnTimeoutMs = 0, straightDepartureMs = 0;
    uint32_t servoMoveMs = 0;
    bool requireNodeOrders = false, requireLiftCommands = false;
    bool reverseSteeringConfirmed = false;
};
struct TelemetrySnapshot {
    AGVState state = AGV_IDLE;
    ErrorCode error = ErrorCode::None;
    AGVPosition position{};
    char nextNode[NODE_TEXT_SIZE]{};
    EncoderData encoder{};
    char taskId[COMMAND_ID_SIZE]{};
    bool taskActive = false, wifi = false, mqtt = false;
    bool batteryCalibrated = false;
    uint8_t pathIndex = 0, pathLength = 0;
    int leftDuty = 0, rightDuty = 0;
    MotorAction motor = MotorAction::Stop;
    float ultrasonicCm = UNKNOWN_VALUE;
    uint32_t batteryAdcMv = 0, uptimeMs = 0;
    float batteryVoltage = UNKNOWN_VALUE, batteryPercentage = UNKNOWN_VALUE;
    bool cargoStable = false, cargoPresent = false;
    bool buzzerOn = false;
};

const char* stateName(AGVState state);
const char* errorName(ErrorCode error);
const char* motorActionName(MotorAction action);
```

Vai trò: Enum và struct có kích thước giới hạn, dùng chung giữa controller, queue và telemetry.

Giao tiếp: Tất cả module.

Cần chỉnh: Không có hiệu chuẩn cần điền tại đây.

SHA-256 source: `e5e969fdf6c4390b11b65146a450cce29c6d94e912d3b2bad61a9a579ee9bdc4`.

## 5. MotorModule.h

Source: [NCKHAGV/src/motion/MotorModule.h](../NCKHAGV/src/motion/MotorModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include "Types.h"

class MotorModule {
public:
    void begin();
    void motorForward();
    void motorBackward();
    void motorLeft();
    void motorRight();
    void motorStop();
    void setLeftSpeed(int speed);
    void setRightSpeed(int speed);
    void setMotorSpeed(int speed);
    bool configured() const;
    int leftDuty() const { return appliedLeft_; }
    int rightDuty() const { return appliedRight_; }
private:
    void drive(int left, int right);
    void writeSide(bool left, int duty);
    int leftSpeed_ = Config::DEFAULT_MOTOR_SPEED;
    int rightSpeed_ = Config::DEFAULT_MOTOR_SPEED;
    int appliedLeft_ = 0, appliedRight_ = 0;
    bool initialized_ = false;
};
```

Vai trò: Khai báo API. Quản lý PWM và hai phía motor, hỗ trợ L298N hoặc adapter BTS7960.

Giao tiếp: MovementModule và LineFollowModule.

Cần chỉnh: MOTOR_DRIVER, MOTOR_DIRECTION_CONFIRMED, inversion và các speed.

SHA-256 source: `ad1796ad88cf5a3accbae1f887e4cf7c2f4f25e01638a7293252f1eb4c0859e6`.

## 6. MotorModule.cpp

Source: [NCKHAGV/src/motion/MotorModule.cpp](../NCKHAGV/src/motion/MotorModule.cpp).

```cpp
#include "MotorModule.h"

bool MotorModule::configured() const {
    return initialized_ && Config::MOTOR_DRIVER != Config::MotorDriver::Unconfigured && Config::MOTOR_DIRECTION_CONFIRMED;
}
void MotorModule::begin() {
    for (int pin : {Config::LEFT_ENA, Config::LEFT_IN1, Config::LEFT_IN2,
                    Config::RIGHT_ENB, Config::RIGHT_IN3, Config::RIGHT_IN4}) {
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
    }
    if (Config::MOTOR_DRIVER == Config::MotorDriver::L298N) {
        for (int channel : {Config::MOTOR_PWM_CHANNELS[0], Config::MOTOR_PWM_CHANNELS[1]}) {
            ledcSetup(channel, Config::MOTOR_PWM_HZ, Config::MOTOR_PWM_BITS);
            ledcWrite(channel, 0);
        }
        ledcAttachPin(Config::LEFT_ENA, Config::MOTOR_PWM_CHANNELS[0]);
        ledcAttachPin(Config::RIGHT_ENB, Config::MOTOR_PWM_CHANNELS[1]);
    } else if (Config::MOTOR_DRIVER == Config::MotorDriver::BTS7960) {
        const int pins[] = {Config::LEFT_IN1, Config::LEFT_IN2, Config::RIGHT_IN3, Config::RIGHT_IN4};
        for (size_t i = 0; i < Config::MOTOR_PWM_CHANNELS.size(); ++i) {
            ledcSetup(Config::MOTOR_PWM_CHANNELS[i], Config::MOTOR_PWM_HZ, Config::MOTOR_PWM_BITS);
            ledcWrite(Config::MOTOR_PWM_CHANNELS[i], 0);
            ledcAttachPin(pins[i], Config::MOTOR_PWM_CHANNELS[i]);
        }
    }
    initialized_ = true;
    motorStop();
}
void MotorModule::setLeftSpeed(int speed) { leftSpeed_ = constrain(speed, 0, 255); }
void MotorModule::setRightSpeed(int speed) { rightSpeed_ = constrain(speed, 0, 255); }
void MotorModule::setMotorSpeed(int speed) {
    setLeftSpeed(speed + Config::LEFT_SPEED_TRIM);
    setRightSpeed(speed + Config::RIGHT_SPEED_TRIM);
}
void MotorModule::motorForward() { drive(leftSpeed_, rightSpeed_); }
void MotorModule::motorBackward() { drive(-leftSpeed_, -rightSpeed_); }
void MotorModule::motorLeft() { drive(-leftSpeed_, rightSpeed_); }
void MotorModule::motorRight() { drive(leftSpeed_, -rightSpeed_); }
void MotorModule::motorStop() {
    if (!initialized_) return;
    writeSide(true, 0);
    writeSide(false, 0);
    appliedLeft_ = appliedRight_ = 0;
}
void MotorModule::drive(int left, int right) {
    if (!configured()) { motorStop(); return; }
    writeSide(true, left);
    writeSide(false, right);
    appliedLeft_ = left;
    appliedRight_ = right;
}
void MotorModule::writeSide(bool left, int duty) {
    const int enable = left ? Config::LEFT_ENA : Config::RIGHT_ENB;
    const int a = left ? Config::LEFT_IN1 : Config::RIGHT_IN3;
    const int b = left ? Config::LEFT_IN2 : Config::RIGHT_IN4;
    const bool inverted = left ? Config::LEFT_FORWARD_INVERTED : Config::RIGHT_FORWARD_INVERTED;
    if (inverted) duty = -duty;
    if (Config::MOTOR_DRIVER == Config::MotorDriver::L298N) {
        const int channel = Config::MOTOR_PWM_CHANNELS[left ? 0 : 1];
        ledcWrite(channel, 0);
        digitalWrite(a, duty > 0 ? HIGH : LOW);
        digitalWrite(b, duty < 0 ? HIGH : LOW);
        ledcWrite(channel, abs(duty));
    } else if (Config::MOTOR_DRIVER == Config::MotorDriver::BTS7960) {
        // Per-side adapter: enable -> joined R_EN/L_EN, a -> RPWM, b -> LPWM.
        const size_t index = left ? 0 : 2;
        digitalWrite(enable, LOW);
        ledcWrite(Config::MOTOR_PWM_CHANNELS[index], 0);
        ledcWrite(Config::MOTOR_PWM_CHANNELS[index + 1], 0);
        if (duty) {
            ledcWrite(Config::MOTOR_PWM_CHANNELS[index + (duty < 0 ? 1 : 0)], abs(duty));
            digitalWrite(enable, HIGH);
        }
    } else {
        digitalWrite(enable, LOW);
        digitalWrite(a, LOW);
        digitalWrite(b, LOW);
    }
}
```

Vai trò: Triển khai API. Quản lý PWM và hai phía motor, hỗ trợ L298N hoặc adapter BTS7960.

Giao tiếp: MovementModule và LineFollowModule.

Cần chỉnh: MOTOR_DRIVER, MOTOR_DIRECTION_CONFIRMED, inversion và các speed.

SHA-256 source: `2e39ed60f45e59c7ca02aa1a71f8573ce0d8bed6a9dd0e8498173c24d8e30e76`.

## 7. SensorModule.h

Source: [NCKHAGV/src/sensors/SensorModule.h](../NCKHAGV/src/sensors/SensorModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include "Types.h"

class SensorModule {
public:
    void begin();
    void update();
    float readDistanceCm() const;
    bool isObstacleDetected() const;
    bool valid() const;
private:
    static void IRAM_ATTR echoInterrupt(void* argument);
    portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
    volatile bool waiting_ = false, risingSeen_ = false, ready_ = false;
    volatile uint32_t risingUs_ = 0, durationUs_ = 0;
    uint32_t triggerUs_ = 0, lastTriggerMs_ = 0, lastResultMs_ = 0;
    float distanceCm_ = UNKNOWN_VALUE;
};
```

Vai trò: Khai báo API. Đo HC-SR04 bằng echo interrupt; timeout/stale trả invalid, không giả lập đường trống.

Giao tiếp: AGVController.

Cần chỉnh: OBSTACLE_DISTANCE_CM; đo lại timing nếu cần.

SHA-256 source: `e38d3f76de7167ae222e090b341385bd7ed832d0d251c7255fd76993bdc223e3`.

## 8. SensorModule.cpp

Source: [NCKHAGV/src/sensors/SensorModule.cpp](../NCKHAGV/src/sensors/SensorModule.cpp).

```cpp
#include "SensorModule.h"
#include <cmath>

void SensorModule::begin() {
    digitalWrite(Config::ULTRASONIC_TRIG_PIN, LOW);
    pinMode(Config::ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(Config::ULTRASONIC_ECHO_PIN, INPUT);
    attachInterruptArg(Config::ULTRASONIC_ECHO_PIN, echoInterrupt, this, CHANGE);
    lastTriggerMs_ = millis() - Config::ULTRASONIC_INTERVAL_MS;
}
void IRAM_ATTR SensorModule::echoInterrupt(void* argument) {
    auto* self = static_cast<SensorModule*>(argument);
    const uint32_t now = micros();
    const bool high = digitalRead(Config::ULTRASONIC_ECHO_PIN) == HIGH;
    portENTER_CRITICAL_ISR(&self->mux_);
    if (self->waiting_) {
        if (high) { self->risingUs_ = now; self->risingSeen_ = true; }
        else if (self->risingSeen_) {
            self->durationUs_ = now - self->risingUs_;
            self->ready_ = true;
            self->waiting_ = false;
        }
    }
    portEXIT_CRITICAL_ISR(&self->mux_);
}
void SensorModule::update() {
    const uint32_t now = millis();
    uint32_t duration = 0;
    bool ready, waiting;
    portENTER_CRITICAL(&mux_);
    ready = ready_;
    if (ready) { duration = durationUs_; ready_ = false; }
    if (waiting_ && static_cast<uint32_t>(micros() - triggerUs_) >= Config::ULTRASONIC_ECHO_TIMEOUT_US) {
        waiting_ = false;
        distanceCm_ = UNKNOWN_VALUE;
    }
    waiting = waiting_;
    portEXIT_CRITICAL(&mux_);
    if (ready) {
        distanceCm_ = duration > 0 && duration <= Config::ULTRASONIC_ECHO_TIMEOUT_US ? duration * Config::ULTRASONIC_SOUND_CM_PER_US / 2.0f : UNKNOWN_VALUE;
        lastResultMs_ = now;
    }
    if (!waiting && static_cast<uint32_t>(now - lastTriggerMs_) >= Config::ULTRASONIC_INTERVAL_MS) {
        portENTER_CRITICAL(&mux_);
        waiting_ = true;
        risingSeen_ = false;
        triggerUs_ = micros();
        portEXIT_CRITICAL(&mux_);
        digitalWrite(Config::ULTRASONIC_TRIG_PIN, HIGH);
        delayMicroseconds(Config::ULTRASONIC_TRIGGER_US);
        digitalWrite(Config::ULTRASONIC_TRIG_PIN, LOW);
        lastTriggerMs_ = now;
    }
}
bool SensorModule::valid() const {
    return std::isfinite(distanceCm_) && static_cast<uint32_t>(millis() - lastResultMs_) <= Config::ULTRASONIC_STALE_MS;
}
float SensorModule::readDistanceCm() const { return valid() ? distanceCm_ : UNKNOWN_VALUE; }
bool SensorModule::isObstacleDetected() const { return valid() && readDistanceCm() <= Config::OBSTACLE_DISTANCE_CM; }
```

Vai trò: Triển khai API. Đo HC-SR04 bằng echo interrupt; timeout/stale trả invalid, không giả lập đường trống.

Giao tiếp: AGVController.

Cần chỉnh: OBSTACLE_DISTANCE_CM; đo lại timing nếu cần.

SHA-256 source: `5d4d920ac51a723dab9772a11e16e8b8337b4d9f6a60172b68b2ff971f54e10a`.

## 9. EncoderModule.h

Source: [NCKHAGV/src/sensors/EncoderModule.h](../NCKHAGV/src/sensors/EncoderModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include "Types.h"

class EncoderModule {
public:
    void begin();
    void reset();
    void resetLeft();
    void resetRight();
    void update();
    uint64_t getPulseCount() const;
    uint64_t getLeftPulseCount() const;
    uint64_t getRightPulseCount() const;
    float getDistance() const { return getDistanceCm(); }
    float getDistanceCm() const;
    float getLeftDistanceCm() const;
    float getRightDistanceCm() const;
    EncoderData snapshot() const;
    EncoderData snapshotAndReset();
private:
    static void IRAM_ATTR leftInterrupt(void* argument);
    static void IRAM_ATTR rightInterrupt(void* argument);
    static EncoderData convert(uint64_t left, uint64_t right);
    mutable portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
    volatile uint64_t left_ = 0, right_ = 0;
};
```

Vai trò: Khai báo API. Hai ISR độc lập; chụp/reset đồng thời hai counter và đổi xung sang cm khi đã hiệu chuẩn.

Giao tiếp: AGVController và AGVMath.

Cần chỉnh: ENCODER_PPR và WHEEL_DIAMETER_CM.

SHA-256 source: `b66f4473c4820883556f0718346f19b85ca898853970c08e4735a0915fdf4744`.

## 10. EncoderModule.cpp

Source: [NCKHAGV/src/sensors/EncoderModule.cpp](../NCKHAGV/src/sensors/EncoderModule.cpp).

```cpp
#include "EncoderModule.h"
#include "core/ControlLogic.h"

void EncoderModule::begin() {
    pinMode(Config::ENCODER_LEFT_PIN, INPUT);
    pinMode(Config::ENCODER_RIGHT_PIN, INPUT);
    reset();
    attachInterruptArg(Config::ENCODER_LEFT_PIN, leftInterrupt, this, RISING);
    attachInterruptArg(Config::ENCODER_RIGHT_PIN, rightInterrupt, this, RISING);
}
void IRAM_ATTR EncoderModule::leftInterrupt(void* argument) {
    auto* self = static_cast<EncoderModule*>(argument);
    portENTER_CRITICAL_ISR(&self->mux_);
    ++self->left_;
    portEXIT_CRITICAL_ISR(&self->mux_);
}
void IRAM_ATTR EncoderModule::rightInterrupt(void* argument) {
    auto* self = static_cast<EncoderModule*>(argument);
    portENTER_CRITICAL_ISR(&self->mux_);
    ++self->right_;
    portEXIT_CRITICAL_ISR(&self->mux_);
}
void EncoderModule::reset() { (void)snapshotAndReset(); }
void EncoderModule::resetLeft() { portENTER_CRITICAL(&mux_); left_ = 0; portEXIT_CRITICAL(&mux_); }
void EncoderModule::resetRight() { portENTER_CRITICAL(&mux_); right_ = 0; portEXIT_CRITICAL(&mux_); }
void EncoderModule::update() { /* ISR counters are the live source; snapshots are atomic. */ }
EncoderData EncoderModule::convert(uint64_t left, uint64_t right) {
    EncoderData data;
    data.leftPulses = left;
    data.rightPulses = right;
    data.leftDistanceCm = AGVMath::distanceCm(left, Config::ENCODER_PPR, Config::WHEEL_DIAMETER_CM);
    data.rightDistanceCm = AGVMath::distanceCm(right, Config::ENCODER_PPR, Config::WHEEL_DIAMETER_CM);
    data.averageDistanceCm = (data.leftDistanceCm + data.rightDistanceCm) / 2.0f;
    return data;
}
EncoderData EncoderModule::snapshot() const {
    portENTER_CRITICAL(&mux_);
    const uint64_t left = left_, right = right_;
    portEXIT_CRITICAL(&mux_);
    return convert(left, right);
}
EncoderData EncoderModule::snapshotAndReset() {
    portENTER_CRITICAL(&mux_);
    const uint64_t left = left_, right = right_;
    left_ = right_ = 0;
    portEXIT_CRITICAL(&mux_);
    return convert(left, right);
}
uint64_t EncoderModule::getLeftPulseCount() const { return snapshot().leftPulses; }
uint64_t EncoderModule::getRightPulseCount() const { return snapshot().rightPulses; }
uint64_t EncoderModule::getPulseCount() const { auto data = snapshot(); return data.leftPulses / 2 + data.rightPulses / 2 + (data.leftPulses % 2 + data.rightPulses % 2) / 2; }
float EncoderModule::getDistanceCm() const { return snapshot().averageDistanceCm; }
float EncoderModule::getLeftDistanceCm() const { return snapshot().leftDistanceCm; }
float EncoderModule::getRightDistanceCm() const { return snapshot().rightDistanceCm; }
```

Vai trò: Triển khai API. Hai ISR độc lập; chụp/reset đồng thời hai counter và đổi xung sang cm khi đã hiệu chuẩn.

Giao tiếp: AGVController và AGVMath.

Cần chỉnh: ENCODER_PPR và WHEEL_DIAMETER_CM.

SHA-256 source: `4520d53e4166dd085566d108f2659775969d76c14823706dee010de1aaca3be9`.

## 11. RFIDModule.h

Source: [NCKHAGV/src/sensors/RFIDModule.h](../NCKHAGV/src/sensors/RFIDModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include <MFRC522.h>
#include "Types.h"

class RFIDModule {
public:
    void begin();
    void update() { readCard(); }
    bool readCard();
    const char* getUID() const { return event_.uid; }
    const char* getNodeFromUID(const char* uid) const;
    bool isNewNode() const { return event_.newNode; }
    bool hasMapping(const char* node) const;
    const RFIDEvent& event() const { return event_; }
private:
    static bool validUid(const char* uid);
    MFRC522 reader_{Config::RFID_SS_PIN, Config::RFID_RST_PIN};
    RFIDEvent event_{};
    char lastNode_[NODE_TEXT_SIZE]{};
    uint32_t lastPollMs_ = 0;
    bool mappingsValid_ = true;
};
```

Vai trò: Khai báo API. Đọc UID RC522, ánh xạ 26 node và chống lặp cùng node; không điều khiển motor.

Giao tiếp: AGVController.

Cần chỉnh: RFID_NODES, UID thật dạng hex chữ hoa.

SHA-256 source: `1dc4a4e6d79a9ea5b6718ca19f304454eada8d66b24caca9b9a9287edd732929`.

## 12. RFIDModule.cpp

Source: [NCKHAGV/src/sensors/RFIDModule.cpp](../NCKHAGV/src/sensors/RFIDModule.cpp).

```cpp
#include "RFIDModule.h"
#include "core/ControlLogic.h"
#include <SPI.h>
#include <cstring>

bool RFIDModule::validUid(const char* uid) {
    const size_t length = std::strlen(uid);
    if (length != 8 && length != 14 && length != 20) return false;
    for (size_t i = 0; i < length; ++i)
        if (!((uid[i] >= '0' && uid[i] <= '9') || (uid[i] >= 'A' && uid[i] <= 'F'))) return false;
    return true;
}
void RFIDModule::begin() {
    SPI.begin(Config::RFID_SCK_PIN, Config::RFID_MISO_PIN, Config::RFID_MOSI_PIN, Config::RFID_SS_PIN);
    reader_.PCD_Init();
    for (size_t i = 0; i < Config::RFID_NODES.size(); ++i) {
        const char* uid = Config::RFID_NODES[i].uid;
        if (!uid[0]) continue;
        if (!validUid(uid)) mappingsValid_ = false;
        for (size_t j = i + 1; j < Config::RFID_NODES.size(); ++j)
            if (std::strcmp(uid, Config::RFID_NODES[j].uid) == 0) mappingsValid_ = false;
    }
    if (!mappingsValid_ && Config::DEBUG_ENABLED) Serial.println("[RFID] Invalid/duplicate UID configuration");
}
const char* RFIDModule::getNodeFromUID(const char* uid) const {
    if (!mappingsValid_ || !uid || !validUid(uid)) return "";
    for (const auto& mapping : Config::RFID_NODES)
        if (std::strcmp(uid, mapping.uid) == 0) return mapping.node;
    return "";
}
bool RFIDModule::hasMapping(const char* node) const {
    if (!mappingsValid_) return false;
    for (const auto& mapping : Config::RFID_NODES)
        if (std::strcmp(node, mapping.node) == 0) return validUid(mapping.uid);
    return false;
}
bool RFIDModule::readCard() {
    event_.newNode = false;
    if (static_cast<uint32_t>(millis() - lastPollMs_) < Config::RFID_POLL_INTERVAL_MS) return false;
    lastPollMs_ = millis();
    if (!reader_.PICC_IsNewCardPresent() || !reader_.PICC_ReadCardSerial()) return false;
    if (reader_.uid.size != 4 && reader_.uid.size != 7 && reader_.uid.size != 10) {
        reader_.PICC_HaltA();
        reader_.PCD_StopCrypto1();
        return false;
    }
    const char* hex = "0123456789ABCDEF";
    for (size_t i = 0; i < reader_.uid.size; ++i) {
        event_.uid[i * 2] = hex[reader_.uid.uidByte[i] >> 4];
        event_.uid[i * 2 + 1] = hex[reader_.uid.uidByte[i] & 15];
    }
    event_.uid[reader_.uid.size * 2] = '\0';
    AGVMath::copyText(event_.node, sizeof(event_.node), getNodeFromUID(event_.uid));
    event_.newNode = event_.node[0] && std::strcmp(event_.node, lastNode_) != 0;
    if (!event_.node[0]) lastNode_[0] = '\0';
    if (event_.newNode) AGVMath::copyText(lastNode_, sizeof(lastNode_), event_.node);
    reader_.PICC_HaltA();
    reader_.PCD_StopCrypto1();
    if (Config::DEBUG_ENABLED) Serial.printf("[RFID] UID=%s node=%s new=%d\n", event_.uid, event_.node, event_.newNode);
    return true;
}
```

Vai trò: Triển khai API. Đọc UID RC522, ánh xạ 26 node và chống lặp cùng node; không điều khiển motor.

Giao tiếp: AGVController.

Cần chỉnh: RFID_NODES, UID thật dạng hex chữ hoa.

SHA-256 source: `55511810ff6a8380699440568db78a8bb8ef0cb017badc15f34cf420636d7ef0`.

## 13. ServoModule.h

Source: [NCKHAGV/src/actuators/ServoModule.h](../NCKHAGV/src/actuators/ServoModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include "Types.h"

class ServoModule {
public:
    void begin();
    void update();
    bool liftUp();
    bool liftDown();
    bool setAngle(size_t servo, int angle);
    int getAngle(size_t servo) const;
    bool liftConfigured() const;
    bool busy() const { return busy_; }
private:
    bool rolesValid() const;
    bool attached_[3]{};
    int angles_[3] = {-1, -1, -1};
    bool busy_ = false;
    uint32_t movedAtMs_ = 0;
};
```

Vai trò: Khai báo API. PWM servo tách timer motor; không đặt góc lúc boot; phối hợp cặp servo nâng/hạ theo lệnh server.

Giao tiếp: AGVController.

Cần chỉnh: SERVO_ENABLED, vai trò, pulse limits, góc nâng/hạ và thời gian ổn định.

SHA-256 source: `4785051fff6af6ef179eff0e81ecd99b4116aecf9160be965e0df2e3c6a8237a`.

## 14. ServoModule.cpp

Source: [NCKHAGV/src/actuators/ServoModule.cpp](../NCKHAGV/src/actuators/ServoModule.cpp).

```cpp
#include "ServoModule.h"

bool ServoModule::rolesValid() const {
    const int roles[] = {Config::SERVO_SCAN, Config::SERVO_LIFT, Config::SERVO_OTHER};
    for (size_t i = 0; i < 3; ++i) {
        if (roles[i] < -1 || roles[i] > 2) return false;
        for (size_t j = i + 1; j < 3; ++j)
            if (roles[i] >= 0 && roles[i] == roles[j]) return false;
    }
    return true;
}
void ServoModule::begin() {
    if (!rolesValid() || Config::SERVO_MIN_PULSE_US <= 0 || Config::SERVO_MAX_PULSE_US <= Config::SERVO_MIN_PULSE_US ||
        Config::SERVO_MAX_PULSE_US >= 1000000 / static_cast<int>(Config::SERVO_PWM_HZ)) return;
    for (size_t i = 0; i < Config::SERVO_PINS.size(); ++i) {
        if (!Config::SERVO_ENABLED[i]) continue;
        ledcSetup(Config::SERVO_PWM_CHANNELS[i], Config::SERVO_PWM_HZ, Config::SERVO_PWM_BITS);
        ledcWrite(Config::SERVO_PWM_CHANNELS[i], 0);
        ledcAttachPin(Config::SERVO_PINS[i], Config::SERVO_PWM_CHANNELS[i]);
        attached_[i] = true;
    }
}
bool ServoModule::liftConfigured() const {
    const int pair[] = {Config::SERVO_LIFT, Config::SERVO_OTHER};
    if (pair[0] < 0 || pair[0] >= 3 || pair[1] < 0 || pair[1] >= 3 ||
        pair[0] == pair[1] || !attached_[pair[0]] || !attached_[pair[1]] ||
        !Config::SERVO_MOVE_TIME_MS) return false;
    for (size_t i = 0; i < 2; ++i)
        if (Config::SERVO_LIFT_UP_ANGLES[i] < 0 || Config::SERVO_LIFT_UP_ANGLES[i] > 180 ||
            Config::SERVO_LIFT_DOWN_ANGLES[i] < 0 || Config::SERVO_LIFT_DOWN_ANGLES[i] > 180) return false;
    return true;
}
bool ServoModule::setAngle(size_t index, int angle) {
    if (index >= 3 || !attached_[index] || angle < 0 || angle > 180 || !Config::SERVO_MOVE_TIME_MS) return false;
    const uint32_t pulseUs = Config::SERVO_MIN_PULSE_US + (Config::SERVO_MAX_PULSE_US - Config::SERVO_MIN_PULSE_US) * angle / 180;
    const uint32_t duty = static_cast<uint64_t>(pulseUs) * Config::SERVO_PWM_HZ * ((1UL << Config::SERVO_PWM_BITS) - 1) / 1000000;
    ledcWrite(Config::SERVO_PWM_CHANNELS[index], duty);
    angles_[index] = angle;
    movedAtMs_ = millis();
    busy_ = true;
    return true;
}
bool ServoModule::liftUp() {
    return liftConfigured() && setAngle(Config::SERVO_LIFT, Config::SERVO_LIFT_UP_ANGLES[0]) &&
        setAngle(Config::SERVO_OTHER, Config::SERVO_LIFT_UP_ANGLES[1]);
}
bool ServoModule::liftDown() {
    return liftConfigured() && setAngle(Config::SERVO_LIFT, Config::SERVO_LIFT_DOWN_ANGLES[0]) &&
        setAngle(Config::SERVO_OTHER, Config::SERVO_LIFT_DOWN_ANGLES[1]);
}
void ServoModule::update() { if (busy_ && static_cast<uint32_t>(millis() - movedAtMs_) >= Config::SERVO_MOVE_TIME_MS) busy_ = false; }
int ServoModule::getAngle(size_t index) const { return index < 3 ? angles_[index] : -1; }
```

Vai trò: Triển khai API. PWM servo tách timer motor; không đặt góc lúc boot; phối hợp cặp servo nâng/hạ theo lệnh server.

Giao tiếp: AGVController.

Cần chỉnh: SERVO_ENABLED, vai trò, pulse limits, góc nâng/hạ và thời gian ổn định.

SHA-256 source: `1fe0fc1d7b5c663c3e97c483fde3a71740ef493f15c234a32dd129141d9e6e60`.

## 15. LineFollowModule.h

Source: [NCKHAGV/src/motion/LineFollowModule.h](../NCKHAGV/src/motion/LineFollowModule.h).

```cpp
#pragma once
#include "MotorModule.h"

class LineFollowModule {
public:
    explicit LineFollowModule(MotorModule& motor) : motor_(motor) {}
    void begin();
    LineReadings readSensors() const;
    void followLine();
    void correctLeft();
    void correctRight();
    void stop();
private:
    MotorModule& motor_;
};
```

Vai trò: Khai báo API. Đọc ba input-only GPIO, chuẩn hóa cực tính và sửa tốc độ hai phía.

Giao tiếp: MotorModule và AGVController.

Cần chỉnh: LINE_SENSOR_ACTIVE_STATE, BASE_SPEED, CORRECTION_SPEED.

SHA-256 source: `fd647cd6c8ac9da2e6fb40a3cde0ba7973994b2361783b1685bd6048f6977943`.

## 16. LineFollowModule.cpp

Source: [NCKHAGV/src/motion/LineFollowModule.cpp](../NCKHAGV/src/motion/LineFollowModule.cpp).

```cpp
#include "LineFollowModule.h"
#include "core/ControlLogic.h"

void LineFollowModule::begin() {
    pinMode(Config::LINE_LEFT_PIN, INPUT);
    pinMode(Config::LINE_CENTER_PIN, INPUT);
    pinMode(Config::LINE_RIGHT_PIN, INPUT);
}
LineReadings LineFollowModule::readSensors() const {
    LineReadings values;
    values.left = digitalRead(Config::LINE_LEFT_PIN);
    values.center = digitalRead(Config::LINE_CENTER_PIN);
    values.right = digitalRead(Config::LINE_RIGHT_PIN);
    values.configured = Config::LINE_SENSOR_ACTIVE_STATE == 0 || Config::LINE_SENSOR_ACTIVE_STATE == 1;
    values.pattern = AGVMath::normalizeLine(values.left, values.center, values.right, Config::LINE_SENSOR_ACTIVE_STATE);
    return values;
}
void LineFollowModule::followLine() {
    switch (AGVMath::lineAction(readSensors().pattern)) {
        case MotorAction::Forward: motor_.setMotorSpeed(Config::BASE_SPEED); motor_.motorForward(); break;
        case MotorAction::CorrectLeft: correctLeft(); break;
        case MotorAction::CorrectRight: correctRight(); break;
        default: stop(); break;
    }
}
void LineFollowModule::correctLeft() {
    motor_.setLeftSpeed(Config::CORRECTION_SPEED + Config::LEFT_SPEED_TRIM);
    motor_.setRightSpeed(0);
    motor_.motorForward();
}
void LineFollowModule::correctRight() {
    motor_.setLeftSpeed(0);
    motor_.setRightSpeed(Config::CORRECTION_SPEED + Config::RIGHT_SPEED_TRIM);
    motor_.motorForward();
}
void LineFollowModule::stop() { motor_.motorStop(); }
```

Vai trò: Triển khai API. Đọc ba input-only GPIO, chuẩn hóa cực tính và sửa tốc độ hai phía.

Giao tiếp: MotorModule và AGVController.

Cần chỉnh: LINE_SENSOR_ACTIVE_STATE, BASE_SPEED, CORRECTION_SPEED.

SHA-256 source: `bc09595f061e4446cd88c0b26eb885d7e7b7c18ccb3eee28cdba21a74ccea5db`.

## 17. MovementModule.h

Source: [NCKHAGV/src/motion/MovementModule.h](../NCKHAGV/src/motion/MovementModule.h).

```cpp
#pragma once
#include "LineFollowModule.h"
#include "actuators/BuzzerModule.h"

class MovementModule {
public:
    MovementModule(MotorModule& motor, LineFollowModule& line, BuzzerModule& buzzer)
        : motor_(motor), line_(line), buzzer_(buzzer) {}
    void begin() { stop(); }
    void moveForward();
    void moveBackward();
    void turnLeft();
    void turnRight();
    void stop();
    void followLine();
    void apply(MotorAction action);
    MotorAction action() const { return action_; }
private:
    MotorModule& motor_;
    LineFollowModule& line_;
    BuzzerModule& buzzer_;
    MotorAction action_ = MotorAction::Stop;
};
```

Vai trò: Khai báo API. Tầng chuyển động: forward/backward/turn/stop/followLine theo quyết định controller.

Giao tiếp: AGVController, MotorModule, LineFollowModule.

Cần chỉnh: Các speed trong Config.h; không sửa GPIO tại đây.

SHA-256 source: `c848a6ee844e4bd008e8d666b2a8137f32b19bd2af370df1f5be49d4fa5b9594`.

## 18. MovementModule.cpp

Source: [NCKHAGV/src/motion/MovementModule.cpp](../NCKHAGV/src/motion/MovementModule.cpp).

```cpp
#include "MovementModule.h"
#include "core/ControlLogic.h"

void MovementModule::moveForward() { motor_.setMotorSpeed(Config::BASE_SPEED); motor_.motorForward(); buzzer_.off(); action_ = MotorAction::Forward; }
void MovementModule::moveBackward() {
    motor_.setMotorSpeed(Config::BASE_SPEED);
    motor_.motorBackward();
    action_ = MotorAction::Backward;
    buzzer_.setReverse(AGVMath::reverseBuzzerRequired(action_, motor_.leftDuty(), motor_.rightDuty()));
}
void MovementModule::turnLeft() { motor_.setMotorSpeed(Config::TURN_SPEED); motor_.motorLeft(); buzzer_.off(); action_ = MotorAction::PivotLeft; }
void MovementModule::turnRight() { motor_.setMotorSpeed(Config::TURN_SPEED); motor_.motorRight(); buzzer_.off(); action_ = MotorAction::PivotRight; }
void MovementModule::stop() { motor_.motorStop(); buzzer_.off(); action_ = MotorAction::Stop; }
void MovementModule::followLine() { apply(AGVMath::lineAction(line_.readSensors().pattern)); }
void MovementModule::apply(MotorAction action) {
    switch (action) {
        case MotorAction::Stop: stop(); break;
        case MotorAction::Forward: moveForward(); break;
        case MotorAction::Backward: moveBackward(); break;
        case MotorAction::PivotLeft: turnLeft(); break;
        case MotorAction::PivotRight: turnRight(); break;
        case MotorAction::CorrectLeft: line_.correctLeft(); buzzer_.off(); action_ = action; break;
        case MotorAction::CorrectRight: line_.correctRight(); buzzer_.off(); action_ = action; break;
    }
}
```

Vai trò: Triển khai API. Tầng chuyển động: forward/backward/turn/stop/followLine theo quyết định controller.

Giao tiếp: AGVController, MotorModule, LineFollowModule.

Cần chỉnh: Các speed trong Config.h; không sửa GPIO tại đây.

SHA-256 source: `dc7866b44bce951734421862ac2c9d8b523b1e5bbcc1a2bbe225b81c9d50287c`.

## 19. WiFiModule.h

Source: [NCKHAGV/src/network/WiFiModule.h](../NCKHAGV/src/network/WiFiModule.h).

```cpp
#pragma once
#include <Arduino.h>

class WiFiModule {
public:
    void begin();
    void connect();
    void loop();
    bool isConnected() const;
    void reconnect();
private:
    uint32_t lastAttemptMs_ = 0;
};
```

Vai trò: Khai báo API. Kết nối và retry Wi-Fi không có vòng chờ delay dài.

Giao tiếp: Task của MQTTModule; controller chỉ đọc trạng thái.

Cần chỉnh: WIFI_SSID, WIFI_PASSWORD.

SHA-256 source: `17d01bcd5ef26526558607549220d966b3ffa9e957c93b9f29c1ca8abd4ee91e`.

## 20. WiFiModule.cpp

Source: [NCKHAGV/src/network/WiFiModule.cpp](../NCKHAGV/src/network/WiFiModule.cpp).

```cpp
#include "WiFiModule.h"
#include "Config.h"
#include <WiFi.h>

void WiFiModule::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    connect();
}
void WiFiModule::connect() {
    lastAttemptMs_ = millis();
    if (!Config::WIFI_SSID[0]) return;
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);
    if (Config::DEBUG_ENABLED) Serial.println("[WiFi] Connection requested");
}
void WiFiModule::reconnect() { connect(); }
void WiFiModule::loop() {
    if (!isConnected() && static_cast<uint32_t>(millis() - lastAttemptMs_) >= Config::WIFI_RETRY_MS) reconnect();
}
bool WiFiModule::isConnected() const { return WiFi.status() == WL_CONNECTED; }
```

Vai trò: Triển khai API. Kết nối và retry Wi-Fi không có vòng chờ delay dài.

Giao tiếp: Task của MQTTModule; controller chỉ đọc trạng thái.

Cần chỉnh: WIFI_SSID, WIFI_PASSWORD.

SHA-256 source: `d3cc6a1fa10ff5de4d70eff39212484b805cda7667aa03029442fba572d7a37d`.

## 21. MQTTModule.h

Source: [NCKHAGV/src/network/MQTTModule.h](../NCKHAGV/src/network/MQTTModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <atomic>
#include "WiFiModule.h"
#include "Types.h"

class MQTTModule {
public:
    explicit MQTTModule(WiFiModule& wifi) : wifi_(wifi), client_(transport_) {}
    bool begin();
    void connect();
    void loop();
    bool subscribeTopics();
    bool isConnected() const;
    uint32_t session() const { return session_.load(); }
    bool popCommand(AGVCommand& command);
    bool consumeEmergencyStop() { return emergencyStop_.exchange(false); }
    bool consumeStop() { return stop_.exchange(false); }
    bool consumeQueueFault() { return queueFault_.exchange(false); }
    void publishStatus(const TelemetrySnapshot& snapshot);
    void publishPosition(const TelemetrySnapshot& snapshot) { publishStatus(snapshot); }
    void publishDistance(const TelemetrySnapshot& snapshot) { publishStatus(snapshot); }
    bool publishEvent(const char* name, const char* node, const EncoderData& distance, uint32_t uptimeMs,
                      const char* taskId = "", const char* fromNode = "", bool reverse = false);
    bool publishCargo(const char* name, const char* node, const char* taskId, bool present, uint32_t uptimeMs);
    bool publishError(ErrorCode reason, uint32_t uptimeMs);
    bool publishCommandResult(const char* id, bool accepted, ErrorCode reason, uint32_t uptimeMs);
private:
    struct Outbound { bool errorTopic = false; char payload[Config::MAX_PUBLISH_BYTES]{}; };
    static void workerEntry(void* argument);
    void onMessage(char* topic, uint8_t* payload, unsigned int length);
    bool enqueue(const Outbound& message);
    void sendTelemetry(const TelemetrySnapshot& snapshot);
    bool configured() const;
    WiFiModule& wifi_;
    WiFiClient transport_;
    PubSubClient client_;
    QueueHandle_t commands_ = nullptr, events_ = nullptr, telemetry_ = nullptr;
    TaskHandle_t worker_ = nullptr;
    std::atomic<bool> ready_{false}, emergencyStop_{false}, stop_{false}, queueFault_{false};
    std::atomic<uint32_t> servicedAtMs_{0}, session_{0};
    uint32_t lastAttemptMs_ = 0;
    char recentIds_[Config::RECENT_COMMAND_IDS][COMMAND_ID_SIZE]{};
    size_t nextId_ = 0;
    Outbound pending_{};
};
```

Vai trò: Khai báo API. Task mạng riêng, parser JSON, session, hàng đợi lệnh/sự kiện và telemetry; không gọi motor.

Giao tiếp: WiFiModule, CommandCodec, AGVController.

Cần chỉnh: AGV_ID, broker, credentials, sáu topic và timeout.

SHA-256 source: `b0fde45c0668687d6ee291f31df84beb19997365a5bbba73a8f3dd5df9179f49`.

## 22. MQTTModule.cpp

Source: [NCKHAGV/src/network/MQTTModule.cpp](../NCKHAGV/src/network/MQTTModule.cpp).

```cpp
#include "MQTTModule.h"
#include "core/CommandCodec.h"
#include "core/ControlLogic.h"
#include <ArduinoJson.h>
#include <esp_system.h>
#include <cmath>
#include <cstring>

namespace {
void number(JsonObject object, const char* key, float value) {
    if (std::isfinite(value)) object[key] = value;
    else object[key] = nullptr;
}
void base(JsonObject object, uint32_t session, uint32_t uptime) {
    object["version"] = 1;
    object["agvId"] = Config::AGV_ID;
    object["session"] = session;
    object["uptimeMs"] = uptime;
}
bool serialize(const JsonDocument& doc, char* output, size_t capacity) {
    if (doc.overflowed() || measureJson(doc) >= capacity) return false;
    return serializeJson(doc, output, capacity) > 0;
}
}
bool MQTTModule::configured() const {
    const char* required[] = {Config::AGV_ID, Config::MQTT_SERVER, Config::MQTT_TOPIC_COMMAND,
        Config::MQTT_TOPIC_STATUS, Config::MQTT_TOPIC_POSITION, Config::MQTT_TOPIC_DISTANCE,
        Config::MQTT_TOPIC_EVENT, Config::MQTT_TOPIC_ERROR};
    for (const char* value : required) if (!value[0]) return false;
    // Publish topics must be concrete, distinct and fit the MQTT packet buffer.
    for (size_t i = 2; i < 8; ++i) {
        if (std::strlen(required[i]) > 128 || std::strchr(required[i], '#') || std::strchr(required[i], '+')) return false;
        for (size_t j = i + 1; j < 8; ++j) if (std::strcmp(required[i], required[j]) == 0) return false;
    }
    return std::strlen(Config::AGV_ID) < COMMAND_ID_SIZE;
}
bool MQTTModule::begin() {
    commands_ = xQueueCreate(Config::COMMAND_QUEUE_LENGTH, sizeof(AGVCommand));
    events_ = xQueueCreate(Config::EVENT_QUEUE_LENGTH, sizeof(Outbound));
    telemetry_ = xQueueCreate(1, sizeof(TelemetrySnapshot));
    if (!commands_ || !events_ || !telemetry_) { queueFault_ = true; return false; }
    client_.setServer(Config::MQTT_SERVER, Config::MQTT_PORT);
    client_.setSocketTimeout(Config::MQTT_SOCKET_TIMEOUT_SECONDS);
    client_.setKeepAlive(Config::MQTT_KEEPALIVE_SECONDS);
    if (!client_.setBufferSize(Config::MAX_PUBLISH_BYTES + 256)) { queueFault_ = true; return false; }
    transport_.setTimeout(Config::MQTT_SOCKET_TIMEOUT_SECONDS);
    client_.setCallback([this](char* topic, uint8_t* payload, unsigned int length) { onMessage(topic, payload, length); });
    servicedAtMs_ = millis();
    const BaseType_t result = xTaskCreatePinnedToCore(workerEntry, "agv-network", Config::NETWORK_TASK_STACK_BYTES, this, 1, &worker_, 0);
    if (result != pdPASS) queueFault_ = true;
    return result == pdPASS;
}
void MQTTModule::workerEntry(void* argument) {
    auto* self = static_cast<MQTTModule*>(argument);
    self->wifi_.begin();
    for (;;) {
        self->loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
bool MQTTModule::isConnected() const {
    return ready_.load() && wifi_.isConnected() && static_cast<uint32_t>(millis() - servicedAtMs_.load()) < Config::NETWORK_WORKER_STALE_MS;
}
bool MQTTModule::subscribeTopics() { return client_.subscribe(Config::MQTT_TOPIC_COMMAND, 0); }
void MQTTModule::connect() {
    lastAttemptMs_ = millis();
    if (!wifi_.isConnected() || !configured()) return;
    char clientId[COMMAND_ID_SIZE + 16];
    uint32_t nonce = esp_random();
    if (!nonce || nonce == session_.load()) { nonce = session_.load() + 1; if (!nonce) nonce = 1; }
    snprintf(clientId, sizeof(clientId), "%s-%08lx", Config::AGV_ID, static_cast<unsigned long>(nonce));
    StaticJsonDocument<256> will;
    base(will.to<JsonObject>(), nonce, 0);
    will["event"] = "AGV_CONNECTION_LOST";
    char willPayload[256];
    if (!serialize(will, willPayload, sizeof(willPayload))) return;
    const bool connected = client_.connect(clientId, Config::MQTT_USERNAME, Config::MQTT_PASSWORD,
        Config::MQTT_TOPIC_ERROR, 0, false, willPayload);
    if (!connected || !subscribeTopics()) { client_.disconnect(); ready_ = false; return; }
    std::memset(recentIds_, 0, sizeof(recentIds_));
    nextId_ = 0;
    session_ = nonce;
    ready_ = true;
    if (Config::DEBUG_ENABLED) Serial.printf("[MQTT] Connected session=%lu\n", static_cast<unsigned long>(nonce));
}
void MQTTModule::loop() {
    wifi_.loop();
    if (!wifi_.isConnected()) { ready_ = false; transport_.stop(); }
    if (wifi_.isConnected() && !client_.connected() && static_cast<uint32_t>(millis() - lastAttemptMs_) >= Config::MQTT_RETRY_MS) connect();
    if (client_.connected()) {
        if (!client_.loop()) ready_ = false;
        if (ready_.load()) {
            // A failed QoS0 publish retains the front event for the next connection.
            if (xQueuePeek(events_, &pending_, 0) == pdTRUE &&
                client_.publish(pending_.errorTopic ? Config::MQTT_TOPIC_ERROR : Config::MQTT_TOPIC_EVENT, pending_.payload, false)) {
                xQueueReceive(events_, &pending_, 0);
            }
            TelemetrySnapshot snapshot;
            if (xQueueReceive(telemetry_, &snapshot, 0) == pdTRUE) sendTelemetry(snapshot);
        }
    } else ready_ = false;
    servicedAtMs_ = millis();
}
bool MQTTModule::popCommand(AGVCommand& command) { return commands_ && xQueueReceive(commands_, &command, 0) == pdTRUE; }
void MQTTModule::onMessage(char* topic, uint8_t* payload, unsigned int length) {
    if (std::strcmp(topic, Config::MQTT_TOPIC_COMMAND) != 0) return;
    AGVCommand command;
    ErrorCode reason;
    if (!CommandCodec::decode(payload, length, command, reason)) { publishError(reason, millis()); return; }
    const bool safetyCommand = command.type == CommandType::Stop || command.type == CommandType::EmergencyStop;
    // Stop priority is independent of deduplication, age, session and queue capacity.
    if (command.type == CommandType::EmergencyStop) emergencyStop_ = true;
    if (command.type == CommandType::Stop) stop_ = true;
    if (!safetyCommand && command.session != session_.load()) { publishCommandResult(command.id, false, ErrorCode::StaleCommand, millis()); return; }
    for (const auto& id : recentIds_)
        if (std::strcmp(id, command.id) == 0) { publishCommandResult(command.id, false, ErrorCode::InvalidCommand, millis()); return; }
    command.receivedAtMs = millis();
    if (xQueueSend(commands_, &command, 0) != pdTRUE) { queueFault_ = true; return; }
    AGVMath::copyText(recentIds_[nextId_], COMMAND_ID_SIZE, command.id);
    nextId_ = (nextId_ + 1) % Config::RECENT_COMMAND_IDS;
}
bool MQTTModule::enqueue(const Outbound& message) {
    if (!events_ || xQueueSend(events_, &message, 0) != pdTRUE) { queueFault_ = true; return false; }
    return true;
}
void MQTTModule::publishStatus(const TelemetrySnapshot& snapshot) {
    if (!telemetry_ || xQueueOverwrite(telemetry_, &snapshot) != pdTRUE) queueFault_ = true;
}
bool MQTTModule::publishEvent(const char* name, const char* node, const EncoderData& distance, uint32_t uptime,
                              const char* taskId, const char* fromNode, bool reverse) {
    StaticJsonDocument<512> doc;
    auto object = doc.to<JsonObject>();
    base(object, session(), uptime);
    object["event"] = name;
    object["node"] = node;
    object["taskId"] = taskId;
    object["fromNode"] = fromNode;
    object["reverse"] = reverse;
    object["leftPulses"] = distance.leftPulses;
    object["rightPulses"] = distance.rightPulses;
    number(object, "distanceCm", distance.averageDistanceCm);
    number(object, "distanceMm", reverse ? -distance.averageDistanceCm * 10.0f : distance.averageDistanceCm * 10.0f);
    number(object, "leftDistanceCm", distance.leftDistanceCm);
    number(object, "rightDistanceCm", distance.rightDistanceCm);
    Outbound message;
    if (!serialize(doc, message.payload, sizeof(message.payload))) { queueFault_ = true; return false; }
    return enqueue(message);
}
bool MQTTModule::publishCargo(const char* name, const char* node, const char* taskId, bool present, uint32_t uptime) {
    StaticJsonDocument<384> doc;
    auto object = doc.to<JsonObject>();
    base(object, session(), uptime);
    object["event"] = name;
    object["node"] = node;
    object["taskId"] = taskId;
    object["cargoPresent"] = present;
    Outbound message;
    if (!serialize(doc, message.payload, sizeof(message.payload))) { queueFault_ = true; return false; }
    return enqueue(message);
}
bool MQTTModule::publishError(ErrorCode reason, uint32_t uptime) {
    StaticJsonDocument<256> doc;
    auto object = doc.to<JsonObject>();
    base(object, session(), uptime);
    object["error"] = errorName(reason);
    Outbound message;
    message.errorTopic = true;
    if (!serialize(doc, message.payload, sizeof(message.payload))) { queueFault_ = true; return false; }
    return enqueue(message);
}
bool MQTTModule::publishCommandResult(const char* id, bool accepted, ErrorCode reason, uint32_t uptime) {
    StaticJsonDocument<384> doc;
    auto object = doc.to<JsonObject>();
    base(object, session(), uptime);
    object["event"] = "AGV_COMMAND_RESULT";
    object["id"] = id;
    object["accepted"] = accepted;
    object["error"] = errorName(reason);
    Outbound message;
    if (!serialize(doc, message.payload, sizeof(message.payload))) { queueFault_ = true; return false; }
    return enqueue(message);
}
void MQTTModule::sendTelemetry(const TelemetrySnapshot& s) {
    StaticJsonDocument<1536> doc;
    auto object = doc.to<JsonObject>();
    base(object, session(), s.uptimeMs);
    object["state"] = stateName(s.state);
    object["error"] = errorName(s.error);
    object["node"] = s.position.node;
    object["nextNode"] = s.nextNode;
    number(object, "distanceCm", s.position.distanceCm);
    number(object, "distanceMm", s.motor == MotorAction::Backward ? -s.position.distanceCm * 10.0f : s.position.distanceCm * 10.0f);
    object["taskId"] = s.taskId;
    object["taskActive"] = s.taskActive;
    object["pathIndex"] = s.pathIndex;
    object["pathLength"] = s.pathLength;
    object["motor"] = motorActionName(s.motor);
    object["leftDuty"] = s.leftDuty;
    object["rightDuty"] = s.rightDuty;
    object["cargoStable"] = s.cargoStable;
    object["cargoPresent"] = s.cargoStable ? s.cargoPresent : false;
    object["buzzerOn"] = s.buzzerOn;
    object["wifi"] = s.wifi;
    object["mqtt"] = s.mqtt;
    number(object, "ultrasonicCm", s.ultrasonicCm);
    object["batteryAdcMv"] = s.batteryAdcMv;
    object["batteryCalibrated"] = s.batteryCalibrated;
    number(object, "batteryVoltage", s.batteryVoltage);
    number(object, "batteryPercentage", s.batteryPercentage);
    char payload[Config::MAX_PUBLISH_BYTES];
    if (serialize(doc, payload, sizeof(payload))) client_.publish(Config::MQTT_TOPIC_STATUS, payload, false);
    else queueFault_ = true;
    doc.clear();
    object = doc.to<JsonObject>();
    base(object, session(), s.uptimeMs);
    object["node"] = s.position.node;
    object["nextNode"] = s.nextNode;
    number(object, "distanceCm", s.position.distanceCm);
    number(object, "distanceMm", s.motor == MotorAction::Backward ? -s.position.distanceCm * 10.0f : s.position.distanceCm * 10.0f);
    object["pathIndex"] = s.pathIndex;
    object["motor"] = motorActionName(s.motor);
    if (serialize(doc, payload, sizeof(payload))) client_.publish(Config::MQTT_TOPIC_POSITION, payload, false);
    doc.clear();
    object = doc.to<JsonObject>();
    base(object, session(), s.uptimeMs);
    object["node"] = s.position.node;
    object["leftPulses"] = s.encoder.leftPulses;
    object["rightPulses"] = s.encoder.rightPulses;
    number(object, "leftDistanceCm", s.encoder.leftDistanceCm);
    number(object, "rightDistanceCm", s.encoder.rightDistanceCm);
    number(object, "distanceCm", s.encoder.averageDistanceCm);
    number(object, "distanceMm", s.motor == MotorAction::Backward ? -s.encoder.averageDistanceCm * 10.0f : s.encoder.averageDistanceCm * 10.0f);
    if (serialize(doc, payload, sizeof(payload))) client_.publish(Config::MQTT_TOPIC_DISTANCE, payload, false);
}
```

Vai trò: Triển khai API. Task mạng riêng, parser JSON, session, hàng đợi lệnh/sự kiện và telemetry; không gọi motor.

Giao tiếp: WiFiModule, CommandCodec, AGVController.

Cần chỉnh: AGV_ID, broker, credentials, sáu topic và timeout.

SHA-256 source: `a56738f535cd28f6cef332f031ab3941f5305ca54e92f1113604a430ae19dba7`.

## 23. AGVController.h

Source: [NCKHAGV/src/core/AGVController.h](../NCKHAGV/src/core/AGVController.h).

```cpp
#pragma once
#include "sensors/BatteryModule.h"
#include "sensors/EncoderModule.h"
#include "sensors/RFIDModule.h"
#include "sensors/SensorModule.h"
#include "actuators/ServoModule.h"
#include "motion/MovementModule.h"
#include "network/MQTTModule.h"
#include "ControlLogic.h"
#include "sensors/CargoModule.h"
#include "actuators/BuzzerModule.h"

class AGVController {
public:
    AGVController();
    void begin();
    void update();
    AGVState state() const { return control_.state(); }
private:
    ControlInput input() const;
    bool consumePriority();
    void handleCommand(const AGVCommand& command, bool inhibited);
    void publishTelemetry();
    void observeState();
    MotorModule motor_;
    SensorModule sensor_;
    EncoderModule encoder_;
    RFIDModule rfid_;
    ServoModule servo_;
    LineFollowModule line_;
    BuzzerModule buzzer_;
    MovementModule movement_;
    BatteryModule battery_;
    CargoModule cargo_;
    WiFiModule wifi_;
    MQTTModule mqtt_;
    ControlLogic control_;
    uint32_t lastTelemetryMs_ = 0;
    AGVState lastState_ = AGV_IDLE;
    ErrorCode lastError_ = ErrorCode::None;
    bool pickupIrReported_ = false;
};
```

Vai trò: Khai báo API. Khởi tạo, kiểm tra safety, nhận task, phối hợp RFID/encoder, quyết định motor/servo và báo trạng thái.

Giao tiếp: Tất cả module và ControlLogic.

Cần chỉnh: Config.h theo checklist; không điền thông số tại controller.

SHA-256 source: `0bd4a9eb298f84614c0e04acca3b0b0a47bf021a506209e462c6f71ff093624c`.

## 24. AGVController.cpp

Source: [NCKHAGV/src/core/AGVController.cpp](../NCKHAGV/src/core/AGVController.cpp).

```cpp
#include "AGVController.h"
#include <cstring>

AGVController::AGVController() : line_(motor_), movement_(motor_, line_, buzzer_), mqtt_(wifi_),
    control_({Config::WIFI_TIMEOUT_MS, Config::MQTT_TIMEOUT_MS, Config::TURN_MIN_MS,
              Config::TURN_TIMEOUT_MS, Config::STRAIGHT_DEPARTURE_MS, Config::SERVO_MOVE_TIME_MS,
              true, true, Config::REVERSE_STEERING_CONFIRMED}) {}
void AGVController::begin() {
    buzzer_.begin();
    motor_.begin();
    movement_.begin();
    line_.begin();
    sensor_.begin();
    encoder_.begin();
    rfid_.begin();
    servo_.begin();
    battery_.begin();
    cargo_.begin();
    control_.begin(millis());
    if (!mqtt_.begin()) control_.fail(ErrorCode::QueueOverflow);
    if (Config::DEBUG_ENABLED) Serial.println("[AGV] Initialized; motion requires calibrated Config.h and explicit commands");
}
ControlInput AGVController::input() const {
    ControlInput result;
    const auto readings = line_.readSensors();
    result.now = millis();
    result.wifi = wifi_.isConnected();
    result.mqtt = mqtt_.isConnected();
    result.motionConfigured = motor_.configured() && Config::OBSTACLE_DISTANCE_CM > 0 &&
        Config::ENCODER_PPR > 0 && Config::WHEEL_DIAMETER_CM > 0;
    result.liftConfigured = servo_.liftConfigured();
    result.distanceValid = sensor_.valid();
    result.obstacle = sensor_.isObstacleDetected();
    result.servoBusy = servo_.busy();
    result.lineConfigured = readings.configured;
    result.linePattern = readings.pattern;
    result.cargoStable = cargo_.stable();
    result.cargoPresent = cargo_.present();
    return result;
}
bool AGVController::consumePriority() {
    bool inhibited = false;
    if (mqtt_.consumeStop()) { control_.pause(); inhibited = true; }
    if (mqtt_.consumeEmergencyStop()) { control_.safeStop(ErrorCode::EmergencyStop); inhibited = true; }
    if (mqtt_.consumeQueueFault()) { control_.safeStop(ErrorCode::QueueOverflow); inhibited = true; }
    if (inhibited) movement_.stop();
    return inhibited;
}
void AGVController::handleCommand(const AGVCommand& command, bool inhibited) {
    ErrorCode reason = ErrorCode::None;
    bool accepted = false;
    const ControlInput in = input();
    if (command.type == CommandType::EmergencyStop) {
        control_.safeStop(ErrorCode::EmergencyStop);
        movement_.stop();
        accepted = true;
    } else if (command.type == CommandType::Stop) {
        control_.pause();
        movement_.stop();
        accepted = true;
    } else if (inhibited || command.session != mqtt_.session() ||
               static_cast<uint32_t>(in.now - command.receivedAtMs) > Config::COMMAND_MAX_AGE_MS) {
        reason = ErrorCode::StaleCommand;
    } else {
        switch (command.type) {
            case CommandType::Task: {
                bool mapped = true;
                for (size_t i = 0; i < command.task.pathLength; ++i)
                    if (!rfid_.hasMapping(command.task.path[i].node)) mapped = false;
                if (!mapped) reason = ErrorCode::Configuration;
                else accepted = control_.setTask(command.task, reason);
                break;
            }
            case CommandType::Start: accepted = control_.start(in, reason); break;
            case CommandType::NodeOrder: accepted = control_.setNodeOrder(command, in.now, reason); break;
            case CommandType::Restart: accepted = control_.restart(in, reason); break;
            case CommandType::CancelTask:
                movement_.stop();
                control_.cancelTask();
                accepted = true;
                break;
            case CommandType::LiftUp: case CommandType::LiftDown:
                if (movement_.action() != MotorAction::Stop) reason = ErrorCode::TaskBusy;
                else accepted = control_.authorizeLift(command, in, reason);
                break;
            default: reason = ErrorCode::InvalidCommand; break;
        }
    }
    if (Config::DEBUG_ENABLED) Serial.printf("[CMD] id=%s accepted=%d error=%s\n", command.id, accepted, errorName(reason));
    mqtt_.publishCommandResult(command.id, accepted, reason, millis());
}
void AGVController::update() {
    cargo_.update(millis());
    sensor_.update();
    encoder_.update();
    servo_.update();
    bool inhibited = consumePriority();
    control_.checkSafety(input());
    if (control_.state() != AGV_MOVING) movement_.stop();
    if (rfid_.readCard()) {
        const RFIDEvent& event = rfid_.event();
        if (!event.node[0]) {
            control_.reachedNode("", millis());
            if (control_.state() != AGV_MOVING) movement_.stop();
        }
        else if (event.newNode) {
            char fromNode[NODE_TEXT_SIZE]{};
            AGVMath::copyText(fromNode, sizeof(fromNode), control_.currentNode());
            const bool reverse = control_.segmentReverse();
            control_.reachedNode(event.node, millis());
            if (control_.state() != AGV_MOVING) movement_.stop();
            // Capture and reset BOTH counters atomically; the event retains the completed segment.
            const EncoderData segment = encoder_.snapshotAndReset();
            mqtt_.publishEvent("AGV_REACHED_NODE", event.node, segment, millis(), control_.task().id, fromNode, reverse);
            publishTelemetry();
        }
    }
    sensor_.update();
    control_.checkSafety(input());
    if (control_.state() != AGV_MOVING) movement_.stop();
    if (control_.state() == AGV_LOADING && cargo_.stable() && !pickupIrReported_) {
        mqtt_.publishCargo("AGV_PICKUP_IR", control_.currentNode(), control_.task().id, cargo_.present(), millis());
        pickupIrReported_ = true;
    }
    if (control_.state() != AGV_LOADING) pickupIrReported_ = false;
    inhibited = consumePriority() || inhibited;
    AGVCommand command;
    for (size_t i = 0; i < Config::COMMAND_QUEUE_LENGTH && mqtt_.popCommand(command); ++i) {
        inhibited = consumePriority() || inhibited;
        handleCommand(command, inhibited);
        if (command.type == CommandType::Stop || command.type == CommandType::EmergencyStop) inhibited = true;
    }
    inhibited = consumePriority() || inhibited;
    if (inhibited) control_.pause();
    sensor_.update();
    const ControlOutput output = control_.update(input());
    movement_.apply(output.motor);
    if (output.servo != ServoAction::None) {
        movement_.stop();
        const bool moved = output.servo == ServoAction::LiftUp ? servo_.liftUp() : servo_.liftDown();
        if (!moved) control_.fail(ErrorCode::ServoConfiguration);
    }
    if (control_.state() != AGV_MOVING) movement_.stop();
    battery_.update();
    observeState();
    if (static_cast<uint32_t>(millis() - lastTelemetryMs_) >= Config::TELEMETRY_INTERVAL_MS) publishTelemetry();
}
void AGVController::observeState() {
    if (lastState_ == control_.state() && lastError_ == control_.error()) return;
    if (Config::DEBUG_ENABLED) Serial.printf("[STATE] %s -> %s error=%s node=%s\n",
        stateName(lastState_), stateName(control_.state()), errorName(control_.error()), control_.currentNode());
    if (control_.error() != ErrorCode::None && control_.error() != lastError_) mqtt_.publishError(control_.error(), millis());
    if (control_.state() == AGV_COMPLETED) mqtt_.publishEvent("AGV_TASK_COMPLETED", control_.currentNode(), encoder_.snapshot(), millis(), control_.task().id);
    lastState_ = control_.state();
    lastError_ = control_.error();
    publishTelemetry();
}
void AGVController::publishTelemetry() {
    TelemetrySnapshot snapshot;
    snapshot.state = control_.state();
    snapshot.error = control_.error();
    snapshot.encoder = encoder_.snapshot();
    AGVMath::copyText(snapshot.position.node, sizeof(snapshot.position.node), control_.currentNode());
    if (control_.task().active && control_.pathIndex() + 1 < control_.task().pathLength)
        AGVMath::copyText(snapshot.nextNode, sizeof(snapshot.nextNode),
                          control_.task().path[control_.pathIndex() + 1].node);
    snapshot.position.distanceCm = snapshot.encoder.averageDistanceCm;
    AGVMath::copyText(snapshot.taskId, sizeof(snapshot.taskId), control_.task().id);
    snapshot.taskActive = control_.task().active;
    snapshot.pathIndex = control_.pathIndex();
    snapshot.pathLength = control_.task().pathLength;
    snapshot.motor = movement_.action();
    snapshot.leftDuty = motor_.leftDuty();
    snapshot.rightDuty = motor_.rightDuty();
    snapshot.wifi = wifi_.isConnected();
    snapshot.mqtt = mqtt_.isConnected();
    snapshot.ultrasonicCm = sensor_.readDistanceCm();
    snapshot.batteryAdcMv = battery_.adcMillivolts();
    snapshot.batteryCalibrated = battery_.calibrated();
    snapshot.batteryVoltage = battery_.readVoltage();
    snapshot.batteryPercentage = battery_.getBatteryPercentage();
    snapshot.cargoStable = cargo_.stable();
    snapshot.cargoPresent = cargo_.present();
    snapshot.buzzerOn = AGVMath::reverseBuzzerRequired(movement_.action(), motor_.leftDuty(), motor_.rightDuty());
    snapshot.uptimeMs = millis();
    mqtt_.publishStatus(snapshot);
    lastTelemetryMs_ = snapshot.uptimeMs;
}
```

Vai trò: Triển khai API. Khởi tạo, kiểm tra safety, nhận task, phối hợp RFID/encoder, quyết định motor/servo và báo trạng thái.

Giao tiếp: Tất cả module và ControlLogic.

Cần chỉnh: Config.h theo checklist; không điền thông số tại controller.

SHA-256 source: `307ced6d274f97a41e053ce84aa9af70ae5764b1e5390360897ce52955896905`.

## 25. main.cpp

Source: [NCKHAGV/src/main.cpp](../NCKHAGV/src/main.cpp).

```cpp
#include <Arduino.h>
#include "core/AGVController.h"

namespace { AGVController agv; }

void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    agv.begin();
}
void loop() {
    agv.update();
    yield();
}
```

Vai trò: Điểm vào ngắn: Serial, begin và update controller.

Giao tiếp: AGVController.

Cần chỉnh: SERIAL_BAUD trong Config.h.

SHA-256 source: `14e0190cb0a6ec697c4ae529401ff481dde38f94d1db91703166153a2acc207d`.

## 26. BatteryModule.h

Source: [NCKHAGV/src/sensors/BatteryModule.h](../NCKHAGV/src/sensors/BatteryModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include "Types.h"

class BatteryModule {
public:
    void begin();
    void update();
    float readVoltage() const { return voltage_; }
    float getBatteryPercentage() const { return percentage_; }
    uint32_t adcMillivolts() const { return adcMv_; }
    bool calibrated() const;
private:
    uint32_t adcMv_ = 0, lastSampleMs_ = 0;
    float voltage_ = UNKNOWN_VALUE, percentage_ = UNKNOWN_VALUE;
};
```

Vai trò: Đọc ADC1 millivolt; chưa hiệu chuẩn thì điện áp/phần trăm là NaN/null.

Giao tiếp: AGVController, AGVMath.

Cần chỉnh: BATTERY_VOLTAGE_SCALE/OFFSET và EMPTY/FULL_VOLTAGE.

SHA-256 source: `7e1eafa99ac654014981f281e923f2db78f5936dcc34b5b5d1f5771db0f014f7`.

## 27. BatteryModule.cpp

Source: [NCKHAGV/src/sensors/BatteryModule.cpp](../NCKHAGV/src/sensors/BatteryModule.cpp).

```cpp
#include "BatteryModule.h"
#include "core/ControlLogic.h"
#include <cmath>

void BatteryModule::begin() {
    pinMode(Config::BATTERY_ADC_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(Config::BATTERY_ADC_PIN, ADC_11db);
    lastSampleMs_ = millis() - Config::BATTERY_SAMPLE_INTERVAL_MS;
}
void BatteryModule::update() {
    if (static_cast<uint32_t>(millis() - lastSampleMs_) < Config::BATTERY_SAMPLE_INTERVAL_MS) return;
    lastSampleMs_ = millis();
    adcMv_ = analogReadMilliVolts(Config::BATTERY_ADC_PIN);
    voltage_ = AGVMath::batteryVoltage(adcMv_, Config::BATTERY_VOLTAGE_SCALE, Config::BATTERY_VOLTAGE_OFFSET);
    percentage_ = AGVMath::batteryPercentage(voltage_, Config::BATTERY_EMPTY_VOLTAGE, Config::BATTERY_FULL_VOLTAGE);
}
bool BatteryModule::calibrated() const { return std::isfinite(voltage_); }
```

Vai trò: Đọc ADC1 millivolt; chưa hiệu chuẩn thì điện áp/phần trăm là NaN/null.

Giao tiếp: AGVController, AGVMath.

Cần chỉnh: BATTERY_VOLTAGE_SCALE/OFFSET và EMPTY/FULL_VOLTAGE.

SHA-256 source: `691c29ba554f2503f375271238f51851faf299fe27bbced781ddc9ca105ad6aa`.

## 28. ControlLogic.h

Source: [NCKHAGV/src/core/ControlLogic.h](../NCKHAGV/src/core/ControlLogic.h).

```cpp
#pragma once

#include "Types.h"

namespace AGVMath {
float distanceCm(uint64_t pulses, uint32_t ppr, float diameterCm);
float batteryVoltage(uint32_t adcMv, float scale, float offset);
float batteryPercentage(float voltage, float emptyVoltage, float fullVoltage);
uint8_t normalizeLine(int left, int center, int right, int activeState);
MotorAction lineAction(uint8_t pattern);
bool validNode(const char* node);
bool validTask(const AGVTask& task);
bool copyText(char* destination, size_t capacity, const char* source);
bool reverseBuzzerRequired(MotorAction action, int leftDuty, int rightDuty);
}

class ControlLogic {
public:
    explicit ControlLogic(SafetySettings settings);
    void begin(uint32_t now);
    void checkSafety(const ControlInput& input);
    ControlOutput update(const ControlInput& input);
    bool setTask(const AGVTask& task, ErrorCode& reason);
    bool setNodeOrder(const AGVCommand& order, uint32_t now, ErrorCode& reason);
    bool authorizeLift(const AGVCommand& command, const ControlInput& input, ErrorCode& reason);
    bool start(const ControlInput& input, ErrorCode& reason);
    bool restart(const ControlInput& input, ErrorCode& reason);
    void pause();
    void cancelTask();
    void reachedNode(const char* node, uint32_t now);
    void safeStop(ErrorCode reason);
    void fail(ErrorCode reason);
    AGVState state() const { return state_; }
    ErrorCode error() const { return error_; }
    const AGVTask& task() const { return task_; }
    const char* currentNode() const { return currentNode_; }
    uint8_t pathIndex() const { return pathIndex_; }
    bool pickupDone() const { return pickupDone_; }
    bool waitingForOrder() const { return waitingForOrder_; }
    bool segmentReverse() const { return activeDeparture_ == Departure::Backward && state_ == AGV_MOVING; }
private:
    void beginSegment(uint32_t now);
    void enterServoState(AGVState state, uint32_t now);
    bool motionClear(const ControlInput& input, ErrorCode& reason) const;
    SafetySettings settings_;
    AGVState state_ = AGV_IDLE;
    ErrorCode error_ = ErrorCode::None;
    AGVTask task_{};
    char currentNode_[NODE_TEXT_SIZE]{};
    uint8_t pathIndex_ = 0;
    bool pickupDone_ = false, servoIssued_ = false;
    bool liftAuthorized_ = false, waitingForOrder_ = false, reverseHold_ = false;
    bool servoInterrupted_ = false;
    AGVCommand pendingOrder_{};
    bool hasOrder_ = false;
    Departure activeDeparture_ = Departure::FollowLine;
    uint32_t reverseStoppedAtMs_ = 0;
    bool turning_ = false, turnInterrupted_ = false;
    uint32_t phaseStartedMs_ = 0, wifiLastGoodMs_ = 0, mqttLastGoodMs_ = 0;
};
```

Vai trò: FSM và toán học production độc lập GPIO, được kiểm thử trực tiếp bằng Unity.

Giao tiếp: AGVController, Types, các module chuyển đổi.

Cần chỉnh: SafetySettings lấy từ Config.h; không tự tính A-Star.

SHA-256 source: `fc8cae291d5151d004819beed9e9f3aad7be2a3c7e7eafff823a6299a01f9f13`.

## 29. ControlLogic.cpp

Source: [NCKHAGV/src/core/ControlLogic.cpp](../NCKHAGV/src/core/ControlLogic.cpp).

```cpp
#include "ControlLogic.h"
#include <cmath>
#include <cstring>

const char* stateName(AGVState s) {
    switch (s) {
        case AGV_IDLE: return "AGV_IDLE";
        case AGV_MOVING: return "AGV_MOVING";
        case AGV_LOADING: return "AGV_LOADING";
        case AGV_UNLOADING: return "AGV_UNLOADING";
        case AGV_WAITING: return "AGV_WAITING";
        case AGV_COMPLETED: return "AGV_COMPLETED";
        case AGV_SAFE_STOP: return "AGV_SAFE_STOP";
        case AGV_ERROR: return "AGV_ERROR";
    }
    return "AGV_ERROR";
}
const char* errorName(ErrorCode e) {
    switch (e) {
        case ErrorCode::None: return "NONE";
        case ErrorCode::Configuration: return "CONFIGURATION_INCOMPLETE";
        case ErrorCode::WifiTimeout: return "WIFI_TIMEOUT";
        case ErrorCode::MqttTimeout: return "MQTT_TIMEOUT";
        case ErrorCode::EmergencyStop: return "EMERGENCY_STOP";
        case ErrorCode::Obstacle: return "OBSTACLE";
        case ErrorCode::SensorInvalid: return "ULTRASONIC_INVALID";
        case ErrorCode::LineLost: return "LINE_LOST";
        case ErrorCode::AmbiguousLine: return "AMBIGUOUS_LINE";
        case ErrorCode::UnknownTag: return "RFID_UNMAPPED_UID";
        case ErrorCode::UnexpectedNode: return "UNEXPECTED_NODE";
        case ErrorCode::TurnTimeout: return "TURN_TIMEOUT";
        case ErrorCode::TurnConfiguration: return "TURN_NOT_CONFIGURED";
        case ErrorCode::TurnInterrupted: return "TURN_INTERRUPTED_CANCEL_TASK";
        case ErrorCode::ServoConfiguration: return "SERVO_NOT_CONFIGURED";
        case ErrorCode::InvalidTask: return "INVALID_TASK";
        case ErrorCode::TaskBusy: return "TASK_BUSY";
        case ErrorCode::NoTask: return "NO_TASK";
        case ErrorCode::NotConnected: return "NETWORK_UNAVAILABLE";
        case ErrorCode::NotAtRouteStart: return "NOT_AT_ROUTE_NODE";
        case ErrorCode::RestartDenied: return "RESTART_REQUIRED_OR_DENIED";
        case ErrorCode::QueueOverflow: return "QUEUE_OVERFLOW";
        case ErrorCode::InvalidCommand: return "INVALID_COMMAND";
        case ErrorCode::StaleCommand: return "STALE_COMMAND";
        case ErrorCode::CargoMissing: return "CARGO_MISSING";
        case ErrorCode::CargoLost: return "CARGO_LOST";
        case ErrorCode::NodeOrderMismatch: return "NODE_ORDER_MISMATCH";
        case ErrorCode::ServoInterrupted: return "SERVO_INTERRUPTED_CANCEL_TASK";
    }
    return "UNKNOWN_ERROR";
}
const char* motorActionName(MotorAction action) {
    switch (action) {
        case MotorAction::Stop: return "STOP";
        case MotorAction::Forward: return "FORWARD";
        case MotorAction::Backward: return "BACKWARD";
        case MotorAction::CorrectLeft: return "CORRECT_LEFT";
        case MotorAction::CorrectRight: return "CORRECT_RIGHT";
        case MotorAction::PivotLeft: return "LEFT";
        case MotorAction::PivotRight: return "RIGHT";
    }
    return "STOP";
}

namespace AGVMath {
bool copyText(char* dst, size_t capacity, const char* src) {
    if (!src || capacity == 0 || std::strlen(src) >= capacity) return false;
    std::memcpy(dst, src, std::strlen(src) + 1);
    return true;
}
bool reverseBuzzerRequired(MotorAction action, int leftDuty, int rightDuty) {
    return action == MotorAction::Backward && leftDuty < 0 && rightDuty < 0;
}
float distanceCm(uint64_t pulses, uint32_t ppr, float diameter) {
    if (!ppr || !std::isfinite(diameter) || diameter <= 0) return UNKNOWN_VALUE;
    return static_cast<float>(static_cast<double>(pulses) / ppr * 3.14159265358979323846 * diameter);
}
float batteryVoltage(uint32_t adcMv, float scale, float offset) {
    if (!std::isfinite(scale) || scale <= 0 || !std::isfinite(offset)) return UNKNOWN_VALUE;
    const float value = adcMv / 1000.0f * scale + offset;
    return value >= 0 && std::isfinite(value) ? value : UNKNOWN_VALUE;
}
float batteryPercentage(float voltage, float empty, float full) {
    if (!std::isfinite(voltage) || !std::isfinite(empty) || !std::isfinite(full) || empty <= 0 || full <= empty) return UNKNOWN_VALUE;
    const float value = (voltage - empty) * 100.0f / (full - empty);
    return value < 0 ? 0 : (value > 100 ? 100 : value);
}
uint8_t normalizeLine(int l, int c, int r, int active) {
    if (active != 0 && active != 1) return 0;
    return static_cast<uint8_t>((l == active ? 4 : 0) | (c == active ? 2 : 0) | (r == active ? 1 : 0));
}
MotorAction lineAction(uint8_t p) {
    switch (p) {
        case 2: case 5: case 7: return MotorAction::Forward;
        case 4: case 6: return MotorAction::CorrectLeft;
        case 1: case 3: return MotorAction::CorrectRight;
        default: return MotorAction::Stop;
    }
}
bool validNode(const char* node) {
    if (!node) return false;
    for (const auto& entry : Config::RFID_NODES)
        if (std::strcmp(node, entry.node) == 0) return true;
    return false;
}
bool validTask(const AGVTask& task) {
    if (!task.id[0] || !validNode(task.startNode) || !validNode(task.targetNode) ||
        task.pathLength == 0 || task.pathLength > Config::MAX_PATH_NODES) return false;
    bool pickupFound = false;
    for (size_t i = 0; i < task.pathLength; ++i) {
        if (!validNode(task.path[i].node)) return false;
        if (i && std::strcmp(task.path[i - 1].node, task.path[i].node) == 0) return false;
        if (std::strcmp(task.path[i].node, task.startNode) == 0) pickupFound = true;
        if (task.path[i].departure != Departure::FollowLine && task.path[i].departure != Departure::Left &&
            task.path[i].departure != Departure::Right && task.path[i].departure != Departure::Straight &&
            task.path[i].departure != Departure::Backward) return false;
    }
    return pickupFound && std::strcmp(task.path[task.pathLength - 1].node, task.targetNode) == 0;
}
}

ControlLogic::ControlLogic(SafetySettings settings) : settings_(settings) {}
void ControlLogic::begin(uint32_t now) {
    wifiLastGoodMs_ = mqttLastGoodMs_ = phaseStartedMs_ = now;
    state_ = AGV_IDLE;
}
void ControlLogic::safeStop(ErrorCode reason) {
    if (turning_) turnInterrupted_ = true;
    if ((state_ == AGV_LOADING || state_ == AGV_UNLOADING) && servoIssued_) servoInterrupted_ = true;
    turning_ = false;
    waitingForOrder_ = hasOrder_ = reverseHold_ = false;
    state_ = AGV_SAFE_STOP;
    error_ = reason;
}
void ControlLogic::fail(ErrorCode reason) {
    safeStop(reason);
    state_ = AGV_ERROR;
}
void ControlLogic::pause() {
    if (state_ == AGV_SAFE_STOP || state_ == AGV_ERROR) return;
    if ((state_ == AGV_LOADING || state_ == AGV_UNLOADING) && servoIssued_) {
        safeStop(ErrorCode::ServoInterrupted);
        return;
    }
    if (turning_) turnInterrupted_ = true;
    turning_ = false;
    waitingForOrder_ = hasOrder_ = reverseHold_ = false;
    state_ = task_.active ? AGV_WAITING : AGV_IDLE;
}
void ControlLogic::cancelTask() {
    task_ = AGVTask{};
    pickupDone_ = servoIssued_ = turning_ = turnInterrupted_ = liftAuthorized_ = waitingForOrder_ = hasOrder_ = reverseHold_ = servoInterrupted_ = false;
    pathIndex_ = 0;
    if (state_ != AGV_SAFE_STOP && state_ != AGV_ERROR) state_ = AGV_IDLE;
}
bool ControlLogic::setTask(const AGVTask& task, ErrorCode& reason) {
    if (state_ == AGV_SAFE_STOP || state_ == AGV_ERROR) { reason = ErrorCode::RestartDenied; return false; }
    if (state_ == AGV_MOVING || state_ == AGV_LOADING || state_ == AGV_UNLOADING || task_.active) { reason = ErrorCode::TaskBusy; return false; }
    if (!AGVMath::validTask(task)) { reason = ErrorCode::InvalidTask; return false; }
    task_ = task;
    task_.active = true;
    pickupDone_ = servoIssued_ = turning_ = turnInterrupted_ = liftAuthorized_ = waitingForOrder_ = hasOrder_ = reverseHold_ = servoInterrupted_ = false;
    pathIndex_ = 0;
    state_ = AGV_WAITING;
    reason = ErrorCode::None;
    return true;
}
bool ControlLogic::motionClear(const ControlInput& in, ErrorCode& reason) const {
    if (!in.wifi || !in.mqtt) reason = ErrorCode::NotConnected;
    else if (!in.motionConfigured || !in.lineConfigured) reason = ErrorCode::Configuration;
    else if (!in.distanceValid) reason = ErrorCode::SensorInvalid;
    else if (in.obstacle) reason = ErrorCode::Obstacle;
    else reason = ErrorCode::None;
    return reason == ErrorCode::None;
}
void ControlLogic::enterServoState(AGVState state, uint32_t now) {
    state_ = state;
    servoIssued_ = false;
    liftAuthorized_ = !settings_.requireLiftCommands;
    waitingForOrder_ = false;
    phaseStartedMs_ = now;
    turning_ = false;
}
void ControlLogic::beginSegment(uint32_t now) {
    phaseStartedMs_ = now;
    if (!pickupDone_ && std::strcmp(currentNode_, task_.startNode) == 0) {
        enterServoState(AGV_LOADING, now);
    } else if (pathIndex_ + 1 >= task_.pathLength) {
        if (pickupDone_) enterServoState(AGV_UNLOADING, now);
        else fail(ErrorCode::InvalidTask);
    } else {
        if (reverseHold_ && static_cast<uint32_t>(now - reverseStoppedAtMs_) < Config::REVERSE_STOP_MS) {
            state_ = AGV_WAITING;
            return;
        }
        reverseHold_ = false;
        Departure departure = task_.path[pathIndex_].departure;
        if (settings_.requireNodeOrders) {
            if (!hasOrder_ || pendingOrder_.pathIndex != pathIndex_) {
                state_ = AGV_WAITING;
                waitingForOrder_ = true;
                return;
            }
            departure = pendingOrder_.departure;
            hasOrder_ = false;
        }
        waitingForOrder_ = false;
        activeDeparture_ = departure;
        if (departure == Departure::Backward && !settings_.reverseSteeringConfirmed) {
            safeStop(ErrorCode::Configuration);
            return;
        }
        if ((departure == Departure::Left || departure == Departure::Right) &&
            (!settings_.turnMinMs || settings_.turnTimeoutMs <= settings_.turnMinMs)) {
            safeStop(ErrorCode::TurnConfiguration);
            return;
        }
        if (departure == Departure::Straight && !settings_.straightDepartureMs) {
            safeStop(ErrorCode::TurnConfiguration);
            return;
        }
        state_ = AGV_MOVING;
        turning_ = departure == Departure::Left || departure == Departure::Right;
    }
}
bool ControlLogic::setNodeOrder(const AGVCommand& order, uint32_t now, ErrorCode& reason) {
    reason = ErrorCode::NodeOrderMismatch;
    if (!settings_.requireNodeOrders || !task_.active || state_ == AGV_SAFE_STOP || state_ == AGV_ERROR) return false;
    if (std::strcmp(order.taskId, task_.id) != 0 || order.pathIndex >= task_.pathLength - 1 ||
        order.pathIndex < pathIndex_ || order.pathIndex > pathIndex_ + 1 ||
        std::strcmp(order.node, task_.path[order.pathIndex].node) != 0 ||
        std::strcmp(order.nextNode, task_.path[order.pathIndex + 1].node) != 0) return false;
    if (state_ == AGV_MOVING && order.pathIndex == pathIndex_) return false;
    if (order.pathIndex == pathIndex_ + 1 && state_ != AGV_MOVING) return false;
    if (order.departure == Departure::Backward) {
        if (order.pathIndex == 0 || order.node[0] != 'K' || order.nextNode[0] != 'G' ||
            std::strcmp(order.nextNode, task_.path[order.pathIndex - 1].node) != 0) return false;
    }
    if (hasOrder_) { reason = ErrorCode::TaskBusy; return false; }
    pendingOrder_ = order;
    hasOrder_ = true;
    reason = ErrorCode::None;
    if (waitingForOrder_ && order.pathIndex == pathIndex_) beginSegment(now);
    return true;
}
bool ControlLogic::authorizeLift(const AGVCommand& command, const ControlInput& in, ErrorCode& reason) {
    reason = ErrorCode::NodeOrderMismatch;
    if (!settings_.requireLiftCommands || !task_.active ||
        (state_ != AGV_LOADING && state_ != AGV_UNLOADING) || liftAuthorized_ || servoIssued_ ||
        std::strcmp(command.taskId, task_.id) != 0 ||
        std::strcmp(command.node, currentNode_) != 0 ||
        command.pathIndex != pathIndex_ ||
        (state_ == AGV_LOADING && command.type != CommandType::LiftUp) ||
        (state_ == AGV_UNLOADING && command.type != CommandType::LiftDown)) return false;
    if (!in.cargoStable || !in.cargoPresent) { reason = ErrorCode::CargoMissing; return false; }
    if (in.servoBusy || !in.liftConfigured) { reason = ErrorCode::ServoConfiguration; return false; }
    if (!motionClear(in, reason)) return false;
    liftAuthorized_ = true;
    reason = ErrorCode::None;
    return true;
}
bool ControlLogic::start(const ControlInput& in, ErrorCode& reason) {
    if (state_ == AGV_SAFE_STOP || state_ == AGV_ERROR) { reason = ErrorCode::RestartDenied; return false; }
    if (state_ == AGV_MOVING || state_ == AGV_LOADING || state_ == AGV_UNLOADING) { reason = ErrorCode::TaskBusy; return false; }
    if (!task_.active) { reason = ErrorCode::NoTask; return false; }
    if (turnInterrupted_) { reason = ErrorCode::TurnInterrupted; return false; }
    if (servoInterrupted_) { reason = ErrorCode::ServoInterrupted; return false; }
    if (in.servoBusy) { reason = ErrorCode::TaskBusy; return false; }
    if (!motionClear(in, reason)) return false;
    if (!in.liftConfigured || !settings_.servoMoveMs) { reason = ErrorCode::ServoConfiguration; return false; }
    if (std::strcmp(currentNode_, task_.path[pathIndex_].node) != 0) { reason = ErrorCode::NotAtRouteStart; return false; }
    beginSegment(in.now);
    reason = error_;
    return state_ != AGV_SAFE_STOP && state_ != AGV_ERROR;
}
bool ControlLogic::restart(const ControlInput& in, ErrorCode& reason) {
    if (state_ != AGV_SAFE_STOP && state_ != AGV_ERROR) { reason = ErrorCode::RestartDenied; return false; }
    if (turnInterrupted_) { reason = ErrorCode::TurnInterrupted; return false; }
    if (servoInterrupted_) { reason = ErrorCode::ServoInterrupted; return false; }
    if (!motionClear(in, reason)) return false;
    if (AGVMath::lineAction(in.linePattern) == MotorAction::Stop) {
        reason = in.linePattern == 0 ? ErrorCode::LineLost : ErrorCode::AmbiguousLine;
        return false;
    }
    state_ = task_.active ? AGV_WAITING : AGV_IDLE;
    error_ = reason = ErrorCode::None;
    return true;
}
void ControlLogic::reachedNode(const char* node, uint32_t now) {
    if (!AGVMath::validNode(node)) {
        if (state_ == AGV_MOVING) {
            currentNode_[0] = '\0';
            safeStop(ErrorCode::UnknownTag);
        }
        return;
    }
    const bool changed = std::strcmp(currentNode_, node) != 0;
    AGVMath::copyText(currentNode_, sizeof(currentNode_), node);
    if (!changed) return;
    if (state_ == AGV_MOVING) {
        if (turning_ || pathIndex_ + 1 >= task_.pathLength || std::strcmp(node, task_.path[pathIndex_ + 1].node) != 0) {
            safeStop(ErrorCode::UnexpectedNode);
            return;
        }
        ++pathIndex_;
        if (activeDeparture_ == Departure::Backward) {
            reverseHold_ = true;
            reverseStoppedAtMs_ = now;
        }
        beginSegment(now);
    } else if (state_ == AGV_LOADING || state_ == AGV_UNLOADING) {
        safeStop(ErrorCode::UnexpectedNode);
    }
}
void ControlLogic::checkSafety(const ControlInput& in) {
    if (in.wifi) wifiLastGoodMs_ = in.now;
    if (in.mqtt) mqttLastGoodMs_ = in.now;
    if (state_ != AGV_SAFE_STOP && state_ != AGV_ERROR) {
        if (!in.wifi && static_cast<uint32_t>(in.now - wifiLastGoodMs_) >= settings_.wifiTimeoutMs) safeStop(ErrorCode::WifiTimeout);
        else if (!in.mqtt && static_cast<uint32_t>(in.now - mqttLastGoodMs_) >= settings_.mqttTimeoutMs) safeStop(ErrorCode::MqttTimeout);
    }
    if (state_ == AGV_SAFE_STOP || state_ == AGV_ERROR) return;
    if (!in.wifi || !in.mqtt) { pause(); return; }
    if (state_ == AGV_MOVING || state_ == AGV_LOADING || state_ == AGV_UNLOADING) {
        ErrorCode reason;
        if (!motionClear(in, reason)) safeStop(reason);
    }
    if (state_ == AGV_MOVING && pickupDone_ && in.cargoStable && !in.cargoPresent)
        safeStop(ErrorCode::CargoLost);
}
ControlOutput ControlLogic::update(const ControlInput& in) {
    ControlOutput out;
    checkSafety(in);
    if (state_ == AGV_SAFE_STOP || state_ == AGV_ERROR || !in.wifi || !in.mqtt) return out;
    if (state_ == AGV_COMPLETED) { state_ = AGV_IDLE; return out; }
    if (state_ == AGV_WAITING && reverseHold_) beginSegment(in.now);
    if (state_ != AGV_MOVING && state_ != AGV_LOADING && state_ != AGV_UNLOADING) return out;
    ErrorCode reason;
    if (!motionClear(in, reason)) { safeStop(reason); return out; }
    if (state_ == AGV_LOADING || state_ == AGV_UNLOADING) {
        if (!in.liftConfigured || !settings_.servoMoveMs) { fail(ErrorCode::ServoConfiguration); return out; }
        if (!liftAuthorized_) return out;
        if (!servoIssued_) {
            out.servo = state_ == AGV_LOADING ? ServoAction::LiftUp : ServoAction::LiftDown;
            servoIssued_ = true;
            phaseStartedMs_ = in.now;
        } else if (!in.servoBusy && static_cast<uint32_t>(in.now - phaseStartedMs_) >= settings_.servoMoveMs) {
            if (settings_.requireLiftCommands && !in.cargoStable) return out;
            if (state_ == AGV_LOADING) {
                if (settings_.requireLiftCommands && !in.cargoPresent) { safeStop(ErrorCode::CargoMissing); return out; }
                pickupDone_ = true; beginSegment(in.now);
            } else if (!settings_.requireLiftCommands || !in.cargoPresent) { task_.active = false; state_ = AGV_COMPLETED; }
        }
        return out;
    }
    if (in.linePattern == 0) { safeStop(ErrorCode::LineLost); return out; }
    if (turning_) {
        const uint32_t elapsed = in.now - phaseStartedMs_;
        if (elapsed >= settings_.turnTimeoutMs) { safeStop(ErrorCode::TurnTimeout); return out; }
        if (elapsed >= settings_.turnMinMs && in.linePattern == 2) {
            turning_ = false;
            out.motor = MotorAction::Forward;
        } else out.motor = activeDeparture_ == Departure::Left ? MotorAction::PivotLeft : MotorAction::PivotRight;
        return out;
    }
    if (activeDeparture_ == Departure::Backward) { out.motor = MotorAction::Backward; return out; }
    if (activeDeparture_ == Departure::Straight &&
        static_cast<uint32_t>(in.now - phaseStartedMs_) < settings_.straightDepartureMs) {
        out.motor = MotorAction::Forward;
        return out;
    }
    out.motor = AGVMath::lineAction(in.linePattern);
    if (out.motor == MotorAction::Stop) safeStop(in.linePattern == 0 ? ErrorCode::LineLost : ErrorCode::AmbiguousLine);
    return out;
}
```

Vai trò: FSM và toán học production độc lập GPIO, được kiểm thử trực tiếp bằng Unity.

Giao tiếp: AGVController, Types, các module chuyển đổi.

Cần chỉnh: SafetySettings lấy từ Config.h; không tự tính A-Star.

SHA-256 source: `626558c0181855758bcc20394241be7e49774968fd24132025316f5b7ac6572a`.

## 30. CommandCodec.h

Source: [NCKHAGV/src/core/CommandCodec.h](../NCKHAGV/src/core/CommandCodec.h).

```cpp
#pragma once
#include "Types.h"

namespace CommandCodec {
bool decode(const uint8_t* payload, size_t length, AGVCommand& command, ErrorCode& reason);
}
```

Vai trò: Parse schema JSON, kiểm tra kiểu/giới hạn/path và lệnh node/nâng hạ, từ chối dữ liệu sai.

Giao tiếp: MQTTModule và ControlLogic.

Cần chỉnh: Đồng bộ server với docs/04-mqtt-contract-v11.md.

SHA-256 source: `b738159c524c319303a0aa5159a387d7ce1a6515a2d7f9c501993438e096495b`.

## 31. CommandCodec.cpp

Source: [NCKHAGV/src/core/CommandCodec.cpp](../NCKHAGV/src/core/CommandCodec.cpp).

```cpp
#include "CommandCodec.h"
#include "ControlLogic.h"
#include <ArduinoJson.h>
#include <cstring>

namespace {
struct PayloadReader {
    const uint8_t* data;
    size_t length, offset = 0;
    int read() { return offset < length ? data[offset++] : -1; }
    size_t readBytes(char* buffer, size_t count) {
        const size_t available = length - offset;
        if (count > available) count = available;
        std::memcpy(buffer, data + offset, count);
        offset += count;
        return count;
    }
};
bool identifier(const char* value) {
    if (!value || !value[0] || std::strlen(value) >= COMMAND_ID_SIZE) return false;
    for (const char* p = value; *p; ++p)
        if (!( (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
               (*p >= '0' && *p <= '9') || *p == '-' || *p == '_' || *p == '.')) return false;
    return true;
}
bool text(JsonVariantConst value, char* destination, size_t capacity) {
    if (!value.is<const char*>()) return false;
    const JsonString string = value.as<JsonString>();
    return string.size() == std::strlen(string.c_str()) && AGVMath::copyText(destination, capacity, string.c_str());
}
bool departure(const char* value, Departure& output) {
    if (!value || std::strcmp(value, "FOLLOW") == 0) output = Departure::FollowLine;
    else if (std::strcmp(value, "LEFT") == 0) output = Departure::Left;
    else if (std::strcmp(value, "RIGHT") == 0) output = Departure::Right;
    else if (std::strcmp(value, "STRAIGHT") == 0) output = Departure::Straight;
    else if (std::strcmp(value, "BACKWARD") == 0) output = Departure::Backward;
    else return false;
    return true;
}
}
bool CommandCodec::decode(const uint8_t* payload, size_t length, AGVCommand& command, ErrorCode& reason) {
    command = AGVCommand{};
    reason = ErrorCode::InvalidCommand;
    if (!payload || !length || length > Config::MAX_COMMAND_BYTES) return false;
    for (size_t i = 0; i < length; ++i) if (payload[i] == 0) return false;
    StaticJsonDocument<4096> doc;
    PayloadReader reader{payload, length};
    if (deserializeJson(doc, reader) || !doc.is<JsonObject>() ||
        !doc["version"].is<int>() || doc["version"].as<int>() != 1 || !doc["command"].is<const char*>()) return false;
    for (size_t i = reader.offset; i < length; ++i)
        if (payload[i] != ' ' && payload[i] != '\t' && payload[i] != '\n' && payload[i] != '\r') return false;
    if (!text(doc["id"], command.id, sizeof(command.id)) || !identifier(command.id)) return false;
    if (doc["command"].as<JsonString>().size() != std::strlen(doc["command"].as<const char*>())) return false;
    const char* name = doc["command"];
    if (std::strcmp(name, "TASK") == 0) command.type = CommandType::Task;
    else if (std::strcmp(name, "START") == 0) command.type = CommandType::Start;
    else if (std::strcmp(name, "STOP") == 0) command.type = CommandType::Stop;
    else if (std::strcmp(name, "EMERGENCY_STOP") == 0) command.type = CommandType::EmergencyStop;
    else if (std::strcmp(name, "RESTART") == 0) command.type = CommandType::Restart;
    else if (std::strcmp(name, "CANCEL_TASK") == 0) command.type = CommandType::CancelTask;
    else if (std::strcmp(name, "LIFT_UP") == 0) command.type = CommandType::LiftUp;
    else if (std::strcmp(name, "LIFT_DOWN") == 0) command.type = CommandType::LiftDown;
    else if (std::strcmp(name, "NODE_ORDER") == 0) command.type = CommandType::NodeOrder;
    else return false;
    const bool stop = command.type == CommandType::Stop || command.type == CommandType::EmergencyStop;
    if (!doc["session"].is<uint32_t>() || (!stop && doc["session"].as<uint32_t>() == 0)) return false;
    command.session = doc["session"].as<uint32_t>();
    if (command.type == CommandType::Task) {
        reason = ErrorCode::InvalidTask;
        JsonObjectConst task = doc["task"].as<JsonObjectConst>();
        if (task.isNull() || !text(task["id"], command.task.id, sizeof(command.task.id)) || !identifier(command.task.id) ||
            !text(task["startNode"], command.task.startNode, sizeof(command.task.startNode)) ||
            !text(task["targetNode"], command.task.targetNode, sizeof(command.task.targetNode))) return false;
        JsonArrayConst path = task["path"].as<JsonArrayConst>();
        if (path.isNull() || path.size() == 0 || path.size() > Config::MAX_PATH_NODES) return false;
        command.task.pathLength = static_cast<uint8_t>(path.size());
        size_t i = 0;
        for (JsonVariantConst value : path) {
            PathStep& step = command.task.path[i++];
            if (value.is<const char*>()) {
                if (!text(value, step.node, sizeof(step.node))) return false;
            } else if (value.is<JsonObjectConst>()) {
                if (!text(value["node"], step.node, sizeof(step.node))) return false;
                if (!value["departure"].isNull() && !value["departure"].is<const char*>()) return false;
                if (value["departure"].is<const char*>() && value["departure"].as<JsonString>().size() != std::strlen(value["departure"].as<const char*>())) return false;
                if (!departure(value["departure"].as<const char*>(), step.departure)) return false;
            } else return false;
        }
        if (!AGVMath::validTask(command.task)) return false;
    } else if (command.type == CommandType::NodeOrder || command.type == CommandType::LiftUp || command.type == CommandType::LiftDown) {
        if (!text(doc["taskId"], command.taskId, sizeof(command.taskId)) || !identifier(command.taskId) ||
            !text(doc["node"], command.node, sizeof(command.node)) || !AGVMath::validNode(command.node) ||
            !doc["pathIndex"].is<uint8_t>()) return false;
        command.pathIndex = doc["pathIndex"].as<uint8_t>();
        if (command.type == CommandType::NodeOrder) {
            if (!text(doc["nextNode"], command.nextNode, sizeof(command.nextNode)) ||
                !AGVMath::validNode(command.nextNode) || !doc["departure"].is<const char*>() ||
                !departure(doc["departure"].as<const char*>(), command.departure) ||
                command.departure == Departure::FollowLine) return false;
        }
    }
    reason = ErrorCode::None;
    return true;
}
```

Vai trò: Parse schema JSON, kiểm tra kiểu/giới hạn/path và lệnh node/nâng hạ, từ chối dữ liệu sai.

Giao tiếp: MQTTModule và ControlLogic.

Cần chỉnh: Đồng bộ server với docs/04-mqtt-contract-v11.md.

SHA-256 source: `4ab65f6a304256896a8eed6e69da335b926651e848deac119f0290a8432066b4`.

## 32. CargoModule.h

Source: [NCKHAGV/src/sensors/CargoModule.h](../NCKHAGV/src/sensors/CargoModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include "Config.h"

class CargoModule {
public:
    void begin();
    void update(uint32_t now);
    bool stable() const { return stable_; }
    bool present() const { return stable_ && value_; }
    bool absent() const { return stable_ && !value_; }
private:
    bool raw_ = false, value_ = false, stable_ = false;
    uint32_t changedAtMs_ = 0;
};
```

Vai trò: Đọc IR hàng hướng lên, lọc tín hiệu trước khi controller xác nhận hàng hoặc báo mất hàng.

Giao tiếp: AGVController.

Cần chỉnh: CARGO_IR_PIN, cực tính và thời gian ổn định trong Config.h.

SHA-256 source: `f7c6a50d99b7a6e21fc8946814cb0e920231f0f9d9b678525b348f175aa17e54`.

## 33. CargoModule.cpp

Source: [NCKHAGV/src/sensors/CargoModule.cpp](../NCKHAGV/src/sensors/CargoModule.cpp).

```cpp
#include "CargoModule.h"

void CargoModule::begin() {
    pinMode(Config::CARGO_IR_PIN, INPUT);
    raw_ = digitalRead(Config::CARGO_IR_PIN) == Config::CARGO_IR_ACTIVE_STATE;
    changedAtMs_ = millis();
    stable_ = false;
}
void CargoModule::update(uint32_t now) {
    const bool sample = digitalRead(Config::CARGO_IR_PIN) == Config::CARGO_IR_ACTIVE_STATE;
    if (sample != raw_) { raw_ = sample; changedAtMs_ = now; stable_ = false; }
    if (static_cast<uint32_t>(now - changedAtMs_) >= Config::CARGO_IR_STABLE_MS) {
        value_ = raw_;
        stable_ = true;
    }
}
```

Vai trò: Đọc IR hàng hướng lên, lọc tín hiệu trước khi controller xác nhận hàng hoặc báo mất hàng.

Giao tiếp: AGVController.

Cần chỉnh: CARGO_IR_PIN, cực tính và thời gian ổn định trong Config.h.

SHA-256 source: `8fa046743bbe293b0c18459a7d8c090c03e31e4fa4f9fed99aae78abe2f20032`.

## 34. BuzzerModule.h

Source: [NCKHAGV/src/actuators/BuzzerModule.h](../NCKHAGV/src/actuators/BuzzerModule.h).

```cpp
#pragma once
#include <Arduino.h>
#include "Config.h"

class BuzzerModule {
public:
    void begin() {
        digitalWrite(Config::BUZZER_PIN, !Config::BUZZER_ACTIVE_STATE);
        pinMode(Config::BUZZER_PIN, OUTPUT);
        off();
    }
    void setReverse(bool active) {
        digitalWrite(Config::BUZZER_PIN, active ? Config::BUZZER_ACTIVE_STATE : !Config::BUZZER_ACTIVE_STATE);
    }
    void off() { setReverse(false); }
};
```

Vai trò: Điều khiển còi cảnh báo khi lùi; triển khai header-only.

Giao tiếp: MovementModule và AGVController.

Cần chỉnh: BUZZER_PIN và cực tính trong Config.h.

SHA-256 source: `c0d015fa83fb37ca1209c3ae6e838e93e47581638cfdfffd45b7f0a90b4580be`.


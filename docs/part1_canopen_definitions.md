# Part 1: Dinh Nghia CANopen va Cau Truc Du Lieu (kinco_ismk_def.h)

Tài liệu kỹ thuật mô tả chi tiết ý nghĩa của các hằng số, bảng địa chỉ hòm thư CANopen và khuôn mẫu cấu trúc dữ liệu trong file kinco_ismk_def.h (áp dụng cho hệ thống sử dụng 1 Động cơ - Cố định Node ID = 1).

---

## 1. Mục Đích và Bản Chất
File kinco_ismk_def.h đóng vai trò là từ điển và quy ước giao tiếp chung cho toàn bộ driver:
- Không chứa mã thực thi logic điều khiển.
- Định nghĩa toàn bộ hằng số địa chỉ giao tiếp theo tiêu chuẩn công nghiệp quốc tế CiA 301 (CANopen Application Layer) và CiA 402 (Device Profile for Drives and Motion Control).
- Cố định toàn bộ các địa chỉ CAN ID cho Motor 1 (Node ID = 1), giúp mã nguồn tối ưu, loại bỏ việc tính toán cộng dồn ID trong thời gian thực.

---

## 2. Bảng Địa Chỉ Gói Tin CANopen (CAN Identifier / COB-ID)

Các địa chỉ này được chuẩn hóa theo Bảng 9-12 (Trang 95) trong tài liệu kỹ thuật chính thức Kinco iSMK Integrated Servo User Manual:

| Tên Macro trong Code | CAN ID (Hex) | Chiều truyền | Ý nghĩa và Chức năng kỹ thuật |
| :--- | :---: | :---: | :--- |
| CANOPEN_COB_NMT | 0x000 | STM32 -> Motor | Lệnh quản lý mạng (NMT): Chuyển đổi trạng thái motor giữa Pre-operational và Operational. |
| CANOPEN_COB_SYNC | 0x080 | STM32 -> Motor | Xung đồng bộ: Đồng bộ chu kỳ truyền thông trên mạng CANopen. |
| CANOPEN_COB_EMCY | 0x081 | Motor -> STM32 | Báo lỗi khẩn cấp (Emergency): Motor tự động phát gói tin khi phát hiện sự cố (quá tải, kẹt trục, sụt áp). |
| CANOPEN_COB_TPDO1 | 0x181 | Motor -> STM32 | Feedback thời gian thực 1: Phản hồi Statusword (Trạng thái servo) và Actual Position (Vị trí góc encoder). |
| CANOPEN_COB_RPDO1 | 0x201 | STM32 -> Motor | Lệnh thời gian thực 1: Truyền trực tiếp Controlword điều khiển máy trạng thái. |
| CANOPEN_COB_TPDO2 | 0x281 | Motor -> STM32 | Feedback thời gian thực 2: Phản hồi Actual Speed (Vận tốc thực tế RPM) và Actual Current (Dòng điện tải). |
| CANOPEN_COB_RPDO2 | 0x301 | STM32 -> Motor | Lệnh thời gian thực 2: Truyền trực tiếp giá trị vận tốc mục tiêu. |
| CANOPEN_COB_SDO_TX | 0x581 | Motor -> STM32 | Phản hồi SDO: Gói tin trả lời dữ liệu từ motor khi STM32 đọc/ghi tham số. |
| CANOPEN_COB_SDO_RX | 0x601 | STM32 -> Motor | Yêu cầu SDO: Kênh cấu hình thông số (góc quay, vận tốc, gia tốc, chế độ hoạt động). |
| CANOPEN_COB_HEARTBEAT | 0x701 | Motor -> STM32 | Giám sát kết nối (Heartbeat): Gói tin báo trạng thái định kỳ để phát hiện đứt cáp truyền thông. |

---

## 3. Bảng Thanh Ghi Đối Tượng CiA 402 (Object Dictionary)

Toàn bộ các thanh ghi này tuân thủ cấu trúc từ điển đối tượng CiA 402 (trích xuất từ Chương 7 sách Kinco iSMK):

| Macro định nghĩa | Địa chỉ Hex | Kiểu dữ liệu | Mô tả chức năng |
| :--- | :---: | :---: | :--- |
| KINCO_OBJ_CONTROLWORD | 0x6040 | uint16 | Thanh ghi điều khiển: Nhận các lệnh chuyển đổi trạng thái 0x06 (Shutdown), 0x07 (Switch on), 0x0F (Enable), 0x86 (Fault Reset). |
| KINCO_OBJ_STATUSWORD | 0x6041 | uint16 | Thanh ghi trạng thái: Cung cấp thông tin trạng thái hoạt động hiện tại, cảnh báo lỗi và cờ báo đến đích (Target Reached). |
| KINCO_OBJ_MODES_OF_OPER | 0x6060 | int8 | Cài đặt chế độ hoạt động: 1 (Profile Position), 3 (Profile Velocity), 4 (Profile Torque), 6 (Homing). |
| KINCO_OBJ_TARGET_POSITION | 0x607A | int32 | Tọa độ góc đích: Giá trị vị trí cần di chuyển tới trong chế độ Position Mode (đơn vị: xung encoder inc). |
| KINCO_OBJ_PROFILE_VELOCITY | 0x6081 | uint32 | Vận tốc tối đa: Giới hạn tốc độ trong quá trình di chuyển vị trí. |
| KINCO_OBJ_TARGET_VELOCITY | 0x60FF | int32 | Vận tốc mục tiêu: Tốc độ quay mong muốn trong chế độ Velocity Mode (đơn vị: DEC). |
| KINCO_OBJ_ACTUAL_POSITION | 0x6063 | int32 | Vị trí thực tế: Đọc giá trị encoder phản hồi từ trục động cơ. |
| KINCO_OBJ_ACTUAL_SPEED | 0x606C | int32 | Vận tốc thực tế: Giá trị tốc độ quay hiện tại của động cơ. |
| KINCO_OBJ_PROFILE_ACCEL | 0x6083 | uint32 | Gia tốc tăng tốc: Độ dốc tăng tốc độ quay (đơn vị: rps/s). |
| KINCO_OBJ_PROFILE_DECEL | 0x6084 | uint32 | Gia tốc giảm tốc: Độ dốc hãm phanh khi giảm tốc độ. |

---

## 4. Cấu Trúc Đối Tượng Motor (Kinco_Motor_t)

Cấu trúc struct được thiết kế nhằm đóng gói toàn bộ trạng thái hoạt động và tham số của động cơ trong một vùng nhớ duy nhất:

```c
typedef struct {
    uint32_t        encoder_resolution;  // Độ phân giải encoder (mặc định iSMK là 65536)
    Kinco_Mode_t    target_mode;         // Chế độ hoạt động mong muốn (Mặc định: Position Mode 1)

    Kinco_State_t   state;               // Trạng thái máy trạng thái CiA 402 hiện tại
    uint16_t        status_word;         // Giá trị Statusword đọc về từ thanh ghi 0x6041
    bool            is_connected;        // Cờ báo trạng thái kết nối CAN
    bool            fault_active;        // Cờ báo lỗi động cơ (Fault)
    bool            target_reached;      // Cờ báo đã hoàn thành di chuyển đến vị trí đặt
    uint16_t        emergency_code;      // Mã lỗi khẩn cấp nhận từ gói tin EMCY

    int32_t         actual_pos_inc;      // Vị trí thực tế (đơn vị: xung encoder)
    float           actual_pos_deg;      // Góc quay thực tế (đơn vị: độ)
    float           actual_speed_rpm;    // Vận tốc thực tế (đơn vị: vòng/phút - RPM)
    float           actual_torque_pct;   // Tải mô-men thực tế (đơn vị: % mô-men định mức)

    // Con trỏ hàm truyền dữ liệu CAN liên kết với driver ngoại vi của vi điều khiển
    bool (*can_send)(uint32_t can_id, const uint8_t *data, uint8_t dlc);
} Kinco_Motor_t;
```

### Ví dụ sử dụng trong chương trình chính (main.c):
```c
Kinco_Motor_t motor;

// Khởi tạo đối tượng với độ phân giải encoder 65536 và hàm gửi CAN
Kinco_Init(&motor, 65536, STM32_CAN_Send);
```
Toàn bộ thông số giám sát và điều khiển sẽ được cập nhật trực tiếp vào đối tượng motor.

# Part 1: Định Nghĩa CANopen & Cấu Trúc Dữ Liệu (`kinco_ismk_def.h`)

Tài liệu này giải thích chi tiết ý nghĩa kỹ thuật của các hằng số, bảng địa chỉ hòm thư CANopen và khuôn mẫu cấu trúc dữ liệu trong file `kinco_ismk_def.h`.

---

## 📌 1. Bản chất của Part 1
File `kinco_ismk_def.h` đóng vai trò là **"Cuốn từ điển & Bảng quy ước chung"** cho toàn bộ driver:
- **Không chứa mã lệnh logic chạy motor.**
- Định nghĩa toàn bộ hằng số địa chỉ giao tiếp theo tiêu chuẩn quốc tế **CiA 301** (CANopen) và **CiA 402** (Motion Control Profile).
- Tạo ra khuôn mẫu đối tượng `Kinco_Motor_t` để quản lý độc lập từng con motor trong hệ thống nhiều trục (như xe AGV 2 bánh).

---

## 📬 2. Bảng Hòm Thư CANopen (CAN Identifier / COB-ID)

Các địa chỉ này được trích xuất trực tiếp từ **Bảng 9-12 (Trang 95)** trong tài liệu chính hãng *Kinco iSMK Integrated Servo User Manual*.

| Tên hòm thư | Địa chỉ Base | CAN ID thực tế (Node ID = $N$) | Chiều truyền | Ý nghĩa chức năng |
| :--- | :---: | :---: | :---: | :--- |
| **NMT** | `0x000` | `0x000` | STM32 ➔ Motor | Lệnh chỉ huy toàn mạng: Đánh thức motor (`Start`), Dừng (`Stop`), Reset. |
| **SYNC** | `0x080` | `0x080` | STM32 ➔ All | Xung đồng bộ phần cứng: Giúp 2 bánh xe nhận lệnh cùng 1 micro-giây. |
| **EMCY** | `0x080` | `0x080 + N` *(VD: `0x081`)* | Motor ➔ STM32 | Báo lỗi khẩn cấp: Tự động bắn ra ngay khi kẹt trục, quá dòng, sụt áp. |
| **TPDO1** | `0x180` | `0x180 + N` *(VD: `0x181`)* | Motor ➔ STM32 | Feedback định kỳ: Chứa **Statusword (trạng thái)** và **Vị trí thực tế**. |
| **RPDO1** | `0x200` | `0x200 + N` *(VD: `0x201`)* | STM32 ➔ Motor | Gửi lệnh thời gian thực: Nạp trực tiếp Controlword xuống motor. |
| **TPDO2** | `0x280` | `0x280 + N` *(VD: `0x281`)* | Motor ➔ STM32 | Feedback định kỳ: Chứa **Vận tốc thực tế (RPM)** và **Dòng điện tải**. |
| **RPDO2** | `0x300` | `0x300 + N` *(VD: `0x301`)* | STM32 ➔ Motor | Gửi lệnh thời gian thực: Nạp trực tiếp Vận tốc đích xuống motor. |
| **SDO Tx** | `0x580` | `0x580 + N` *(VD: `0x581`)* | Motor ➔ STM32 | Phản hồi của Motor khi STM32 gửi lệnh đọc/ghi tham số cài đặt. |
| **SDO Rx** | `0x600` | `0x600 + N` *(VD: `0x601`)* | STM32 ➔ Motor | Kênh cài đặt cấu hình: STM32 ghi tham số vào motor qua kênh này. |
| **HEARTBEAT**| `0x700` | `0x700 + N` *(VD: `0x701`)* | Motor ➔ STM32 | Gói tin điểm danh: Bắn ra định kỳ để báo cho STM32 biết motor còn sống. |

---

## 🗄️ 3. Bảng Ngăn Kéo Thanh Ghi CiA 402 (Object Dictionary)

Toàn bộ các thanh ghi này tuân thủ chuẩn điều khiển truyền động CiA 402 (trích từ **Chương 7 sách Kinco iSMK**):

| Tên định nghĩa trong code | Địa chỉ Hex | Kiểu dữ liệu | Mô tả chức năng |
| :--- | :---: | :---: | :--- |
| `KINCO_OBJ_CONTROLWORD` | `0x6040` | `uint16` | **Thanh ghi điều khiển:** Nhận các lệnh `0x06` (Shutdown), `0x07` (Switch on), `0x0F` (Enable), `0x86` (Clear fault). |
| `KINCO_OBJ_STATUSWORD` | `0x6041` | `uint16` | **Thanh ghi trạng thái:** Đọc xem motor đã bật chưa, có bị cờ lỗi không, đã quay đến góc đích chưa (`Target reached`). |
| `KINCO_OBJ_MODES_OF_OPER` | `0x6060` | `int8` | **Cài đặt chế độ hoạt động:** 1 (Góc/Vị trí), 3 (Vận tốc RPM), 4 (Lực Torque). |
| `KINCO_OBJ_TARGET_POSITION`| `0x607A` | `int32` | **Vị trí góc đích:** Chứa số xung encoder (inc) cần quay tới trong Mode 1. |
| `KINCO_OBJ_PROFILE_VELOCITY`| `0x6081` | `uint32` | **Vận tốc tối đa khi quay góc:** Giới hạn tốc độ xoay góc để không bị giật. |
| `KINCO_OBJ_TARGET_VELOCITY`| `0x60FF` | `int32` | **Vận tốc đích:** Tốc độ quay mong muốn trong Mode 3 (đơn vị: xung nội bộ DEC). |
| `KINCO_OBJ_ACTUAL_POSITION`| `0x6063` | `int32` | **Vị trí thực tế:** Đọc góc quay encoder hiện tại của trục motor. |
| `KINCO_OBJ_ACTUAL_SPEED` | `0x606C` | `int32` | **Vận tốc thực tế:** Tốc độ quay hiện tại của motor (đơn vị DEC). |
| `KINCO_OBJ_PROFILE_ACCEL` | `0x6083` | `uint32` | **Gia tốc tăng tốc:** Đặt độ dốc tăng tốc mượt mà (đơn vị: rps/s). |
| `KINCO_OBJ_PROFILE_DECEL` | `0x6084` | `uint32` | **Gia tốc giảm tốc:** Đặt độ dốc hãm phanh khi dừng lại. |

---

## 🏗️ 4. Cấu Trúc Hồ Sơ Đối Tượng Motor (`Kinco_Motor_t`)

Trong lập trình C nhúng, việc đóng gói mọi thông tin của motor vào một cấu trúc `struct` mang lại lợi ích rất lớn:

```c
typedef struct {
    uint8_t         node_id;             // Địa chỉ mạng (1, 2, 3...)
    uint32_t        encoder_resolution;  // Độ phân giải encoder (iSMK là 65536)
    Kinco_Mode_t    target_mode;         // Chế độ đang chọn (Mode 1: Góc)

    Kinco_State_t   state;               // Trạng thái máy trạng thái DS402
    uint16_t        status_word;         // Giá trị thanh ghi 0x6041
    bool            is_connected;        // Đã nối mạng thành công
    bool            fault_active;        // Báo true nếu có lỗi
    bool            target_reached;      // Báo true khi đã quay đến góc đích

    int32_t         actual_pos_inc;      // Vị trí thực tế (xung)
    float           actual_pos_deg;      // Góc thực tế (độ)
    float           actual_speed_rpm;    // Vận tốc thực tế (RPM)
    float           actual_torque_pct;   // Lực tải thực tế (%)

    // Con trỏ hàm gửi gói tin CAN liên kết với STM32 HAL
    bool (*can_send)(uint32_t can_id, const uint8_t *data, uint8_t dlc);
} Kinco_Motor_t;
```

### 💡 Lợi ích thực tế:
Khi làm xe tự hành AGV có 2 bánh, bạn chỉ cần khai báo:
```c
Kinco_Motor_t motor_left;   // Bánh trái (Node ID = 1)
Kinco_Motor_t motor_right;  // Bánh phải (Node ID = 2)
```
- Góc quay, vận tốc và cờ lỗi của từng bánh xe sẽ được lưu độc lập trong từng biến.
- Không bao giờ xảy ra tình trạng bánh này đè dữ liệu lên bánh kia.

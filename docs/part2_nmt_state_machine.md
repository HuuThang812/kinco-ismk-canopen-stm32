# Part 2: Quản Lý Mạng NMT và Máy Trạng Thái CiA 402 (kinco_ismk_core.c)

Tài liệu này hướng dẫn chi tiết quy trình khởi tạo truyền thông, đánh thức mạng CANopen và chuỗi lệnh kích hoạt động cơ servo Kinco iSMK theo tiêu chuẩn máy trạng thái CiA 402 (DS402).

---

## 1. Tổng Quan Quy Trình Khởi Động

Khác với các dòng động cơ thông thường chỉ cần cấp điện là hoạt động, động cơ servo công nghiệp tuân theo tiêu chuẩn CANopen CiA 402 yêu cầu quy trình khởi động qua hai lớp bảo vệ nghiêm ngặt:

1. **Lớp truyền thông CANopen (NMT Layer):** Chuyển trạng thái vi xử lý của driver từ chế độ chờ (Pre-operational) sang chế độ làm việc (Operational).
2. **Lớp máy trạng thái truyền động (CiA 402 State Machine):** Chuyển trạng thái mạch công suất từ ngắt mạch động lực sang cấp điện, khóa trục và nhả phanh an toàn.

---

## 2. Quản Lý Mạng CANopen (NMT Management)

Khi được cấp nguồn, động cơ Kinco iSMK tự động ở trạng thái Pre-operational. Ở trạng thái này, động cơ chỉ tiếp nhận cấu hình tham số qua SDO và từ chối mọi lệnh điều khiển chuyển động.

Vi điều khiển STM32 cần gửi bản tin NMT Master để chuyển động cơ sang trạng thái Operational:

### Bản tin NMT Start Node:
- **CAN ID:** 0x000 (NMT Module Control)
- **DLC:** 2 byte
- **Dữ liệu:** 0x01 0x01 (Byte 0: Lệnh Start Node 0x01; Byte 1: Node ID mục tiêu 0x01)

### Hàm thực thi trong thư viện:
```c
// Đánh thức động cơ sang trạng thái Operational
Kinco_NMT_Start(&motor);

// Khởi động lại vi xử lý của động cơ từ xa qua CAN
Kinco_NMT_Reset(&motor);
```

---

## 3. Máy Trạng Thái CiA 402 (DS402 State Machine)

Sau khi mạng CANopen đã ở trạng thái Operational, việc cấp nguồn và khóa trục động cơ được điều khiển tuần tự thông qua thanh ghi Controlword (0x6040) (gửi qua kênh SDO 0x601):

```text
[ Cấp nguồn ] 
      |
      v
[ Switch On Disabled ] (Trạng thái sau khởi động)
      |
      |  Gửi 0x0006 (Shutdown)
      v
[ Ready to Switch On ] (Mạch điều khiển sẵn sàng)
      |
      |  Gửi 0x0007 (Switch On)
      v
[ Switched On ] (Mạch công suất sẵn sàng)
      |
      |  Gửi 0x000F (Enable Operation)
      v
[ OPERATION ENABLED ]  <=== Khóa cứng trục động cơ & Mở phanh điện từ!
```

### Danh sách các hàm điều khiển trạng thái:

| Tên Hàm | Mã Controlword | Ý nghĩa kỹ thuật |
| :--- | :---: | :--- |
| Kinco_ClearFault(&motor) | 0x0086 | Xóa cờ lỗi (Fault Reset): Đưa động cơ thoát khỏi trạng thái Fault do quá dòng, kẹt trục hoặc sụt áp. |
| Kinco_Shutdown(&motor) | 0x0006 | Chuẩn bị bật: Chuyển trạng thái sang Ready to Switch On. |
| Kinco_SwitchOn(&motor) | 0x0007 | Bật công suất: Đóng mạch động lực, chuyển sang Switched On. |
| Kinco_EnableOperation(&motor) | 0x000F | Kích hoạt động cơ: Cấp dòng điện giữ trục, mở cuộn phanh cơ (Brake Release), sẵn sàng nhận lệnh quay. |
| Kinco_DisableOperation(&motor) | 0x0006 | Vô hiệu hóa: Thả lỏng trục quay tự do. |
| Kinco_QuickStop(&motor) | 0x0002 | Dừng khẩn cấp: Hãm phanh dừng động cơ theo độ dốc Quick Stop. |

---

## 4. Khởi Tạo và Cấu Hình Chế Độ Hoạt Động (Mode of Operation)

Hàm Kinco_Init() thực hiện thiết lập cấu hình ban đầu cho đối tượng động cơ:
- Cố định địa chỉ mạng Node ID = 1.
- Cấu hình độ phân giải encoder (mặc định là 65536 xung/vòng đối với dòng iSMK).
- Tự động thiết lập chế độ hoạt động mặc định là Chế độ Vị trí / Góc (Profile Position Mode = 1).

```c
void Kinco_Init(Kinco_Motor_t *motor, uint32_t encoder_resolution,
                bool (*can_send_callback)(uint32_t can_id, const uint8_t *data, uint8_t dlc));
```

Nếu muốn chuyển đổi chế độ thủ công:
```c
Kinco_SetMode(&motor, KINCO_MODE_PROFILE_POSITION); // Mode 1: Góc / Vị trí
Kinco_SetMode(&motor, KINCO_MODE_PROFILE_VELOCITY); // Mode 3: Vận tốc (RPM)
Kinco_SetMode(&motor, KINCO_MODE_PROFILE_TORQUE);   // Mode 4: Mô-men xoắn (Lực)
```

---

## 5. Ví Dụ Trình Tự Khởi Động Chuẩn Trong main()

Trình tự mã nguồn chuẩn cần thực hiện khi khởi động hệ thống vi điều khiển STM32:

```c
#include "kinco_ismk.h"

Kinco_Motor_t motor;

int main(void) {
    // 1. Khởi tạo ngoại vi STM32 (Clock, GPIO, CAN...)
    HAL_Init();
    SystemClock_Config();
    MX_CAN_Init();

    // 2. Khởi tạo đối tượng động cơ (Độ phân giải 65536, gắn hàm gửi CAN)
    Kinco_Init(&motor, 65536, STM32_CAN_Send);

    // 3. Đánh thức mạng CANopen (NMT Start)
    Kinco_NMT_Start(&motor);
    HAL_Delay(10);

    // 4. Xóa cờ lỗi nếu có từ phiên trước
    Kinco_ClearFault(&motor);
    HAL_Delay(50);

    // 5. Chuỗi kích hoạt động cơ (CiA 402 Enable Sequence)
    Kinco_Shutdown(&motor);         // Gửi 0x0006
    HAL_Delay(20);

    Kinco_SwitchOn(&motor);         // Gửi 0x0007
    HAL_Delay(20);

    Kinco_EnableOperation(&motor);  // Gửi 0x000F -> Trục khóa cứng và phanh nhả
    HAL_Delay(50);

    // Sau bước này, động cơ đã sẵn sàng thực hiện các lệnh quay ở Part 3
    while (1) {
        // Vòng lặp điều khiển chính
    }
}
```

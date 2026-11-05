# Kinco iSMK Integrated Servo CANopen (CiA 402) Driver for STM32

Driver điều khiển động cơ servo tích hợp **Kinco iSMK Series** (iSMK40, iSMK60, iSMK80) theo tiêu chuẩn truyền thông công nghiệp **CANopen (CiA 402)** dành cho dòng vi điều khiển **STM32** (STM32F1, F4, G4, H7...) sử dụng **STM32 HAL**.

---

## 📌 Tính năng chính (Key Features)

- ✅ **Chuẩn CiA 402 DS402:** Hỗ trợ đầy đủ máy trạng thái khởi động an toàn (`Shutdown 0x06` ➔ `Switch On 0x07` ➔ `Enable Operation 0x0F`), xóa cờ lỗi (`Fault Reset 0x86`).
- ✅ **Độc lập phần cứng (HAL-independent):** Sử dụng callback con trỏ hàm gửi gói tin CAN. Dễ dàng chuyển đổi giữa các dòng STM32 hoặc port sang ESP32 / Arduino / ROS / Linux.
- ✅ **Hỗ trợ đa trục (Multi-axis):** Quản lý động cơ theo kiểu đối tượng C OOP (`Kinco_Motor_t`), điều khiển độc lập hoặc đồng bộ nhiều motor (bánh trái, bánh phải, trục nâng...).
- ✅ **Hỗ trợ đầy đủ 3 chế độ điều khiển:**
  - 🚗 **Profile Velocity Mode (Mode 3):** Điều khiển vận tốc theo đơn vị thực tế **RPM** (vòng/phút), tự động chuyển đổi sang đơn vị xung nội bộ (DEC).
  - 🎯 **Profile Position Mode (Mode 1):** Điều khiển vị trí góc quay theo **Độ (Degrees)** hoặc **Số xung Encoder (inc)** với cơ chế kích cờ sườn xung Bit 4 (*New Setpoint*).
  - 💪 **Profile Torque Mode (Mode 4):** Điều khiển mô-men xoắn / lực ép theo **% mô-men định mức**, có giới hạn tốc độ bảo vệ.
- ✅ **Bóc tách Feedback thời gian thực:** Tự động giải mã các gói tin `TPDO1` (Vị trí, Statusword), `TPDO2` (Vận tốc RPM, dòng điện), và gói tin báo lỗi khẩn cấp `EMCY`.

---

## ⚡ Sơ đồ chân & Đấu nối (Wiring Guide)

### 1. Cáp nguồn chính (Power Cable - 2 dây to)
| Chân | Dây | Tín hiệu | Mô tả |
| :---: | :---: | :---: | :--- |
| **Pin 1** | Đỏ | **DC+** | Cấp nguồn chính **`24VDC ~ 48VDC`** |
| **Pin 3** | Đen | **DC-** | Cấp nguồn **`GND (0V)`** |

### 2. Cáp tín hiệu (Signal Cable - 18 Chân)
| Chân | Tên chân | Màu dây (Thân) | Chức năng |
| :---: | :---: | :---: | :--- |
| **Pin 1** | **24V** | Đỏ | Nguồn Logic 24VDC nuôi vi xử lý driver |
| **Pin 10** | **GND** | Đen | Mass nguồn logic *(⚠️ Xem lưu ý bên dưới!)* |
| **Pin 2** | **Lock+** | Tím | Cực (+) mở phanh cơ (24V) |
| **Pin 11** | **Lock-** | Tím sọc đen | Cực (-) mở phanh cơ (0V) |
| **Pin 3, 4** | **CANH** | Xanh sọc đen | Đường truyền CAN High |
| **Pin 12, 13** | **CANL** | Xanh dương | Đường truyền CAN Low |
| **Pin 5, 6** | **RS485A** | Cam sọc đen | Cổng RS485 (dành cho cấu hình KS3 / Modbus) |
| **Pin 14, 15**| **RS485B** | Cam | Cổng RS485 |

> ⚠️ **LƯU Ý CỰC KỲ QUAN TRỌNG VỀ NGUỒN 24V:**
> - Nếu dùng **CHUNG 1 bộ nguồn 24V** cho cả nguồn chính và logic: Cực $(+)$ nối vào cả `DC+` và Pin 1 (`24V`). Cực $(-)$ nối vào `DC-`.
> - **TUYỆT ĐỐI KHÔNG ĐẤU PIN 10 (GND LOGIC) VỀ CỰC (-) NGUỒN** nếu dùng chung nguồn để tránh hiện tượng vòng lặp mass (*Ground Loop*) làm cháy dây tín hiệu khi motor ăn dòng lớn.
> - Bắt buộc gắn **điện trở đầu cuối $120\,\Omega$** giữa 2 dây `CANH` và `CANL` ở 2 đầu mạng CAN bus.

---

## 📐 Công thức quy đổi đơn vị (Unit Scaling)

Các công thức trích xuất từ Manual Kinco iSMK:
- **Tốc độ (Speed):**
  $$\text{DEC} = \left\lfloor \frac{\text{RPM} \times 512 \times \text{Encoder\_Resolution}}{1875} \right\rfloor$$
  *(Với encoder 65.536 xung mặc định của iSMK: $1\text{ RPM} \approx 17895\text{ DEC}$)*.
- **Gia tốc (Acceleration):**
  $$\text{DEC} = \left\lfloor \frac{(\text{RPS/s}) \times 65536 \times \text{Encoder\_Resolution}}{4000000} \right\rfloor$$
- **Vị trí góc (Angle):**
  $$\text{Target Inc} = \left\lfloor \frac{\text{Degrees} \times \text{Encoder\_Resolution}}{360.0} \right\rfloor$$

---

## 🚀 Hướng dẫn tích hợp vào STM32 (Quick Start)

### 1. Thêm file vào project STM32CubeIDE
Copy 2 file `kinco_ismk.h` và `kinco_ismk.c` vào thư mục `Core/Inc` và `Core/Src` của project.

### 2. Gắn hàm gửi CAN và Ngắt nhận CAN
Trong file `main.c` (hoặc `can.c`):

```c
#include "kinco_ismk.h"

extern CAN_HandleTypeDef hcan1;
Kinco_Motor_t motor_left;

// 1. Hàm gửi CAN
bool STM32_CAN_Send(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;

    tx_header.StdId = can_id;
    tx_header.IDE   = CAN_ID_STD; // Standard 11-bit
    tx_header.RTR   = CAN_RTR_DATA;
    tx_header.DLC   = dlc;
    tx_header.TransmitGlobalTime = DISABLE;

    return (HAL_CAN_AddTxMessage(&hcan1, &tx_header, (uint8_t*)data, &tx_mailbox) == HAL_OK);
}

// 2. Ngắt nhận CAN
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
        Kinco_ProcessRxMessage(&motor_left, rx_header.StdId, rx_data, rx_header.DLC);
    }
}
```

### 3. Cấu hình Filter cho CAN trong `main()`
```c
CAN_FilterTypeDef can_filter;
can_filter.FilterBank = 0;
can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
can_filter.FilterIdHigh = 0x0000;
can_filter.FilterIdLow = 0x0000;
can_filter.FilterMaskIdHigh = 0x0000;
can_filter.FilterMaskIdLow = 0x0000; // Nhận tất cả gói tin CAN
can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;
can_filter.FilterActivation = ENABLE;
HAL_CAN_ConfigFilter(&hcan1, &can_filter);

HAL_CAN_Start(&hcan1);
HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
```

### 4. Quy trình chạy motor
```c
// Khởi tạo motor Node ID = 1, encoder 65536
Kinco_Init(&motor_left, 1, 65536, STM32_CAN_Send);

// Đánh thức mạng CAN
Kinco_NMT_Start(&motor_left);
HAL_Delay(10);

// Xóa lỗi nếu có
Kinco_ClearFault(&motor_left);
HAL_Delay(50);

// Chọn Mode vận tốc
Kinco_SetMode(&motor_left, KINCO_MODE_PROFILE_VELOCITY);
HAL_Delay(20);

// Chuỗi kích hoạt CiA 402 Enable
Kinco_Shutdown(&motor_left);         // 0x0006
HAL_Delay(20);
Kinco_SwitchOn(&motor_left);         // 0x0007
HAL_Delay(20);
Kinco_EnableOperation(&motor_left);  // 0x000F -> Motor khóa trục & mở phanh!
HAL_Delay(50);

// Đặt tốc độ chạy 150 RPM
Kinco_SetVelocityRPM(&motor_left, 150.0f);

// Đọc phản hồi thực tế liên tục:
// motor_left.actual_speed_rpm;  // Vận tốc RPM thực tế
// motor_left.actual_pos_deg;    // Vị trí góc thực tế
```

---

## 📄 Bản quyền (License)

Dự án được phát hành theo giấy phép [MIT License](LICENSE).

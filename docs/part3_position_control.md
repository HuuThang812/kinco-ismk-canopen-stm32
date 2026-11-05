# Part 3: Dieu Khien Goc Quay va Vi Tri (kinco_ismk_pos.c)

Tài liệu này hướng dẫn chi tiết phương pháp điều khiển góc quay và tọa độ vị trí cho động cơ servo Kinco iSMK theo chế độ Profile Position Mode (CiA 402 Mode 1).

---

## 1. Nguyen Ly Hoat Dong Cua Che Do Vi Tri (Mode 1)

Trong chế độ Profile Position Mode:
- Driver nhận tọa độ mục tiêu (Target Position), sau đó tự động tính toán đường cong chuyển động hình thang (Trapezoidal Profile) hoặc hình chữ S (S-curve) dựa trên các thông số: Vận tốc tối đa (Profile Velocity), Gia tốc tăng tốc (Profile Acceleration) và Gia tốc giảm tốc (Profile Deceleration).
- Động cơ tự động tăng tốc, quay với vận tốc ổn định, giảm tốc và dừng chính xác tại tọa độ đặt mà không yêu cầu vi điều khiển phải băm xung hay tính toán liên tục.

---

## 2. Cong Thuc Quy Doi Toan Hoc

Tất cả các công thức được trích xuất từ tài liệu chính hãng Kinco iSMK Integrated Servo User Manual (Phụ lục II):

### 2.1. Quy đổi từ Góc quay (Do) sang Xung Encoder (inc)
Động cơ Kinco iSMK trang bị encoder 16-bit với độ phân giải mặc định là 65536 xung trên 1 vòng quay (360 do):

$$\text{So xung (inc)} = \frac{\text{Goc quay (do)} \times 65536}{360.0}$$

- Quay 90 do:  `90.0 * 65536 / 360.0 = 16384 inc`
- Quay 180 do: `180.0 * 65536 / 360.0 = 32768 inc`
- Quay -45 do: `-45.0 * 65536 / 360.0 = -8192 inc`

### 2.2. Quy đổi từ Van toc (RPM) sang Don vi noi bo (DEC)
Vận tốc tối đa khi xoay góc được cài đặt vào thanh ghi Profile Velocity (0x6081) dưới dạng đơn vị số nguyên DEC:

$$\text{DEC} = \frac{\text{RPM} \times 512 \times 65536}{1875} \approx \text{RPM} \times 17895$$

---

## 3. Co Che Kich Phat Lenh Chuyen Dong (New Set-point Trigger)

Trong tiêu chuẩn CiA 402, việc ghi giá trị vào thanh ghi Target Position (0x607A) chưa làm động cơ quay ngay. Để động cơ bắt đầu chuyển động, vi điều khiển phải tạo một sườn xung từ 0 lên 1 tại Bit 4 của thanh ghi Controlword (0x6040):

1. **Buoc 1 (Chuan bi):** Ghi Controlword = `0x002F` (Bit 4 = 0).
2. **Buoc 2 (Kich hoat):** Ghi Controlword = `0x003F` (Bit 4 = 1).
3. Ngay khi phát hiện sườn xung 0 -> 1, driver chốt tọa độ đích và bắt đầu điều khiển động cơ quay. Khi đã quay đến góc đích, Bit 10 của thanh ghi Statusword (0x6041) sẽ tự động bật lên 1 (Target Reached).

Đối với chế độ quay tương đối (Relative Move - quay thêm một góc so với vị trí hiện tại), Bit 6 của Controlword được bật lên 1: Chuyển đổi từ `0x004F` -> `0x005F`.

---

## 4. Danh Sach Cac Ham Dieu Khien Trong Thu Vien

### 4.1. Cai dat van toc va gia toc:
```c
// Cai dat van toc xoay toi da (don vi: RPM)
bool Kinco_SetProfileVelocity(Kinco_Motor_t *motor, float max_rpm);

// Cai dat gia toc va giam toc (don vi: rps/s)
bool Kinco_SetProfileAcceleration(Kinco_Motor_t *motor, float rps2);
bool Kinco_SetProfileDeceleration(Kinco_Motor_t *motor, float rps2);
```

### 4.2. Cai dat toa do va kich hoat:
```c
// Cai dat goc dich theo do (vi du: 90.0, 180.0, -45.0)
bool Kinco_SetTargetAngleDeg(Kinco_Motor_t *motor, float target_deg);

// Kich hoat lenh quay: is_relative = false (Tuyet doi), true (Tuong doi)
bool Kinco_TriggerPositionMove(Kinco_Motor_t *motor, bool is_relative);
```

### 4.3. Ham tron goi tien loi (All-in-One):
```c
// Thuc hien toan bo cac buoc tren trong mot lenh duy nhat
bool Kinco_MoveToAngle(Kinco_Motor_t *motor, float target_deg, float max_rpm);
```

---

## 5. Vi Du Thuc Te Trong main()

Đoạn mã mẫu điều khiển động cơ quay đến các góc độ khác nhau:

```c
#include "kinco_ismk.h"

Kinco_Motor_t motor;

void Test_Angle_Control(void) {
    // 1. Khoi tao va kich hoat dong co (Part 2)
    Kinco_Init(&motor, 65536, STM32_CAN_Send);
    Kinco_NMT_Start(&motor);
    HAL_Delay(10);

    Kinco_ClearFault(&motor);
    HAL_Delay(50);

    Kinco_Shutdown(&motor);
    HAL_Delay(20);
    Kinco_SwitchOn(&motor);
    HAL_Delay(20);
    Kinco_EnableOperation(&motor);
    HAL_Delay(50);

    // 2. Cach 1: Su dung ham tron goi MoveToAngle
    // Ra lenh quay den goc 90 do voi van toc toi da 120 RPM
    Kinco_MoveToAngle(&motor, 90.0f, 120.0f);
    HAL_Delay(2000); // Cho dong co quay den dich

    // 3. Cach 2: Ra lenh quay ve goc 0 do bang cac ham don le
    Kinco_SetProfileVelocity(&motor, 80.0f); // Van toc 80 RPM
    Kinco_SetTargetAngleDeg(&motor, 0.0f);   // Ve goc 0 do
    Kinco_TriggerPositionMove(&motor, false); // Kich hoat quay tuyet doi
}
```

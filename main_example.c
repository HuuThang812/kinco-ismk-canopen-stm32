/**
 * @file main_example.c
 * @brief Ví dụ mẫu tích hợp Kinco iSMK vào project STM32CubeIDE (HAL)
 */

#include "kinco_ismk.h"
#include <stdio.h>

/* ==================================================================== */
/*          1. HÀM TRUYỀN DỮ LIỆU CAN (GẮN VỚI STM32 HAL)              */
/* ==================================================================== */

/* Khai báo biến ngoại vi CAN từ STM32 HAL (Ví dụ: hcan hoặc hcan1) */
/* extern CAN_HandleTypeDef hcan1; */

/* Khởi tạo 2 motor cho xe robot (Bánh Trái và Bánh Phải) */
Kinco_Motor_t motor_left;
Kinco_Motor_t motor_right;

/**
 * @brief Callback gửi gói tin CAN qua STM32 HAL
 * @note Thay thế hcan1 bằng biến CAN thực tế trên chip của bạn (hcan, hcan1, hcan2)
 */
bool STM32_CAN_Send(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    /* Mẫu code chuẩn cho STM32 HAL (Bỏ comment khi đưa vào project thật):
    
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;

    tx_header.StdId = can_id;
    tx_header.ExtId = 0x00;
    tx_header.IDE   = CAN_ID_STD;         // Kinco dùng chuẩn Standard ID 11-bit
    tx_header.RTR   = CAN_RTR_DATA;
    tx_header.DLC   = dlc;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, (uint8_t*)data, &tx_mailbox) == HAL_OK) {
        return true;
    }
    return false;
    */
    
    return true; /* Giả lập trả về true */
}

/* ==================================================================== */
/*          2. HÀM NGẮT NHẬN CAN (XỬ LÝ FEEDBACK TỰ ĐỘNG)              */
/* ==================================================================== */

/**
 * @brief Hàm ngắt nhận CAN tự động gọi bởi HAL khi có gói tin mới
 */
/*
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
        uint32_t can_id = rx_header.StdId;

        // Bóc tách dữ liệu cho motor trái (Node ID = 1)
        Kinco_ProcessRxMessage(&motor_left, can_id, rx_data, rx_header.DLC);

        // Bóc tách dữ liệu cho motor phải (Node ID = 2)
        Kinco_ProcessRxMessage(&motor_right, can_id, rx_data, rx_header.DLC);
    }
}
*/

/* ==================================================================== */
/*          3. QUY TRÌNH KHỞI ĐỘNG VÀ ĐIỀU KHIỂN MOTOR                  */
/* ==================================================================== */

void Kinco_Setup_And_Run(void) {
    /* 1. Khởi tạo đối tượng motor */
    Kinco_Init(&motor_left,  1, 65536, STM32_CAN_Send);  // Node ID = 1
    Kinco_Init(&motor_right, 2, 65536, STM32_CAN_Send);  // Node ID = 2

    /* 2. Đánh thức mạng CAN (Start Remote Node) */
    Kinco_NMT_Start(&motor_left);
    /* HAL_Delay(10); */

    /* 3. Xóa cờ lỗi nếu có */
    Kinco_ClearFault(&motor_left);
    /* HAL_Delay(50); */

    /* 4. Chọn chế độ chạy: Ví dụ Mode Vận tốc (RPM) cho xe AGV */
    Kinco_SetMode(&motor_left, KINCO_MODE_PROFILE_VELOCITY);
    /* HAL_Delay(20); */

    /* 5. Cài gia tốc / giảm tốc (ví dụ 50 rps/s) */
    Kinco_SetProfileAcceleration(&motor_left, 50.0f);
    Kinco_SetProfileDeceleration(&motor_left, 50.0f);
    /* HAL_Delay(20); */

    /* 6. Chuỗi Enable theo chuẩn CiA 402 */
    Kinco_Shutdown(&motor_left);         // Gửi 0x0006
    /* HAL_Delay(20); */
    Kinco_SwitchOn(&motor_left);         // Gửi 0x0007
    /* HAL_Delay(20); */
    Kinco_EnableOperation(&motor_left);  // Gửi 0x000F -> Khóa trục, mở phanh!
    /* HAL_Delay(50); */

    /* 7. Bơm tốc độ chạy thử: 150 RPM */
    Kinco_SetVelocityRPM(&motor_left, 150.0f);
}

/* ==================================================================== */
/*          4. VÍ DỤ CHUYỂN SANG ĐIỀU KHIỂN GÓC (POSITION MODE)         */
/* ==================================================================== */

void Kinco_Example_Position_Control(void) {
    /* Chuyển sang Mode 1: Profile Position */
    Kinco_SetMode(&motor_left, KINCO_MODE_PROFILE_POSITION);
    /* HAL_Delay(20); */

    /* Đặt tốc độ tối đa khi xoay góc: 200 RPM */
    Kinco_SetProfileVelocity(&motor_left, 200.0f);

    /* Đặt góc đích: Quay đúng 90.0 độ */
    Kinco_SetTargetAngleDeg(&motor_left, 90.0f);

    /* Kích hoạt chuyển động tuyệt đối (Absolute Move) */
    Kinco_TriggerPositionMove(&motor_left, false);
}

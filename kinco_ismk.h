/**
 * @file kinco_ismk.h
 * @brief Khai báo các hàm điều khiển cốt lõi cho Kinco iSMK CANopen (Dành cho 1 Motor - Node ID 1)
 */

#ifndef KINCO_ISMK_H
#define KINCO_ISMK_H

#include "kinco_ismk_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 1. Khởi tạo đối tượng motor (Tự động chọn Mode Góc & Cố định ID 1) */
void Kinco_Init(Kinco_Motor_t *motor, uint32_t encoder_resolution,
                bool (*can_send_callback)(uint32_t can_id, const uint8_t *data, uint8_t dlc));

/* 2. Đánh thức motor qua mạng CANopen (Lệnh NMT từ STM32 -> Motor) */
bool Kinco_NMT_Start(Kinco_Motor_t *motor);
bool Kinco_NMT_Reset(Kinco_Motor_t *motor);

/* 3. Hàm gửi Controlword */
bool Kinco_SendControlword(Kinco_Motor_t *motor, uint16_t controlword);

/* 4. Chuỗi máy trạng thái CiA 402 (Enable / Disable / Dừng / Xóa lỗi) */
bool Kinco_ClearFault(Kinco_Motor_t *motor);        /* Xóa cờ lỗi (0x0086) */
bool Kinco_Shutdown(Kinco_Motor_t *motor);          /* Về trạng thái sẵn sàng (0x0006) */
bool Kinco_SwitchOn(Kinco_Motor_t *motor);          /* Đóng mạch động lực (0x0007) */
bool Kinco_EnableOperation(Kinco_Motor_t *motor);   /* Khóa cứng trục & mở phanh (0x000F) */
bool Kinco_DisableOperation(Kinco_Motor_t *motor);  /* Thả lỏng trục tự do (0x0006) */
bool Kinco_QuickStop(Kinco_Motor_t *motor);         /* Dừng khẩn cấp (0x0002) */

/* Thiết lập chế độ chạy (Mặc định tự động chọn Mode 1: Góc / Position) */
bool Kinco_SetMode(Kinco_Motor_t *motor, Kinco_Mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* KINCO_ISMK_H */

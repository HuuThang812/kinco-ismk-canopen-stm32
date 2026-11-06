/**
 * @file kinco_ismk.h
 * @brief Khai báo các hàm điều khiển cho Kinco iSMK CANopen (Dành cho 1 Motor - Node ID 1)
 */

#ifndef KINCO_ISMK_H
#define KINCO_ISMK_H

#include "kinco_ismk_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================== */
/* 1. KHOI TAO & DANG KY HAM GUI CAN                                    */
/* ==================================================================== */
void Kinco_Init(Kinco_Motor_t *motor, uint32_t encoder_resolution,
                bool (*can_send_callback)(uint32_t can_id, const uint8_t *data, uint8_t dlc));

/* ==================================================================== */
/* 2. QUAN LY MANG CANOPEN (NMT)                                        */
/* ==================================================================== */
bool Kinco_NMT_Start(Kinco_Motor_t *motor);
bool Kinco_NMT_Reset(Kinco_Motor_t *motor);

/* ==================================================================== */
/* 3. MAY TRANG THAI CIA 402                                            */
/* ==================================================================== */
bool Kinco_SendControlword(Kinco_Motor_t *motor, uint16_t controlword);
bool Kinco_ClearFault(Kinco_Motor_t *motor);        /* Xóa cờ lỗi (0x0086) */
bool Kinco_Shutdown(Kinco_Motor_t *motor);          /* Về trạng thái sẵn sàng (0x0006) */
bool Kinco_SwitchOn(Kinco_Motor_t *motor);          /* Đóng mạch động lực (0x0007) */
bool Kinco_EnableOperation(Kinco_Motor_t *motor);   /* Khóa cứng trục & mở phanh (0x000F) */
bool Kinco_DisableOperation(Kinco_Motor_t *motor);  /* Thả lỏng trục tự do (0x0006) */
bool Kinco_QuickStop(Kinco_Motor_t *motor);         /* Dừng khẩn cấp (0x0002) */

bool Kinco_SetMode(Kinco_Motor_t *motor, Kinco_Mode_t mode);

/* ==================================================================== */
/* 4. DIEU KHIEN GOC QUAY & VI TRI (PART 3)                            */
/* ==================================================================== */

/* Cài đặt vận tốc tối đa khi xoay góc (đơn vị: RPM) */
bool Kinco_SetProfileVelocity(Kinco_Motor_t *motor, float max_rpm);

/* Cài đặt gia tốc và giảm tốc (đơn vị: rps/s) */
bool Kinco_SetProfileAcceleration(Kinco_Motor_t *motor, float rps2);
bool Kinco_SetProfileDeceleration(Kinco_Motor_t *motor, float rps2);

/* Cài đặt góc đích theo số xung encoder hoặc theo độ */
bool Kinco_SetTargetPositionInc(Kinco_Motor_t *motor, int32_t target_inc);
bool Kinco_SetTargetAngleDeg(Kinco_Motor_t *motor, float target_deg);

/* Kích hoạt lệnh chuyển động góc (Absolute hoặc Relative) */
bool Kinco_TriggerPositionMove(Kinco_Motor_t *motor, bool is_relative);

/* Hàm trọn gói: Nạp góc + Vận tốc tối đa -> Kích hoạt quay ngay */
bool Kinco_MoveToAngle(Kinco_Motor_t *motor, float target_deg, float max_rpm);

/* ==================================================================== */
/* 5. CONG THUC QUY DOI DON VI TOAN HOC                                 */
/* ==================================================================== */
int32_t Kinco_Deg_To_Inc(float degrees, uint32_t resolution);
float   Kinco_Inc_To_Deg(int32_t inc, uint32_t resolution);
int32_t Kinco_RPM_To_DEC(float rpm, uint32_t resolution);

/* ==================================================================== */
/* 6. BO GIAI MA GOI TIN PHAN HOI RX (PART 4)                          */
/* ==================================================================== */

/**
 * @brief Xu ly va giai ma goi tin CAN nhan duoc tu Motor Kinco
 * @param motor Con tro toi doi tuong motor
 * @param can_id CAN ID cua goi tin nhan duoc (StdId)
 * @param data Con tro mang du lieu nhan duoc
 * @param dlc Do dai du lieu (so byte)
 * @return true neu goi tin dung dinh danh va duoc giai ma thanh cong
 */
bool Kinco_ProcessRxMessage(Kinco_Motor_t *motor, uint32_t can_id, const uint8_t *data, uint8_t dlc);

/**
 * @brief Giai ma trang thai CiA 402 tu thanh ghi Statusword (0x6041)
 * @param status_word Gia tri 16-bit cua Statusword
 * @return Trang thai may Kinco_State_t tuong ung
 */
Kinco_State_t Kinco_DecodeState(uint16_t status_word);

#ifdef __cplusplus
}
#endif

#endif /* KINCO_ISMK_H */
/**
 * @file kinco_ismk_pos.c
 * @brief Triển khai các hàm điều khiển Góc quay và Vị trí (CiA 402 Profile Position Mode)
 */

#include "kinco_ismk.h"

/* ==================================================================== */
/* 1. CONG THUC QUY DOI TOAN HOC                                        */
/* ==================================================================== */

int32_t Kinco_Deg_To_Inc(float degrees, uint32_t resolution) {
    if (resolution == 0) resolution = 65536;
    double inc = ((double)degrees * (double)resolution) / 360.0;
    return (int32_t)inc;
}

float Kinco_Inc_To_Deg(int32_t inc, uint32_t resolution) {
    if (resolution == 0) resolution = 65536;
    double deg = ((double)inc * 360.0) / (double)resolution;
    return (float)deg;
}

int32_t Kinco_RPM_To_DEC(float rpm, uint32_t resolution) {
    if (resolution == 0) resolution = 65536;
    /* Cong thuc tu Manual Kinco iSMK Appendix II:
     * DEC = [(RPM * 512 * resolution) / 1875]
     */
    double dec = ((double)rpm * 512.0 * (double)resolution) / 1875.0;
    return (int32_t)dec;
}

/* ==================================================================== */
/* 2. HAM CAI DAT THAM SO CHUYEN DONG GOC                               */
/* ==================================================================== */

static bool Kinco_SDO_Write32(Kinco_Motor_t *motor, uint16_t index, uint8_t subindex, uint32_t val) {
    if (!motor || !motor->can_send) return false;

    uint8_t payload[8];
    payload[0] = 0x23;                            /* Lenh SDO ghi 4 byte */
    payload[1] = (uint8_t)(index & 0xFF);
    payload[2] = (uint8_t)((index >> 8) & 0xFF);
    payload[3] = subindex;
    payload[4] = (uint8_t)(val & 0xFF);
    payload[5] = (uint8_t)((val >> 8) & 0xFF);
    payload[6] = (uint8_t)((val >> 16) & 0xFF);
    payload[7] = (uint8_t)((val >> 24) & 0xFF);

    return motor->can_send(CANOPEN_COB_SDO_RX, payload, 8);
}

bool Kinco_SetProfileVelocity(Kinco_Motor_t *motor, float max_rpm) {
    if (!motor) return false;
    int32_t dec_speed = Kinco_RPM_To_DEC(max_rpm, motor->encoder_resolution);
    return Kinco_SDO_Write32(motor, KINCO_OBJ_PROFILE_VELOCITY, 0x00, (uint32_t)dec_speed);
}

bool Kinco_SetProfileAcceleration(Kinco_Motor_t *motor, float rps2) {
    if (!motor) return false;
    /* DEC = [(rps2 * 65536 * resolution) / 4000000] */
    uint32_t dec = (uint32_t)(((double)rps2 * 65536.0 * (double)motor->encoder_resolution) / 4000000.0);
    return Kinco_SDO_Write32(motor, KINCO_OBJ_PROFILE_ACCEL, 0x00, dec);
}

bool Kinco_SetProfileDeceleration(Kinco_Motor_t *motor, float rps2) {
    if (!motor) return false;
    uint32_t dec = (uint32_t)(((double)rps2 * 65536.0 * (double)motor->encoder_resolution) / 4000000.0);
    return Kinco_SDO_Write32(motor, KINCO_OBJ_PROFILE_DECEL, 0x00, dec);
}

bool Kinco_SetTargetPositionInc(Kinco_Motor_t *motor, int32_t target_inc) {
    return Kinco_SDO_Write32(motor, KINCO_OBJ_TARGET_POSITION, 0x00, (uint32_t)target_inc);
}

bool Kinco_SetTargetAngleDeg(Kinco_Motor_t *motor, float target_deg) {
    if (!motor) return false;
    int32_t target_inc = Kinco_Deg_To_Inc(target_deg, motor->encoder_resolution);
    return Kinco_SetTargetPositionInc(motor, target_inc);
}

/* ==================================================================== */
/* 3. KICH HOAT LENH CHAY GOC (TRIGGER POSITION MOVE)                  */
/* ==================================================================== */

bool Kinco_TriggerPositionMove(Kinco_Motor_t *motor, bool is_relative) {
    if (!motor) return false;

    /* Theo chuan CiA 402:
     * - Che do Tuyet doi (Absolute): Giao dong Bit 4 tu 0 len 1 (0x002F -> 0x003F)
     * - Che do Tuong doi (Relative): Bit 6 = 1 (0x004F -> 0x005F)
     */
    uint16_t prep_cw = is_relative ? 0x004F : 0x002F;
    uint16_t trig_cw = is_relative ? 0x005F : 0x003F;

    if (!Kinco_SendControlword(motor, prep_cw)) return false;
    return Kinco_SendControlword(motor, trig_cw);
}

bool Kinco_MoveToAngle(Kinco_Motor_t *motor, float target_deg, float max_rpm) {
    if (!motor) return false;

    /* 1. Cai dat van toc xoay toi da */
    if (!Kinco_SetProfileVelocity(motor, max_rpm)) return false;

    /* 2. Cai dat goc quay muc tieu */
    if (!Kinco_SetTargetAngleDeg(motor, target_deg)) return false;

    /* 3. Kich phat lenh quay tuyet doi */
    return Kinco_TriggerPositionMove(motor, false);
}

/**
 * @file kinco_ismk.c
 * @brief Triển khai driver CANopen CiA 402 cho Kinco iSMK
 */

#include "kinco_ismk.h"
#include <string.h>

/* ==================================================================== */
/*                   INTERNAL SDO HELPER FUNCTIONS                      */
/* ==================================================================== */

static bool Kinco_SDO_Write(Kinco_Motor_t *motor, uint8_t cmd, uint16_t index, uint8_t subindex, uint32_t val) {
    if (!motor || !motor->can_send) return false;

    uint8_t payload[8];
    payload[0] = cmd;
    payload[1] = (uint8_t)(index & 0xFF);
    payload[2] = (uint8_t)((index >> 8) & 0xFF);
    payload[3] = subindex;
    payload[4] = (uint8_t)(val & 0xFF);
    payload[5] = (uint8_t)((val >> 8) & 0xFF);
    payload[6] = (uint8_t)((val >> 16) & 0xFF);
    payload[7] = (uint8_t)((val >> 24) & 0xFF);

    uint32_t sdo_rx_id = CANOPEN_COB_SDO_RX_BASE + motor->node_id;
    return motor->can_send(sdo_rx_id, payload, 8);
}

static bool Kinco_SDO_Write8(Kinco_Motor_t *motor, uint16_t index, uint8_t subindex, uint8_t val) {
    return Kinco_SDO_Write(motor, 0x2F, index, subindex, (uint32_t)val);
}

static bool Kinco_SDO_Write16(Kinco_Motor_t *motor, uint16_t index, uint8_t subindex, uint16_t val) {
    return Kinco_SDO_Write(motor, 0x2B, index, subindex, (uint32_t)val);
}

static bool Kinco_SDO_Write32(Kinco_Motor_t *motor, uint16_t index, uint8_t subindex, uint32_t val) {
    return Kinco_SDO_Write(motor, 0x23, index, subindex, val);
}

/* ==================================================================== */
/*                       STATE DECODER HELPER                           */
/* ==================================================================== */

static Kinco_State_t Kinco_DecodeState(uint16_t sw) {
    /* Tham khảo bảng trạng thái CiA 402 */
    if ((sw & 0x004F) == 0x0000) return KINCO_STATE_NOT_READY_TO_SWITCH_ON;
    if ((sw & 0x004F) == 0x0040) return KINCO_STATE_SWITCH_ON_DISABLED;
    if ((sw & 0x006F) == 0x0021) return KINCO_STATE_READY_TO_SWITCH_ON;
    if ((sw & 0x006F) == 0x0023) return KINCO_STATE_SWITCHED_ON;
    if ((sw & 0x006F) == 0x0027) return KINCO_STATE_OPERATION_ENABLED;
    if ((sw & 0x006F) == 0x0007) return KINCO_STATE_QUICK_STOP_ACTIVE;
    if ((sw & 0x004F) == 0x000F) return KINCO_STATE_FAULT_REACTION_ACTIVE;
    if ((sw & 0x004F) == 0x0008) return KINCO_STATE_FAULT;
    return KINCO_STATE_UNKNOWN;
}

/* ==================================================================== */
/*                       PUBLIC API IMPLEMENTATION                      */
/* ==================================================================== */

void Kinco_Init(Kinco_Motor_t *motor, uint8_t node_id, uint32_t encoder_resolution,
                bool (*can_send_callback)(uint32_t can_id, const uint8_t *data, uint8_t dlc)) {
    if (!motor) return;
    memset(motor, 0, sizeof(Kinco_Motor_t));

    motor->node_id = node_id;
    motor->encoder_resolution = (encoder_resolution > 0) ? encoder_resolution : 65536;
    motor->can_send = can_send_callback;
    motor->state = KINCO_STATE_UNKNOWN;
    motor->target_mode = KINCO_MODE_NONE;
}

bool Kinco_NMT_Start(Kinco_Motor_t *motor) {
    if (!motor || !motor->can_send) return false;
    /* NMT Module Control: 0x01 là lệnh Start Remote Node */
    uint8_t payload[2] = {0x01, motor->node_id};
    return motor->can_send(CANOPEN_COB_NMT, payload, 2);
}

bool Kinco_NMT_Reset(Kinco_Motor_t *motor) {
    if (!motor || !motor->can_send) return false;
    /* NMT Reset Node: 0x81 */
    uint8_t payload[2] = {0x81, motor->node_id};
    return motor->can_send(CANOPEN_COB_NMT, payload, 2);
}

bool Kinco_SendControlword(Kinco_Motor_t *motor, uint16_t controlword) {
    return Kinco_SDO_Write16(motor, KINCO_OBJ_CONTROLWORD, 0x00, controlword);
}

bool Kinco_ClearFault(Kinco_Motor_t *motor) {
    /* Gửi 0x0086 (Bit 7: Fault Reset) */
    return Kinco_SendControlword(motor, 0x0086);
}

bool Kinco_Shutdown(Kinco_Motor_t *motor) {
    /* Chuyển sang Ready to Switch On */
    return Kinco_SendControlword(motor, 0x0006);
}

bool Kinco_SwitchOn(Kinco_Motor_t *motor) {
    /* Chuyển sang Switched On */
    return Kinco_SendControlword(motor, 0x0007);
}

bool Kinco_EnableOperation(Kinco_Motor_t *motor) {
    /* Kích hoạt motor chạy - Khóa trục, mở phanh cơ */
    return Kinco_SendControlword(motor, 0x000F);
}

bool Kinco_DisableOperation(Kinco_Motor_t *motor) {
    /* Tắt chế độ hoạt động, thả lỏng trục */
    return Kinco_SendControlword(motor, 0x0006);
}

bool Kinco_QuickStop(Kinco_Motor_t *motor) {
    /* Dừng khẩn cấp */
    return Kinco_SendControlword(motor, 0x0002);
}

bool Kinco_SetMode(Kinco_Motor_t *motor, Kinco_Mode_t mode) {
    if (!motor) return false;
    motor->target_mode = mode;
    return Kinco_SDO_Write8(motor, KINCO_OBJ_MODES_OF_OPER, 0x00, (uint8_t)mode);
}

/* ==================================================================== */
/*                   VELOCITY CONTROL (RPM)                             */
/* ==================================================================== */

bool Kinco_SetVelocityRPM(Kinco_Motor_t *motor, float rpm) {
    if (!motor) return false;
    int32_t dec_speed = Kinco_RPM_To_DEC(rpm, motor->encoder_resolution);
    return Kinco_SDO_Write32(motor, KINCO_OBJ_TARGET_VELOCITY, 0x00, (uint32_t)dec_speed);
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

/* ==================================================================== */
/*                   POSITION / ANGLE CONTROL                           */
/* ==================================================================== */

bool Kinco_SetProfileVelocity(Kinco_Motor_t *motor, float max_rpm) {
    if (!motor) return false;
    int32_t dec = Kinco_RPM_To_DEC(max_rpm, motor->encoder_resolution);
    return Kinco_SDO_Write32(motor, KINCO_OBJ_PROFILE_VELOCITY, 0x00, (uint32_t)dec);
}

bool Kinco_SetTargetPositionInc(Kinco_Motor_t *motor, int32_t target_inc) {
    return Kinco_SDO_Write32(motor, KINCO_OBJ_TARGET_POSITION, 0x00, (uint32_t)target_inc);
}

bool Kinco_SetTargetAngleDeg(Kinco_Motor_t *motor, float target_deg) {
    if (!motor) return false;
    int32_t inc = Kinco_Deg_To_Inc(target_deg, motor->encoder_resolution);
    return Kinco_SetTargetPositionInc(motor, inc);
}

bool Kinco_TriggerPositionMove(Kinco_Motor_t *motor, bool is_relative) {
    if (!motor) return false;
    /* 
     * Kích hoạt chạy vị trí theo CiA 402:
     * - Chế độ Tuyệt đối (Absolute): Gửi 0x2F rồi bật sườn lên 0x3F (Bit 4: New setpoint)
     * - Chế độ Tương đối (Relative): Bit 6 = 1 (0x4F rồi bật lên 0x5F)
     */
    uint16_t prep_cw = is_relative ? 0x004F : 0x002F;
    uint16_t trig_cw = is_relative ? 0x005F : 0x003F;

    Kinco_SendControlword(motor, prep_cw);
    return Kinco_SendControlword(motor, trig_cw);
}

/* ==================================================================== */
/*                   TORQUE / FORCE CONTROL                             */
/* ==================================================================== */

bool Kinco_SetTargetTorquePercent(Kinco_Motor_t *motor, float torque_pct) {
    if (!motor) return false;
    /* Thanh ghi 0x6071 nhận đơn vị 0.1% mô-men định mức (VD: 100.0% = 1000) */
    int16_t raw_val = (int16_t)(torque_pct * 10.0f);
    return Kinco_SDO_Write16(motor, KINCO_OBJ_TARGET_TORQUE, 0x00, (uint16_t)raw_val);
}

bool Kinco_SetMaxSpeedLimitInTorqueMode(Kinco_Motor_t *motor, float max_rpm) {
    if (!motor) return false;
    int32_t dec = Kinco_RPM_To_DEC(max_rpm, motor->encoder_resolution);
    return Kinco_SDO_Write16(motor, KINCO_OBJ_MAX_VELOCITY, 0x00, (uint16_t)dec);
}

/* ==================================================================== */
/*                   CAN RX MESSAGE PROCESSING                          */
/* ==================================================================== */

void Kinco_ProcessRxMessage(Kinco_Motor_t *motor, uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (!motor || !data || dlc == 0) return;

    /* 1. Gói TPDO1 (CAN ID: 0x180 + Node_ID) -> Statusword & Actual Position */
    if (can_id == (CANOPEN_COB_TPDO1_BASE + motor->node_id)) {
        if (dlc >= 2) {
            motor->status_word = (uint16_t)(data[0] | (data[1] << 8));
            motor->state = Kinco_DecodeState(motor->status_word);
            motor->fault_active = (motor->status_word & (1 << 3)) != 0;
            motor->target_reached = (motor->status_word & (1 << 10)) != 0;
            motor->is_connected = true;
        }
        if (dlc >= 6) {
            motor->actual_pos_inc = (int32_t)(data[2] | (data[3] << 8) | (data[4] << 16) | (data[5] << 24));
            motor->actual_pos_deg = Kinco_Inc_To_Deg(motor->actual_pos_inc, motor->encoder_resolution);
            motor->actual_pos_rev = (float)motor->actual_pos_inc / (float)motor->encoder_resolution;
        }
    }
    /* 2. Gói TPDO2 (CAN ID: 0x280 + Node_ID) -> Actual Speed & Torque/Current */
    else if (can_id == (CANOPEN_COB_TPDO2_BASE + motor->node_id)) {
        if (dlc >= 4) {
            motor->actual_speed_dec = (int32_t)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
            motor->actual_speed_rpm = Kinco_DEC_To_RPM(motor->actual_speed_dec, motor->encoder_resolution);
            motor->is_connected = true;
        }
        if (dlc >= 6) {
            motor->actual_torque_raw = (int16_t)(data[4] | (data[5] << 8));
            motor->actual_torque_pct = (float)motor->actual_torque_raw / 10.0f;
        }
    }
    /* 3. Gói SDO Response (CAN ID: 0x580 + Node_ID) */
    else if (can_id == (CANOPEN_COB_SDO_TX_BASE + motor->node_id)) {
        motor->is_connected = true;
        /* Có thể mở rộng để parse các giá trị đọc qua SDO */
    }
    /* 4. Gói Báo lỗi khẩn cấp Emergency (EMCY: 0x080 + Node_ID) */
    else if (can_id == (CANOPEN_COB_EMCY_BASE + motor->node_id)) {
        motor->fault_active = true;
        if (dlc >= 2) {
            motor->emergency_code = (uint16_t)(data[0] | (data[1] << 8));
        }
    }
    /* 5. Gói Heartbeat / Bootup (CAN ID: 0x700 + Node_ID) */
    else if (can_id == (CANOPEN_COB_HEARTBEAT_BASE + motor->node_id)) {
        motor->is_connected = true;
    }
}

/* ==================================================================== */
/*                   UNIT CONVERSION FORMULAS                           */
/* ==================================================================== */

int32_t Kinco_RPM_To_DEC(float rpm, uint32_t resolution) {
    /* Công thức trích từ iSMK Manual Appendix II:
     * DEC = [(RPM * 512 * resolution) / 1875]
     */
    double dec = ((double)rpm * 512.0 * (double)resolution) / 1875.0;
    return (int32_t)dec;
}

float Kinco_DEC_To_RPM(int32_t dec, uint32_t resolution) {
    if (resolution == 0) return 0.0f;
    double rpm = ((double)dec * 1875.0) / (512.0 * (double)resolution);
    return (float)rpm;
}

int32_t Kinco_Deg_To_Inc(float degrees, uint32_t resolution) {
    double inc = ((double)degrees * (double)resolution) / 360.0;
    return (int32_t)inc;
}

float Kinco_Inc_To_Deg(int32_t inc, uint32_t resolution) {
    if (resolution == 0) return 0.0f;
    double deg = ((double)inc * 360.0) / (double)resolution;
    return (float)deg;
}

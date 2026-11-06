/**
 * @file kinco_ismk_rx.c
 * @brief Bo giai ma cac goi tin phan hoi CANopen (TPDO1, TPDO2, EMCY, Heartbeat, SDO Tx)
 *        danh cho 1 Motor Kinco iSMK (Node ID = 1)
 */

#include "kinco_ismk.h"

/* ==================================================================== */
/* 1. GIAI MA MAY TRANG THAI CIA 402 TU STATUSWORD (0x6041)             */
/* ==================================================================== */

Kinco_State_t Kinco_DecodeState(uint16_t status_word) {
    /* Bang doi chieu bitmask theo chuan CiA 402 & Kinco Manual Table 5-2:
     * - Bit 0: Ready to switch on
     * - Bit 1: Switched on
     * - Bit 2: Operation enabled
     * - Bit 3: Fault
     * - Bit 5: Quick stop (0: Active, 1: Inactive)
     * - Bit 6: Switch on disabled
     */

    /* Kiem tra trang thai Fault truoc */
    if ((status_word & 0x004F) == 0x0008) {
        return KINCO_STATE_FAULT;
    }
    if ((status_word & 0x004F) == 0x000F) {
        return KINCO_STATE_FAULT_REACTION_ACTIVE;
    }

    /* Kiem tra Switch on disabled */
    if ((status_word & 0x004F) == 0x0040) {
        return KINCO_STATE_SWITCH_ON_DISABLED;
    }

    /* Kiem tra Quick Stop Active */
    if ((status_word & 0x006F) == 0x0007) {
        return KINCO_STATE_QUICK_STOP_ACTIVE;
    }

    /* Kiem tra Operation Enabled (Truc khoa cung & mo phanh) */
    if ((status_word & 0x006F) == 0x0027) {
        return KINCO_STATE_OPERATION_ENABLED;
    }

    /* Kiem tra Switched On */
    if ((status_word & 0x006F) == 0x0023) {
        return KINCO_STATE_SWITCHED_ON;
    }

    /* Kiem tra Ready to switch on */
    if ((status_word & 0x006F) == 0x0021) {
        return KINCO_STATE_READY_TO_SWITCH_ON;
    }

    /* Not ready to switch on */
    if ((status_word & 0x004F) == 0x0000) {
        return KINCO_STATE_NOT_READY_TO_SWITCH_ON;
    }

    return KINCO_STATE_UNKNOWN;
}

/* ==================================================================== */
/* 2. HAM XU LY VA GIAI MA GOI TIN CAN RX                               */
/* ==================================================================== */

bool Kinco_ProcessRxMessage(Kinco_Motor_t *motor, uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (!motor || !data || dlc == 0) return false;

    switch (can_id) {

        /* ============================================================ */
        /* GÓI 1: TPDO1 (0x181) - Vi tri thuc te encoder & Statusword   */
        /* Cau hinh xuat xuong iSMK (Figure 9-4 & 9-5):                */
        /* - Byte 0..3: Actual Position 0x6063 (int32, Little-Endian)   */
        /* - Byte 4..5: Statusword 0x6041 (uint16, Little-Endian)       */
        /* ============================================================ */
        case CANOPEN_COB_TPDO1: {
            if (dlc < 6) return false;

            /* 1. Boc tach vi tri thuc te (int32) */
            int32_t raw_pos = (int32_t)(
                (uint32_t)data[0] |
                ((uint32_t)data[1] << 8) |
                ((uint32_t)data[2] << 16) |
                ((uint32_t)data[3] << 24)
            );
            motor->actual_pos_inc = raw_pos;
            motor->actual_pos_deg = Kinco_Inc_To_Deg(raw_pos, motor->encoder_resolution);
            motor->actual_pos_rev = (motor->encoder_resolution > 0) ? 
                                    ((float)raw_pos / (float)motor->encoder_resolution) : 0.0f;

            /* 2. Boc tach Statusword (uint16) */
            uint16_t sw = (uint16_t)(data[4] | (data[5] << 8));
            motor->status_word = sw;

            /* 3. Cap nhat trang thai va cac co quan trong */
            motor->state = Kinco_DecodeState(sw);
            motor->is_connected = true;

            /* Bit 3: Fault active (1 = dang co loi) */
            motor->fault_active = (sw & (1 << 3)) ? true : false;

            /* Bit 10: Target Reached (1 = da hãm dung tai goc dich) */
            motor->target_reached = (sw & (1 << 10)) ? true : false;

            return true;
        }

        /* ============================================================ */
        /* GÓI 2: TPDO2 (0x281) - Van toc thuc te & Mo-men / Dong dien  */
        /* - Byte 0..3: Actual Speed 0x606C (int32 DEC, Little-Endian)  */
        /* - Byte 4..5: Actual Torque 0x6077 (int16, 0.1% rated torque) */
        /* ============================================================ */
        case CANOPEN_COB_TPDO2: {
            if (dlc >= 4) {
                int32_t raw_speed = (int32_t)(
                    (uint32_t)data[0] |
                    ((uint32_t)data[1] << 8) |
                    ((uint32_t)data[2] << 16) |
                    ((uint32_t)data[3] << 24)
                );
                motor->actual_speed_dec = raw_speed;
                /* Quy doi nguoc tu DEC sang RPM theo cong thuc Appendix II:
                 * RPM = (DEC * 1875) / (512 * resolution)
                 */
                if (motor->encoder_resolution > 0) {
                    double rpm = ((double)raw_speed * 1875.0) / (512.0 * (double)motor->encoder_resolution);
                    motor->actual_speed_rpm = (float)rpm;
                }
            }

            if (dlc >= 6) {
                int16_t raw_torque = (int16_t)(data[4] | (data[5] << 8));
                motor->actual_torque_raw = raw_torque;
                motor->actual_torque_pct = (float)raw_torque * 0.1f;
            }

            motor->is_connected = true;
            return true;
        }

        /* ============================================================ */
        /* GÓI 3: HEARTBEAT (0x701) - Diem danh song/chet tu Motor     */
        /* Byte 0: State (0x00 Boot-up, 0x04 Stopped, 0x05 Operational) */
        /* ============================================================ */
        case CANOPEN_COB_HEARTBEAT: {
            motor->is_connected = true;
            return true;
        }

        /* ============================================================ */
        /* GÓI 4: EMCY (0x081) - Coi bao dong loi khan cap tu Motor     */
        /* Byte 0..1: Emergency Error Code                             */
        /* Byte 2: Error Register                                       */
        /* ============================================================ */
        case CANOPEN_COB_EMCY: {
            if (dlc >= 2) {
                motor->emergency_code = (uint16_t)(data[0] | (data[1] << 8));
            }
            motor->fault_active = true;
            motor->state = KINCO_STATE_FAULT;
            motor->is_connected = true;
            return true;
        }

        /* ============================================================ */
        /* GÓI 5: SDO TX (0x581) - Phan hoi SDO tu Motor ve STM32       */
        /* Byte 0 = 0x60: Ghi thanh cong vao Object Dictionary          */
        /* Byte 0 = 0x80: SDO Abort (Loi truy cap thanh ghi)           */
        /* ============================================================ */
        case CANOPEN_COB_SDO_TX: {
            motor->is_connected = true;
            if (data[0] == 0x80) {
                /* Ghi nhan co loi SDO */
                motor->fault_active = true;
            }
            return true;
        }

        default:
            return false;
    }
}

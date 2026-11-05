/**
 * @file kinco_ismk_core.c
 * @brief Thực thi logic khởi tạo, NMT và máy trạng thái CiA 402 cho 1 Motor Kinco iSMK (Node ID = 1)
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
    payload[1] = (uint8_t)(index & 0xFF);         /* Byte thấp của địa chỉ thanh ghi */
    payload[2] = (uint8_t)((index >> 8) & 0xFF);  /* Byte cao của địa chỉ thanh ghi */
    payload[3] = subindex;                        /* Sub-index */
    payload[4] = (uint8_t)(val & 0xFF);           /* Dữ liệu Little-endian */
    payload[5] = (uint8_t)((val >> 8) & 0xFF);
    payload[6] = (uint8_t)((val >> 16) & 0xFF);
    payload[7] = (uint8_t)((val >> 24) & 0xFF);

    /* Gửi thẳng đến hòm thư SDO Rx của Motor 1 (CAN ID: 0x601) */
    return motor->can_send(CANOPEN_COB_SDO_RX, payload, 8);
}

static bool Kinco_SDO_Write8(Kinco_Motor_t *motor, uint16_t index, uint8_t subindex, uint8_t val) {
    return Kinco_SDO_Write(motor, 0x2F, index, subindex, (uint32_t)val);
}

static bool Kinco_SDO_Write16(Kinco_Motor_t *motor, uint16_t index, uint8_t subindex, uint16_t val) {
    return Kinco_SDO_Write(motor, 0x2B, index, subindex, (uint32_t)val);
}

/* ==================================================================== */
/* 1. KHỞI TẠO ĐỐI TƯỢNG MOTOR (AUTO CHỌN CHẾ ĐỘ GÓC / VỊ TRÍ)           */
/* ==================================================================== */

void Kinco_Init(Kinco_Motor_t *motor, uint32_t encoder_resolution,
                bool (*can_send_callback)(uint32_t can_id, const uint8_t *data, uint8_t dlc)) {
    if (!motor) return;
    memset(motor, 0, sizeof(Kinco_Motor_t));

    motor->encoder_resolution = (encoder_resolution > 0) ? encoder_resolution : 65536;
    motor->can_send = can_send_callback;
    motor->state = KINCO_STATE_UNKNOWN;

    /* Mặc định tự động chọn chế độ Vị trí / Góc (Profile Position Mode = 1) */
    motor->target_mode = KINCO_MODE_PROFILE_POSITION;
}

/* ==================================================================== */
/* 2. ĐÁNH THỨC MOTOR QUA MẠNG CANOPEN (NMT)                            */
/* ==================================================================== */

bool Kinco_NMT_Start(Kinco_Motor_t *motor) {
    if (!motor || !motor->can_send) return false;
    /* Gửi lệnh NMT 0x01 để đánh thức Motor 1 sang trạng thái Operational */
    uint8_t payload[2] = {0x01, KINCO_NODE_ID};
    return motor->can_send(CANOPEN_COB_NMT, payload, 2);
}

bool Kinco_NMT_Reset(Kinco_Motor_t *motor) {
    if (!motor || !motor->can_send) return false;
    /* Lệnh NMT 0x81: Khởi động lại vi xử lý của Motor 1 */
    uint8_t payload[2] = {0x81, KINCO_NODE_ID};
    return motor->can_send(CANOPEN_COB_NMT, payload, 2);
}

/* ==================================================================== */
/* 3. HÀM GỬI CONTROLWORD                                               */
/* ==================================================================== */

bool Kinco_SendControlword(Kinco_Motor_t *motor, uint16_t controlword) {
    return Kinco_SDO_Write16(motor, KINCO_OBJ_CONTROLWORD, 0x00, controlword);
}

/* ==================================================================== */
/* 4. CHUỖI MÁY TRẠNG THÁI CIA 402                                      */
/* ==================================================================== */

bool Kinco_ClearFault(Kinco_Motor_t *motor) {
    /* Gửi 0x0086 (Bit 7: Fault Reset) để xóa cờ lỗi */
    return Kinco_SendControlword(motor, 0x0086);
}

bool Kinco_Shutdown(Kinco_Motor_t *motor) {
    /* Chuyển sang trạng thái Ready to Switch On (0x0006) */
    return Kinco_SendControlword(motor, 0x0006);
}

bool Kinco_SwitchOn(Kinco_Motor_t *motor) {
    /* Đóng mạch động lực (0x0007) */
    return Kinco_SendControlword(motor, 0x0007);
}

bool Kinco_EnableOperation(Kinco_Motor_t *motor) {
    /* Kích hoạt motor: Khóa cứng trục, mở phanh cơ (0x000F) */
    return Kinco_SendControlword(motor, 0x000F);
}

bool Kinco_DisableOperation(Kinco_Motor_t *motor) {
    /* Thả lỏng trục tự do (0x0006) */
    return Kinco_SendControlword(motor, 0x0006);
}

bool Kinco_QuickStop(Kinco_Motor_t *motor) {
    /* Dừng khẩn cấp (0x0002) */
    return Kinco_SendControlword(motor, 0x0002);
}

/* ==================================================================== */
/* THIẾT LẬP CHẾ ĐỘ CHẠY                                                */
/* ==================================================================== */

bool Kinco_SetMode(Kinco_Motor_t *motor, Kinco_Mode_t mode) {
    if (!motor) return false;
    motor->target_mode = mode;
    return Kinco_SDO_Write8(motor, KINCO_OBJ_MODES_OF_OPER, 0x00, (uint8_t)mode);
}

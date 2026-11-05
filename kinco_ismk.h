/**
 * @file kinco_ismk.h
 * @brief Driver giao tiếp CANopen (CiA 402) cho Động cơ Servo Kinco iSMK
 * @note Tương thích với các dòng STM32 (F1, F4, G4, H7...) sử dụng STM32 HAL
 */

#ifndef KINCO_ISMK_H
#define KINCO_ISMK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ==================================================================== */
/*                   CANOPEN COB-ID DEFINITIONS                         */
/* ==================================================================== */
#define CANOPEN_COB_NMT             (0x000)
#define CANOPEN_COB_SYNC            (0x080)
#define CANOPEN_COB_EMCY_BASE       (0x080)
#define CANOPEN_COB_TPDO1_BASE      (0x180)
#define CANOPEN_COB_RPDO1_BASE      (0x200)
#define CANOPEN_COB_TPDO2_BASE      (0x280)
#define CANOPEN_COB_RPDO2_BASE      (0x300)
#define CANOPEN_COB_SDO_TX_BASE     (0x580) /* Phản hồi từ Motor -> STM32 */
#define CANOPEN_COB_SDO_RX_BASE     (0x600) /* Yêu cầu từ STM32 -> Motor */
#define CANOPEN_COB_HEARTBEAT_BASE  (0x700)

/* ==================================================================== */
/*             CIA 402 OBJECT DICTIONARY INDEX DEFINITIONS              */
/* ==================================================================== */
#define KINCO_OBJ_CONTROLWORD       (0x6040)
#define KINCO_OBJ_STATUSWORD        (0x6041)
#define KINCO_OBJ_QUICK_STOP_MODE   (0x605A)
#define KINCO_OBJ_MODES_OF_OPER     (0x6060)
#define KINCO_OBJ_MODES_OF_OPER_DSP (0x6061)
#define KINCO_OBJ_ACTUAL_POSITION   (0x6063)
#define KINCO_OBJ_ACTUAL_SPEED      (0x606C)
#define KINCO_OBJ_TARGET_TORQUE     (0x6071)
#define KINCO_OBJ_ACTUAL_TORQUE     (0x6077)
#define KINCO_OBJ_ACTUAL_CURRENT    (0x6078)
#define KINCO_OBJ_TARGET_POSITION   (0x607A)
#define KINCO_OBJ_MAX_VELOCITY      (0x6080)
#define KINCO_OBJ_PROFILE_VELOCITY  (0x6081)
#define KINCO_OBJ_PROFILE_ACCEL     (0x6083)
#define KINCO_OBJ_PROFILE_DECEL     (0x6084)
#define KINCO_OBJ_TARGET_VELOCITY   (0x60FF)

/* ==================================================================== */
/*                   CIA 402 MODES OF OPERATION                         */
/* ==================================================================== */
typedef enum {
    KINCO_MODE_NONE             = 0,
    KINCO_MODE_PROFILE_POSITION = 1,   /* Điều khiển Vị trí / Góc */
    KINCO_MODE_PROFILE_VELOCITY = 3,   /* Điều khiển Vận tốc (RPM) có vòng vị trí */
    KINCO_MODE_VELOCITY_FAST    = -3,  /* Điều khiển Vận tốc tức thời (Immediate) */
    KINCO_MODE_PROFILE_TORQUE   = 4,   /* Điều khiển Lực / Mô-men */
    KINCO_MODE_HOMING           = 6    /* Chế độ tìm gốc tọa độ (Home) */
} Kinco_Mode_t;

/* ==================================================================== */
/*                    CIA 402 STATE MACHINE ENUM                        */
/* ==================================================================== */
typedef enum {
    KINCO_STATE_NOT_READY_TO_SWITCH_ON,
    KINCO_STATE_SWITCH_ON_DISABLED,
    KINCO_STATE_READY_TO_SWITCH_ON,
    KINCO_STATE_SWITCHED_ON,
    KINCO_STATE_OPERATION_ENABLED,
    KINCO_STATE_QUICK_STOP_ACTIVE,
    KINCO_STATE_FAULT_REACTION_ACTIVE,
    KINCO_STATE_FAULT,
    KINCO_STATE_UNKNOWN
} Kinco_State_t;

/* ==================================================================== */
/*                   MOTOR INSTANCE DATA STRUCTURE                      */
/* ==================================================================== */
typedef struct {
    /* Cấu hình cơ bản */
    uint8_t         node_id;             /* ID của motor trên mạng CAN (1, 2, 3...) */
    uint32_t        encoder_resolution;  /* Mặc định iSMK là 65536 xung/vòng */
    Kinco_Mode_t    target_mode;         /* Mode mong muốn */
    Kinco_Mode_t    actual_mode;         /* Mode motor đang chạy phản hồi về */

    /* Trạng thái hoạt động (DS402) */
    Kinco_State_t   state;
    uint16_t        status_word;
    bool            is_connected;
    bool            fault_active;
    bool            target_reached;      /* Đã đến góc/vị trí mục tiêu */
    uint16_t        emergency_code;      /* Mã lỗi EMCY khi có sự cố */

    /* Dữ liệu phản hồi thực tế (Feedback) */
    int32_t         actual_pos_inc;      /* Vị trí thực tế (đơn vị: xung) */
    float           actual_pos_deg;      /* Vị trí góc thực tế (đơn vị: độ) */
    float           actual_pos_rev;      /* Vị trí thực tế (đơn vị: số vòng) */

    int32_t         actual_speed_dec;    /* Tốc độ nội bộ DEC */
    float           actual_speed_rpm;    /* Tốc độ thực tế (vòng/phút - RPM) */

    int16_t         actual_torque_raw;   /* Mô-men xoắn thực tế */
    float           actual_torque_pct;   /* Mô-men thực tế (% định mức) */

    /* Hàm con trỏ gửi tin CAN do người dùng cung cấp */
    bool (*can_send)(uint32_t can_id, const uint8_t *data, uint8_t dlc);
} Kinco_Motor_t;

/* ==================================================================== */
/*                       FUNCTION PROTOTYPES                            */
/* ==================================================================== */

/**
 * @brief Khởi tạo đối tượng motor
 * @param motor Con trỏ đối tượng motor
 * @param node_id ID mạng CAN của motor (thường là 1)
 * @param encoder_resolution Độ phân giải encoder (iSMK mặc định là 65536)
 * @param can_send_callback Hàm truyền gói tin CAN (gắn với HAL_CAN_AddTxMessage)
 */
void Kinco_Init(Kinco_Motor_t *motor, uint8_t node_id, uint32_t encoder_resolution,
                bool (*can_send_callback)(uint32_t can_id, const uint8_t *data, uint8_t dlc));

/* --- Quản lý mạng CANopen (NMT) --- */
bool Kinco_NMT_Start(Kinco_Motor_t *motor);
bool Kinco_NMT_Reset(Kinco_Motor_t *motor);

/* --- Chuỗi kích hoạt động cơ (CiA 402 State Machine) --- */
bool Kinco_SendControlword(Kinco_Motor_t *motor, uint16_t controlword);
bool Kinco_ClearFault(Kinco_Motor_t *motor);
bool Kinco_Shutdown(Kinco_Motor_t *motor);
bool Kinco_SwitchOn(Kinco_Motor_t *motor);
bool Kinco_EnableOperation(Kinco_Motor_t *motor);
bool Kinco_DisableOperation(Kinco_Motor_t *motor);
bool Kinco_QuickStop(Kinco_Motor_t *motor);

/* --- Cấu hình chế độ chạy (Mode) --- */
bool Kinco_SetMode(Kinco_Motor_t *motor, Kinco_Mode_t mode);

/* --- Điều khiển VẬN TỐC (RPM) --- */
bool Kinco_SetVelocityRPM(Kinco_Motor_t *motor, float rpm);
bool Kinco_SetProfileAcceleration(Kinco_Motor_t *motor, float rps2);
bool Kinco_SetProfileDeceleration(Kinco_Motor_t *motor, float rps2);

/* --- Điều khiển VỊ TRÍ / GÓC (Angle / Position) --- */
bool Kinco_SetProfileVelocity(Kinco_Motor_t *motor, float max_rpm);
bool Kinco_SetTargetPositionInc(Kinco_Motor_t *motor, int32_t target_inc);
bool Kinco_SetTargetAngleDeg(Kinco_Motor_t *motor, float target_deg);
bool Kinco_TriggerPositionMove(Kinco_Motor_t *motor, bool is_relative);

/* --- Điều khiển LỰC / MÔ-MEN (Torque) --- */
bool Kinco_SetTargetTorquePercent(Kinco_Motor_t *motor, float torque_pct);
bool Kinco_SetMaxSpeedLimitInTorqueMode(Kinco_Motor_t *motor, float max_rpm);

/* --- Xử lý dữ liệu nhận từ Ngắt CAN (CAN Rx Handler) --- */
void Kinco_ProcessRxMessage(Kinco_Motor_t *motor, uint32_t can_id, const uint8_t *data, uint8_t dlc);

/* --- Công thức quy đổi đơn vị --- */
int32_t Kinco_RPM_To_DEC(float rpm, uint32_t resolution);
float   Kinco_DEC_To_RPM(int32_t dec, uint32_t resolution);
int32_t Kinco_Deg_To_Inc(float degrees, uint32_t resolution);
float   Kinco_Inc_To_Deg(int32_t inc, uint32_t resolution);

#ifdef __cplusplus
}
#endif

#endif /* KINCO_ISMK_H */

/**
 * @file kinco_ismk_def.h
 * @brief Định nghĩa hằng số CANopen, Thanh ghi CiA 402 và Cấu trúc dữ liệu cho Kinco iSMK
 */

#ifndef KINCO_ISMK_DEF_H
#define KINCO_ISMK_DEF_H

#include <stdint.h>
#include <stdbool.h>

/* ==================================================================== */
/* 1. ĐỊNH NGHĨA CANOPEN COB-ID (Địa chỉ các gói tin CAN)                */
/* ==================================================================== */
#define CANOPEN_COB_NMT             (0x000)  /* Lệnh chỉ huy toàn mạng (Start/Stop Node) */
#define CANOPEN_COB_SYNC            (0x080)  /* Xung đồng bộ các trục */
#define CANOPEN_COB_EMCY_BASE       (0x080)  /* + Node_ID: Gói báo lỗi khẩn cấp */
#define CANOPEN_COB_TPDO1_BASE      (0x180)  /* + Node_ID: Motor gửi Statusword & Vị trí */
#define CANOPEN_COB_RPDO1_BASE      (0x200)  /* + Node_ID: Master gửi Controlword xuống */
#define CANOPEN_COB_TPDO2_BASE      (0x280)  /* + Node_ID: Motor gửi Vận tốc & Dòng điện */
#define CANOPEN_COB_RPDO2_BASE      (0x300)  /* + Node_ID: Master gửi Vận tốc đích xuống */
#define CANOPEN_COB_SDO_TX_BASE     (0x580)  /* + Node_ID: Phản hồi SDO từ Motor -> STM32 */
#define CANOPEN_COB_SDO_RX_BASE     (0x600)  /* + Node_ID: Yêu cầu SDO từ STM32 -> Motor */
#define CANOPEN_COB_HEARTBEAT_BASE  (0x700)  /* + Node_ID: Gói điểm danh sống/chết (Heartbeat) */

/* ==================================================================== */
/* 2. ĐỊNH NGHĨA THANH GHI CHUẨN CIA 402 (Object Dictionary)           */
/* ==================================================================== */
#define KINCO_OBJ_CONTROLWORD       (0x6040) /* Thanh ghi điều khiển (0x06, 0x07, 0x0F...) */
#define KINCO_OBJ_STATUSWORD        (0x6041) /* Thanh ghi trạng thái đọc về từ motor */
#define KINCO_OBJ_QUICK_STOP_MODE   (0x605A) /* Cấu hình kiểu dừng khẩn cấp */
#define KINCO_OBJ_MODES_OF_OPER     (0x6060) /* Cài đặt chế độ hoạt động (1, 3, 4, 6...) */
#define KINCO_OBJ_MODES_OF_OPER_DSP (0x6061) /* Chế độ hoạt động hiện tại motor đang chạy */
#define KINCO_OBJ_ACTUAL_POSITION   (0x6063) /* Vị trí góc encoder thực tế (inc) */
#define KINCO_OBJ_ACTUAL_SPEED      (0x606C) /* Tốc độ quay thực tế (DEC) */
#define KINCO_OBJ_TARGET_TORQUE     (0x6071) /* Mô-men xoắn đích (0.1% định mức) */
#define KINCO_OBJ_ACTUAL_TORQUE     (0x6077) /* Mô-men xoắn thực tế */
#define KINCO_OBJ_ACTUAL_CURRENT    (0x6078) /* Dòng điện thực tế */
#define KINCO_OBJ_TARGET_POSITION   (0x607A) /* Vị trí đích cần chạy tới (inc) */
#define KINCO_OBJ_MAX_VELOCITY      (0x6080) /* Giới hạn tốc độ tối đa cho phép */
#define KINCO_OBJ_PROFILE_VELOCITY  (0x6081) /* Tốc độ tối đa khi chạy chế độ Vị trí */
#define KINCO_OBJ_PROFILE_ACCEL     (0x6083) /* Gia tốc khi tăng tốc (rps/s) */
#define KINCO_OBJ_PROFILE_DECEL     (0x6084) /* Gia tốc khi giảm tốc (rps/s) */
#define KINCO_OBJ_TARGET_VELOCITY   (0x60FF) /* Tốc độ đích cần quay trong Mode 3 (DEC) */

/* ==================================================================== */
/* 3. CÁC CHẾ ĐỘ HOẠT ĐỘNG (Modes of Operation)                        */
/* ==================================================================== */
typedef enum {
    KINCO_MODE_NONE             = 0,
    KINCO_MODE_PROFILE_POSITION = 1,   /* Chế độ Vị trí / Góc (xoay góc, nâng hạ) */
    KINCO_MODE_PROFILE_VELOCITY = 3,   /* Chế độ Vận tốc (bánh xe robot, AGV) */
    KINCO_MODE_VELOCITY_FAST    = -3,  /* Chế độ Vận tốc tức thời (Immediate) */
    KINCO_MODE_PROFILE_TORQUE   = 4,   /* Chế độ Lực / Mô-men xoắn (kẹp gắp) */
    KINCO_MODE_HOMING           = 6    /* Chế độ tự tìm gốc cơ khí (Home) */
} Kinco_Mode_t;

/* ==================================================================== */
/* 4. MÁY TRẠNG THÁI CIA 402 STATE MACHINE                              */
/* ==================================================================== */
typedef enum {
    KINCO_STATE_NOT_READY_TO_SWITCH_ON,
    KINCO_STATE_SWITCH_ON_DISABLED,
    KINCO_STATE_READY_TO_SWITCH_ON,
    KINCO_STATE_SWITCHED_ON,
    KINCO_STATE_OPERATION_ENABLED,     /* Trạng thái động cơ đã khóa trục và sẵn sàng chạy */
    KINCO_STATE_QUICK_STOP_ACTIVE,     /* Đang dừng nhanh */
    KINCO_STATE_FAULT_REACTION_ACTIVE,
    KINCO_STATE_FAULT,                 /* Đang có lỗi (cần Clear Fault 0x86) */
    KINCO_STATE_UNKNOWN
} Kinco_State_t;

/* ==================================================================== */
/* 5. CẤU TRÚC ĐỐI TƯỢNG MOTOR (Kinco_Motor_t)                         */
/* ==================================================================== */
typedef struct {
    /* Cấu hình cơ bản */
    uint8_t         node_id;             /* Địa chỉ ID của motor trên mạng CAN (1, 2...) */
    uint32_t        encoder_resolution;  /* Độ phân giải encoder (iSMK mặc định là 65536) */
    Kinco_Mode_t    target_mode;         /* Chế độ hoạt động mong muốn */

    /* Trạng thái vận hành */
    Kinco_State_t   state;               /* Trạng thái DS402 hiện tại */
    uint16_t        status_word;         /* Giá trị thanh ghi 0x6041 đọc về */
    bool            is_connected;        /* Đã nhận được gói tin từ motor */
    bool            fault_active;        /* Báo true nếu motor đang bị lỗi */
    bool            target_reached;      /* Báo true khi motor đã chạy tới góc/vị trí đặt */
    uint16_t        emergency_code;      /* Mã lỗi khẩn cấp EMCY từ motor */

    /* Dữ liệu phản hồi thực tế (Feedback) */
    int32_t         actual_pos_inc;      /* Vị trí thực tế (đơn vị: xung) */
    float           actual_pos_deg;      /* Góc thực tế (đơn vị: độ) */
    float           actual_pos_rev;      /* Số vòng quay thực tế */

    int32_t         actual_speed_dec;    /* Tốc độ nội bộ DEC */
    float           actual_speed_rpm;    /* Tốc độ thực tế (vòng/phút - RPM) */

    int16_t         actual_torque_raw;   /* Giá trị mô-men thô */
    float           actual_torque_pct;   /* Mô-men thực tế (% định mức) */

    /* Con trỏ hàm gửi gói tin CAN (Gắn với HAL_CAN_AddTxMessage trên STM32) */
    bool (*can_send)(uint32_t can_id, const uint8_t *data, uint8_t dlc);
} Kinco_Motor_t;

#endif /* KINCO_ISMK_DEF_H */

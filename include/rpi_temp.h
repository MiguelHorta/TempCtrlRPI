#ifndef RPI_TEMP_H
#define RPI_TEMP_H


enum regs {
    NUL = 0,            // NIL
    RPM,                // R/W
    DUTY_CYCLE,         // R/W
    CONTROL_MODE,       // R
    SYS_FAN_LEVEL,       // R/W
    RAW_RPM,            // R
};

typedef enum target_mode {
    RPM_MODE = 0,
    DUTY_CYCLE_MODE = 1,
} target_mode_t;

typedef enum sys_fan_level {
    FAN_IDLE,
    FAN_MEDIUM,
    FAN_HIGH,
    FAN_HIGHEST,
} sys_fan_level_t;

#endif /* RPI_TEMP_H */

#ifndef RPI_TEMP_H
#define RPI_TEMP_H


enum regs {
    NUL = 0,            // NIL
    RPM,                // R/W
    DUTY_CYCLE,         // R/W
    CONTROL_MODE,       // R
    TEMP_1,             // R
    TEMP_2,             // R
    RAW_RPM,            // R
    RAW_TEMP_1,         // R
    RAW_TEMP_2,         // R
};

typedef enum target_mode {
    RPM_MODE = 0,
    DUTY_CYCLE_MODE = 1,
} target_mode_t;

#endif /* RPI_TEMP_H */

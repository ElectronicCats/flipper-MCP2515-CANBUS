#include "pid_library.h"

uint16_t get_engine_speed(uint8_t value_a, uint8_t value_b) {
    uint16_t operation_a = 0;
    uint16_t operation_b = 0;

    if(value_a != 0) operation_a = value_a * 64;
    if(value_b != 0) operation_b = value_b / 4;

    return operation_a + operation_b;
}

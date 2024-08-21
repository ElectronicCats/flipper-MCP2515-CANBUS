#include "pid_library.h"

bool pid_init(OBDII* obdii) {
    obdii->CAN = mcp_alloc(MCP_NORMAL, MCP_16MHZ, obdii->bitrate);

    if(mcp2515_init(obdii->CAN) != ERROR_OK) return false;

    obdii->codes = (pid_code*)malloc(200 * (sizeof(pid_code)));

    init_mask(obdii->CAN, 0, 0x7FF);
    init_filter(obdii->CAN, 0, 0x7E8);

    init_mask(obdii->CAN, 1, 0x7FF);
    init_filter(obdii->CAN, 1, 0x7E8);

    obdii->frame_to_send.canId = 0x7DF;
    obdii->frame_to_send.data_lenght = 8;

    return true;
}

bool pid_show_data(OBDII* obdii, uint8_t pid, uint8_t* data, uint8_t size) {
    MCP2515* CAN = obdii->CAN;
    CANFRAME frame = obdii->frame_to_send;
    frame.buffer[0] = 2;
    frame.buffer[1] = SHOW_DATA;
    frame.buffer[2] = pid;

    if(size < 8) return false;

    if(send_can_frame(CAN, &frame) != ERROR_OK) return false;

    furi_delay_ms(1);

    if(read_can_message(CAN, &obdii->frame_to_received) != ERROR_OK) return false;

    for(uint8_t i = 0; i < size; i++) {
        data[i] = obdii->frame_to_received.buffer[i];
    }

    return true;
}

void pid_deinit(OBDII* obdii) {
    free_mcp2515(obdii->CAN);
    free(obdii->codes);
}

uint16_t calculate_engine_speed(uint8_t value_a, uint8_t value_b) {
    uint16_t operation_a = 0;
    uint16_t operation_b = 0;

    if(value_a != 0) operation_a = value_a * 64;
    if(value_b != 0) operation_b = value_b / 4;

    return operation_a + operation_b;
}

float calculate_engine_load(uint8_t value) {
    if(value == 0) return 0;
    return value / 2.55;
}

uint16_t sum_value(uint8_t A, uint8_t B) {
    return (A * 256) + B;
}

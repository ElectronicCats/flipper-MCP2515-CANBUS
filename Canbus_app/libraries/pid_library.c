#include "pid_library.h"

// PID TYPICAL NAMES

char* pid_codes_name[] = {
    "PIDs supported A",
    "Monitor Status since DTCs cleared",
    "DTC that caused freeze frame to be store",
    "Fuel System Status",
    "Calculated engine load",
    "Engine Coolant temperature",
    "STFT BANK 1",
    "LTFT BANK 1",
    "STFT BANK 2",
    "LFTF BANK 2",
    "Fuel pressure",
    "Intake manifold absolute pressure",
    "Engine speed",
    "Vehicle speed",
    "Timing advance",
    "Intake air temperature",
    "Mass air flow sensor (MAF) air flow rate",
    "Throttle position",
    "Commanded secondary air status",
    "Oxygen sensors present (in 2 banks)",
    "Oxygen Sensor 1",
    "Oxygen Sensor 2",
    "Oxygen Sensor 3",
    "Oxygen Sensor 4",
    "Oxygen Sensor 5",
    "Oxygen Sensor 6",
    "Oxygen Sensor 7",
    "Oxygen Sensor 8",
    "OBD standards this vehicle conforms to",
    "Oxygen sensors present (in 4 banks)",
    "Auxiliary input status",
    "Run time since engine start",
    "PIDs supported B",
};

// Init the obdii
bool pid_init(OBDII* obdii) {
    obdii->CAN = mcp_alloc(MCP_NORMAL, MCP_16MHZ, obdii->bitrate);

    if(mcp2515_init(obdii->CAN) != ERROR_OK) return false;

    obdii->codes = (pid_code*)malloc(200 * (sizeof(pid_code)));

    init_mask(obdii->CAN, 0, 0x7FF);
    init_filter(obdii->CAN, 0, 0x7E8);

    init_mask(obdii->CAN, 1, 0x7FF);
    init_filter(obdii->CAN, 1, 0x7E8);

    obdii->frame_to_send.canId = ECU_REQUEST_ID;
    obdii->frame_to_send.data_lenght = 8;

    return true;
}

// It works to get the current data of the requested pid
bool pid_show_data(OBDII* obdii, uint8_t pid, uint8_t* data, uint8_t size) {
    MCP2515* CAN = obdii->CAN;
    CANFRAME frame = obdii->frame_to_send;
    ERROR_CAN ret = ERROR_NOMSG;

    frame.buffer[0] = 2;
    frame.buffer[1] = SHOW_DATA;
    frame.buffer[2] = pid;

    if(size < 8) return false;

    if(send_can_frame(CAN, &frame) != ERROR_OK) return false;

    uint16_t time_delay = 0;

    do {
        ret = read_can_message(CAN, &obdii->frame_to_received);
        furi_delay_us(1);
        time_delay++;

    } while((ret != ERROR_OK) && (time_delay < 1500));

    if(ret != ERROR_OK) return false;

    for(uint8_t i = 0; i < size; i++) {
        data[i] = obdii->frame_to_received.buffer[i];
    }

    return true;
}

// It works to send a mode in the pid
bool pid_manual_request(OBDII* obdii, pid_services mode, uint8_t pid, uint8_t* data) {
    MCP2515* CAN = obdii->CAN;
    CANFRAME frame = obdii->frame_to_send;

    ERROR_CAN ret = ERROR_NOMSG;

    frame.data_lenght = 8;

    if(mode == CLEAR_STORAGE_DTC) {
        frame.buffer[0] = 01;
        frame.buffer[1] = mode;

        if(send_can_frame(CAN, &frame) != ERROR_OK) return false;
        return true;
    }

    frame.buffer[0] = 02;
    frame.buffer[1] = mode;
    frame.buffer[2] = pid;

    if(send_can_frame(CAN, &frame) != ERROR_OK) return false;

    uint16_t time_delay = 0;

    do {
        ret = read_can_message(CAN, &frame);
        furi_delay_us(1);
        time_delay++;
    } while((ret != ERROR_OK) && (time_delay < 15000));

    if(ret != ERROR_OK) return false;

    for(uint8_t i = 0; i < 8; i++)
        data[i] = frame.buffer[i];

    return true;
}

// It works to get the supported datas
bool pid_get_supported_pid(OBDII* obdii, uint8_t block) {
    uint8_t data[8];

    if(!pid_manual_request(obdii, SHOW_DATA, block, data)) return false;

    obdii->codes[block].pid_num = block;
    obdii->codes[block].is_supported = true;
    obdii->codes[block].name = "Supported";

    uint32_t codes_available = (data[3] << 24) + (data[4] << 16) + (data[5] << 8) + data[6];

    for(uint8_t i = 1; i <= 32; i++) {
        obdii->codes[block + i].pid_num = block + i;
        obdii->codes[block + i].is_supported = codes_available & (1 << (32 - i));
        if(block == BLOCK_A)
            obdii->codes[i].name = pid_codes_name[i];
        else
            obdii->codes[block + i].name = "UNKNOWN";
    }

    return true;
}

// It works to free
void pid_deinit(OBDII* obdii) {
    free_mcp2515(obdii->CAN);
    free(obdii->codes);
}

// Calculate the engine speed
uint16_t calculate_engine_speed(uint8_t value_a, uint8_t value_b) {
    uint16_t operation_a = 0;
    uint16_t operation_b = 0;

    if(value_a != 0) operation_a = value_a * 64;
    if(value_b != 0) operation_b = value_b / 4;

    return operation_a + operation_b;
}

// Calcuate the engine load
float calculate_engine_load(uint8_t value) {
    if(value == 0) return 0;
    return value / 2.55;
}

// To add between the A value and B value
uint16_t sum_value(uint8_t A, uint8_t B) {
    return (A * 256) + B;
}

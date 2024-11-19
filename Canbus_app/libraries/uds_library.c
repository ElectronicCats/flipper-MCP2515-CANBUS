#include "uds_library.h"

// Function to malloc the instance
UDS_SERVICE* uds_service_alloc(
    uint32_t id_to_send,
    uint32_t id_to_received,
    MCP_MODE mode,
    MCP_CLOCK clk,
    MCP_BITRATE bitrate) {
    UDS_SERVICE* instance = malloc(sizeof(UDS_SERVICE));
    instance->CAN = mcp_alloc(mode, clk, bitrate);
    instance->id_to_send = id_to_send;
    instance->id_to_received = id_to_received;

    return instance;
}

// Init the mcp2515 with it respeclty mask and filters
bool uds_init(UDS_SERVICE* uds_instance) {
    MCP2515* CAN = uds_instance->CAN;
    if(mcp2515_init(CAN) != ERROR_OK) return false;

    uint32_t mask = 0x7ff;

    if(uds_instance->id_to_received > 0x7ff) {
        mask = 0x1FFFFFFF;
    }

    init_mask(CAN, 0, mask);
    init_filter(CAN, 0, uds_instance->id_to_received);

    init_mask(CAN, 1, mask);
    init_filter(CAN, 1, uds_instance->id_to_received);

    return true;
}

// Free instance
void free_uds(UDS_SERVICE* uds_instance) {
    free_mcp2515(uds_instance->CAN);
    free(uds_instance);
}

// Get Frames
bool read_frames_uds(MCP2515* CAN, uint32_t id, CANFRAME* frame) {
    uint32_t time_delay = 0;

    do {
        if(read_can_message(CAN, frame) == ERROR_OK) {
            if(frame->canId == id) {
                return true;
            }
        }
        furi_delay_us(1);
        time_delay++;

    } while((time_delay < 6000));

    log_exception("Error");

    return false;
}

// Function to send a service
bool uds_single_frame_request(
    UDS_SERVICE* uds_instance,
    uint8_t* data_to_send,
    uint8_t count_of_bytes,
    CANFRAME* frames_to_received,
    uint8_t count_of_frames) {
    MCP2515* CAN = uds_instance->CAN;
    CANFRAME frame_to_send = {0};
    frame_to_send.canId = uds_instance->id_to_send;
    frame_to_send.data_lenght = count_of_bytes + 1;
    uint32_t id_to_received = uds_instance->id_to_received;
    ERROR_CAN ret = ERROR_OK;

    for(uint8_t i = 0; i < frame_to_send.data_lenght; i++)
        frame_to_send.buffer[i] = data_to_send[i];

    ret = send_can_frame(CAN, &frame_to_send);

    if(ret != ERROR_OK) return false;

    memset(frame_to_send.buffer, 0, sizeof(frame_to_send.buffer));
    frame_to_send.buffer[0] = 0x30;
    frame_to_send.data_lenght = 3;

    for(uint8_t i = 0; i < count_of_frames; i++) {
        if(i == 1) {
            ret = send_can_frame(CAN, &frame_to_send);
            if(ret != ERROR_OK) return false;
        }

        if(!read_frames_uds(CAN, id_to_received, &(frames_to_received[i]))) {
            if(i == 0)
                return false;
            else
                break;
        }
    }

    return true;
}

// Function to get VIN
bool uds_get_vin(UDS_SERVICE* uds_instance, FuriString* text) {
    CANFRAME canframes[3];

    memset(canframes, 0, sizeof(canframes));

    uint8_t request[] = {0x22, 0xf1, 0x90};

    if(!uds_single_frame_request(uds_instance, request, 3, canframes, 3)) return false;

    if(canframes[0].buffer[0] != 0x62) return false;

    char vin_name[17];

    uint8_t pos = 0;

    for(uint8_t i = 0; i < 3; i++) {
        uint8_t start_num = 1; // The byte to start saving
        if(i == 0) start_num = 5;

        // Save bytes in the char array
        for(uint8_t j = start_num; j < 8; j++) {
            vin_name[pos++] = canframes[i].buffer[j];
        }
    }

    furi_string_reset(text);

    furi_string_cat_str(text, vin_name);

    return true;
}

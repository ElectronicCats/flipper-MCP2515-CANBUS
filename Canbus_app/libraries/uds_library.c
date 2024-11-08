#include "uds_library.h"

// Function to malloc the instance
UDS_SERVICE* uds_service_alloc(MCP2515* CAN, uint32_t id_to_send, uint32_t id_to_received) {
    UDS_SERVICE* instance = malloc(sizeof(UDS_SERVICE));
    instance->CAN = mcp_alloc(CAN->mode, CAN->clck, CAN->bitRate);
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

// Function to send a service
bool manual_uds_service_request(
    UDS_SERVICE* uds_instance,
    CANFRAME* frames_to_received,
    uint8_t count_of_frames) {
    MCP2515* CAN = uds_instance->CAN;
    CANFRAME frame_to_send = {0};
    frame_to_send.canId = uds_instance->id_to_send;
    uint32_t id_to_received = uds_instance->id_to_received;

    UNUSED(frame_to_send);
    UNUSED(id_to_received);
    UNUSED(frames_to_received);
    UNUSED(count_of_frames);
    UNUSED(CAN);

    return false;
}

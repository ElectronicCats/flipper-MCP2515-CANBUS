#ifndef UDS_LIBRARY
#define UDS_LIBRARY

#include <furi_hal.h>
#include "mcp_can_2515.h"

#define DEFAULT_ECU_REQUEST  0x7e0
#define DEFAULT_ECU_RESPONSE 0x7e8

typedef struct {
    MCP2515* CAN;
    uint32_t id_to_send;
    uint32_t id_to_received;
} UDS_SERVICE;

UDS_SERVICE* uds_service_alloc(
    uint32_t id_to_send,
    uint32_t id_to_received,
    MCP_MODE mode,
    MCP_CLOCK clk,
    MCP_BITRATE bitrate);

// Init Uds
bool uds_init(UDS_SERVICE* uds_instance);

// send a manual request
bool uds_single_frame_request(
    UDS_SERVICE* uds_instance,
    uint8_t* data_to_send,
    uint8_t count_of_bytes,
    CANFRAME* frames_to_received,
    uint8_t count_of_frames);

// Send a multi frame request
bool uds_multi_frame_request(
    UDS_SERVICE* uds,
    uint8_t* data,
    uint8_t length,
    CANFRAME* canframes_to_send,
    uint8_t count_of_frames_to_received,
    CANFRAME* canframes_to_received);

// Get VIN Number
bool uds_get_vin(UDS_SERVICE* uds_instance, FuriString* text);

// Free uds
void free_uds(UDS_SERVICE* uds_instance);

#endif

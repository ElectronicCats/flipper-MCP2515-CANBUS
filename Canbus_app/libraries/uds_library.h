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

UDS_SERVICE* uds_service_alloc(MCP2515* CAN, uint32_t id_to_send, uint32_t id_to_received);
bool uds_init(UDS_SERVICE* uds_instance);
void free_uds(UDS_SERVICE* uds_instance);

#endif

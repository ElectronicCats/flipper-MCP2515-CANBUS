#include "library/mcp_can_2515.h"

//--------------------------------------------------------------------------
//  Main
//--------------------------------------------------------------------------

int app_main(void* p) {
    UNUSED(p);

    CANFRAME* frame = malloc(sizeof(CANFRAME));

    ERROR_CAN debugStatus = ERROR_OK;
    MCP2515* mcp_can = mcp_alloc(MCP_NORMAL, MCP_16MHZ, MCP_500KBPS);
    debugStatus = mcp2515_init(mcp_can);
    if(debugStatus == ERROR_OK) {
        log_info("ALL GOOD");
    } else {
        log_exception("FAILURE");
    }

    if(readMSG(mcp_can, frame) == ERROR_OK) {
        log_info("val %u", frame->buffer[0]);
    } else {
        log_exception("NO MSG READ");
    }

    freeMCP2515(mcp_can);

    return 0;
}
#include "mcp_can_2515.h"

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

    while(furi_hal_gpio_read(&gpio_button_back)) {
        if(checkReceive(mcp_can) == ERROR_OK) {
            log_info("Message received");
        }
        if(readMSG(mcp_can, frame) == ERROR_OK) {
            log_info(
                "id: %li \tlen: %u\tdata: %u\t%u\t%u\t%u\t%u\t%u\t%u\t%u",
                frame->canId,
                frame->len,
                frame->buffer[0],
                frame->buffer[1],
                frame->buffer[2],
                frame->buffer[3],
                frame->buffer[4],
                frame->buffer[5],
                frame->buffer[6],
                frame->buffer[7]);
        }

        if(checkError(mcp_can) == ERROR_FAIL) {
            log_exception("ERROR");
        }
        furi_delay_ms(10);
    }

    freeMCP2515(mcp_can);

    return 0;
}
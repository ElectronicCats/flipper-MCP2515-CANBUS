#include "furi.h"
#include "furi_hal.h"
#include "mcp_can_2515.h"

#define pinInterrupt &gpio_swclk;

static bool interrupt = false;

void callback_interrupt(void* context) {
    UNUSED(context);
    interrupt = true;
}

int app_main(void* p) {
    UNUSED(p);

    CANFRAME* frame = malloc(sizeof(CANFRAME));
    bool run = true;
    ERROR_CAN msg_ok = ERROR_OK;
    uint8_t error = 0;

    UNUSED(msg_ok);
    UNUSED(error);

    ERROR_CAN debugStatus = ERROR_OK;
    MCP2515* mcp_can = mcp_alloc(MCP_LISTENONLY, MCP_16MHZ, MCP_500KBPS);
    debugStatus = mcp2515_init(mcp_can);

    furi_hal_gpio_init(&gpio_swclk, GpioModeInterruptFall, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_add_int_callback(&gpio_swclk, callback_interrupt, NULL);

    if(debugStatus == ERROR_OK) {
        log_info("ALL GOOD");
    } else {
        log_exception("FAILURE");
        run = false;
    }

    while(furi_hal_gpio_read(&gpio_button_back) && run) {
        if(interrupt) {
            msg_ok = checkError(mcp_can);
            error = get_error(mcp_can);
            interrupt = false;
        }

        if((readMSG(mcp_can, frame) == ERROR_OK) && (msg_ok == ERROR_OK)) {
            log_info(
                "MSG OK  id: %li \tlen: %u\tdata: %u\t%u\t%u\t%u\t%u\t%u\t%u\t%u",
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
        } else if((readMSG(mcp_can, frame) == ERROR_OK) && (msg_ok == ERROR_FAIL)) {
            log_exception(
                "MSG ERROR: %u \tid: %li \tlen: %u\tdata: %u\t%u\t%u\t%u\t%u\t%u\t%u\t%u",
                error,
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
        furi_delay_ms(10);
    }

    furi_hal_gpio_remove_int_callback(&gpio_swclk);

    freeMCP2515(mcp_can);

    return 0;
}
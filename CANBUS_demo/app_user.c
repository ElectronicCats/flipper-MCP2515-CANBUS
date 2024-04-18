#include "furi.h"
#include "furi_hal.h"
#include "mcp_can_2515.h"

static bool interrupt = false;

void callback_interrupt(void* context) {
    UNUSED(context);
    interrupt = true;
}

int app_main(void* p) {
    UNUSED(p);

    CANFRAME* frame = malloc(sizeof(CANFRAME));

    frame->canId = 3000;
    frame->data_lenght = 5;
    frame->ext = 1;
    frame->req = 0;

    for(uint8_t i = 0; i < frame->data_lenght; i++) {
        frame->buffer[i] = i;
    }

    bool run = true;
    ERROR_CAN msg_ok = ERROR_OK;
    uint8_t error = 0;

    uint8_t buff[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    UNUSED(buff);
    UNUSED(frame);
    UNUSED(msg_ok);
    UNUSED(error);

    ERROR_CAN debugStatus = ERROR_OK;
    MCP2515* mcp_can = mcp_alloc(MCP_NORMAL, MCP_16MHZ, MCP_500KBPS);
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
        /*if(interrupt) {
            interrupt = false;
        }

        if((read_can_message(mcp_can, frame) == ERROR_OK)) {
            log_info(
                "MSG OK  id: %li \tlen: %u\tdata: %u\t%u\t%u\t%u\t%u\t%u\t%u\t%u",
                frame->canId,
                frame->data_lenght,
                frame->buffer[0],
                frame->buffer[1],
                frame->buffer[2],
                frame->buffer[3],
                frame->buffer[4],
                frame->buffer[5],
                frame->buffer[6],
                frame->buffer[7]);
        }*/

        if(!(furi_hal_gpio_read(&gpio_button_right))) {
            send_can_frame(mcp_can, frame);
            furi_delay_ms(1000);
        }

        furi_delay_ms(10);
    }

    furi_hal_gpio_remove_int_callback(&gpio_swclk);

    free_mcp2515(mcp_can);
    free(frame);

    return 0;
}
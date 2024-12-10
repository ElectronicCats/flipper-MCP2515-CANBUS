#include "../app_user.h"

bool interr = false;

const char* bitrates_names[] = {
    "125KBPS",
    "250KBPS",
    "500KBPS",
    "1000KBPS",
};

static int32_t thread_to_detect_speed(void* context);

void app_scene_speed_detector_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    app->thread = furi_thread_alloc_ex("Auto-detector Speed", 1024, thread_to_detect_speed, app);
    furi_thread_start(app->thread);

    draw_in_development(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_speed_detector_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(app);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void app_scene_speed_detector_on_exit(void* context) {
    App* app = context;
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
    widget_reset(app->widget);
}

/*
bool get_real_bitrate(MCP2515* mcp_can, MCP_BITRATE bitrate) {
    for(uint8_t j = 0; j < 10; j++) {
        if(detect_baudrate(mcp_can, bitrate)) {
            return true;
        }

        furi_delay_ms(1);
    }
}
*/

static int32_t thread_to_detect_speed(void* context) {
    App* app = context;

    MCP2515* mcp_can = app->mcp_can;
    mcp_can->mode = MCP_LISTENONLY;

    bool debug = (mcp2515_init(mcp_can) == ERROR_OK) ? true : false;

    if(!debug) draw_device_no_connected(app);

    uint8_t baud_detected = 0;

    MCP_BITRATE bitrates[] = {
        MCP_125KBPS,
        MCP_250KBPS,
        MCP_500KBPS,
        MCP_1000KBPS,
    };

    UNUSED(bitrates);

    furi_delay_ms(100);

    // uint8_t count = 0;

    uint32_t time = furi_get_tick();

    while(furi_hal_gpio_read(&gpio_button_back)) {
        /*if(detect_baudrate(mcp_can, count)) {
            log_info("The bitrate %s is %u", bitrates_names[count], count);
        }*/

        if((furi_get_tick() - time) > 1000) {
            log_info(
                "Con bitrate de %s value of: %u ---------------------",
                bitrates_names[mcp_can->bitRate],
                mcp_can->bitRate);
            detect_baudrate(mcp_can, mcp_can->bitRate);
            // count++;
            time = furi_get_tick();
        }

        // if(count > 3) count = 0;
    }

    log_info("BAUD DETECTED %u", baud_detected);

    free_mcp2515(mcp_can);

    return 0;
}

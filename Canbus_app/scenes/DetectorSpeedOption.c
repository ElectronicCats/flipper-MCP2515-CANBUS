#include "../app_user.h"

bool interr = false;

const char* bitrates_names[] = {
    "125KBPS",
    "250KBPS",
    "500KBPS",
    "1000KBPS",
};

MCP_BITRATE bitrates[] = {
    MCP_125KBPS,
    MCP_250KBPS,
    MCP_500KBPS,
    MCP_1000KBPS,
};

uint32_t time = 0;

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

// Callback for the timer
void timer_callback(void* context) {
    UNUSED(context);
    time++;
}

//Thread to detect the bitrate
static int32_t thread_to_detect_speed(void* context) {
    App* app = context;

    MCP2515* mcp_can = app->mcp_can;
    mcp_can->mode = MCP_LISTENONLY;

    bool debug = (mcp2515_init(mcp_can) == ERROR_OK) ? true : false;

    if(!debug) draw_device_no_connected(app);

    furi_delay_ms(100);

    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, app);

    furi_timer_start(timer, 1000);

    uint32_t counter = 0;
    uint32_t bitrate_selector = 0;

    ERROR_CAN response = ERROR_NOMSG;

    while(furi_hal_gpio_read(&gpio_button_back) && debug) {
        response = is_this_bitrate(mcp_can, bitrates[bitrate_selector]);

        if(response == ERROR_OK) {
            counter++;
            time = 0;
        }

        if((counter > 0) && (response == ERROR_FAIL)) {
            bitrate_selector++;
            counter = 0;
        }

        if(counter > 5) break;

        if(bitrate_selector > 3) break;

        if(time > 10) break;

        furi_delay_ms(1);
    }

    if(response == ERROR_NOMSG && debug) {
        log_exception("Bitrate no detected");
    } else {
        if(bitrate_selector > 3) {
            log_exception("No bitrate detected");
        }

        if(bitrate_selector <= 3) {
            log_info("The Bitrate is: %s", bitrates_names[bitrate_selector]);
            mcp_can->bitRate = bitrates[bitrate_selector];
        }
    }

    time = 0;

    furi_timer_stop(timer);
    furi_timer_free(timer);

    free_mcp2515(mcp_can);

    return 0;
}

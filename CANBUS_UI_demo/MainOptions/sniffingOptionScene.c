#include "../app_user.h"

static void callback_interrupt(void* context) {
    App* app = context;
    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagReceived);
}

static int32_t sniffing_worker(void* context) {
    App* app = context;
    MCP2515* mcp_can = app->mcp_can;
    CANFRAME frame = app->can_frame;

    ERROR_CAN debugStatus = mcp2515_init(mcp_can);

    furi_hal_gpio_init(&gpio_swclk, GpioModeInterruptFall, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_add_int_callback(&gpio_swclk, callback_interrupt, app);

    if(debugStatus == ERROR_OK) {
        log_info("MCP START OK");
    } else {
        log_exception("MCP SET FAILURE");
    }

    while(true) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        uint8_t error = 0;
        bool error_msg = false;

        UNUSED(error);
        UNUSED(error_msg);

        if(events & WorkerflagStop) {
            break;
        } else if(events & WorkerflagReceived) {
            if((read_can_message(mcp_can, &frame) == ERROR_OK) && (error_msg == false)) {
                furi_string_cat_printf(app->text, "addr:%lx    ERROR: OK\n", frame.canId);
                furi_string_cat_printf(app->text, "DATA[%x]    req:%x\n", frame.len, frame.req);
                for(uint8_t i = 0; i < frame.len; i++) {
                    furi_string_cat_printf(app->text, "%x  ", frame.buffer[i]);
                }
                furi_string_cat_printf(app->text, "\n");
            }

            if((read_can_message(mcp_can, &frame) == ERROR_OK) && (error_msg == true)) {
                furi_string_cat_printf(app->text, "addr:%lx  ERROR: %x\n", frame.canId, error);
                furi_string_cat_printf(app->text, "DATA[%x]    req:%x\n", frame.len, frame.req);
                for(uint8_t i = 0; i < frame.len; i++) {
                    furi_string_cat_printf(app->text, "%x  ", frame.buffer[i]);
                }
                furi_string_cat_printf(app->text, "\n");
            }

            view_dispatcher_send_custom_event(app->view_dispatcher, Refresh);
        }
    }

    furi_hal_gpio_remove_int_callback(&gpio_swclk);
    free_mcp2515(mcp_can);
    return 0;
}

// Sniffing on enter
void app_scene_Sniffing_on_enter(void* context) {
    App* app = context;
    app->mcp_can->mode = MCP_LISTENONLY;

    //  We alloc the thread
    app->thread = furi_thread_alloc_ex("SniffingWork", 1024, sniffing_worker, app);

    // Start the thread
    furi_thread_start(app->thread);

    if(scene_manager_get_scene_state(app->scene_manager, AppScenesniffingOption) ==
       SniffingOption) {
        text_box_set_font(app->textBox, TextBoxFontText);
        text_box_set_focus(app->textBox, TextBoxFocusEnd);
        furi_string_cat_printf(app->text, " ");
    }

    text_box_reset(app->textBox);
    view_dispatcher_switch_to_view(app->view_dispatcher, TextBoxView);
    text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
}

// Sniffing on event
bool app_scene_Sniffing_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(app);
    bool consume = false;
    if(event.type == SceneManagerEventTypeCustom) {
        text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
        text_box_set_focus(app->textBox, TextBoxFocusEnd);
        consume = true;
    }
    return consume;
}

// Sniffing on exit
void app_scene_Sniffing_on_exit(void* context) {
    App* app = context;

    // To close the Thread
    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagStop);
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);

    // Clear the string
    furi_string_reset(app->text);

    // widget Reset
    text_box_reset(app->textBox);
}
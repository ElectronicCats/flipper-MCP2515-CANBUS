#include "../app_user.h"

static void callback_interrupt(void* context) {
    App* app = context;
    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagReceived);
}

static int32_t workerSniffing(void* context) {
    App* app = context;
    MCP2515* mcp_can = app->mcp_can;
    CANFRAME* frame = app->can_frame;

    uint32_t currentId = 0;
    uint32_t listId[64];
    uint8_t numOfDevices = 0;
    bool first = true;

    ERROR_CAN debugStatus = mcp2515_init(mcp_can);

    furi_hal_gpio_init(&gpio_swclk, GpioModeInterruptFall, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_add_int_callback(&gpio_swclk, callback_interrupt, app);

    if(debugStatus == ERROR_OK) {
        log_info("MCP START OK");
    } else {
        log_exception("MCP SET FAILURE");
    }

    while(1) {
        bool new = true;
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        if(events & WorkerflagStop) break;
        if(events & WorkerflagReceived) {
            if(readMSG(mcp_can, frame) == ERROR_OK) {
                currentId = frame->canId;
                log_info("direccion leida");
            }

            if(first) {
                listId[0] = currentId;
                numOfDevices++;
                first = false;
                furi_string_reset(app->textLabel);
                furi_string_cat_printf(app->textLabel, "0x%lx", currentId);
                view_dispatcher_send_custom_event(app->view_dispatcher, RefreshTest);
            } else {
                log_info("Size: %u", numOfDevices);

                for(uint8_t i = 0; i < numOfDevices; i++) {
                    if(listId[i] == currentId) {
                        new = false;
                        break;
                    }
                }

                if(new) {
                    listId[numOfDevices] = currentId;
                    numOfDevices++;
                    furi_string_reset(app->textLabel);
                    furi_string_cat_printf(app->textLabel, "0x%lx", currentId);
                    view_dispatcher_send_custom_event(app->view_dispatcher, RefreshTest);
                }
            }
        }
    }

    furi_hal_gpio_remove_int_callback(&gpio_swclk);
    freeMCP2515(mcp_can);
    return 0;
}

void sniffingTest_callback(void* context, uint32_t index) {
    UNUSED(context);
    UNUSED(index);
}

void app_scene_SniffingTest_on_enter(void* context) {
    App* app = context;

    app->thread = furi_thread_alloc_ex("SniffingWork", 1024, workerSniffing, app);
    furi_thread_start(app->thread);

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "CANBUS ADDRESS");
    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool app_scene_SniffingTest_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case RefreshTest:
            submenu_add_item(
                app->submenu, furi_string_get_cstr(app->textLabel), 0, sniffingTest_callback, app);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return consumed;
}

void app_scene_SniffingTest_on_exit(void* context) {
    App* app = context;

    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagStop);
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);

    furi_string_reset(app->textLabel);
    submenu_reset(app->submenu);
}

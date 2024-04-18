#include "../app_user.h"

bool condition = true;

static void callback_interrupt(void* context) {
    App* app = context;
    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagReceived);
}

static int32_t workerSniffing(void* context) {
    App* app = context;
    MCP2515* mcp_can = app->mcp_can;
    CANFRAME frame = app->can_frame;

    uint32_t currentId = 0;
    uint8_t numOfDevices = 0;
    bool first = true;

    furi_hal_gpio_init(&gpio_swclk, GpioModeInterruptFall, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_add_int_callback(&gpio_swclk, callback_interrupt, app);

    ERROR_CAN debugStatus = mcp2515_init(mcp_can);

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
            if(read_can_message(mcp_can, &frame) == ERROR_OK) {
                currentId = frame.canId;
            }

            if(first) {
                app->frameArray[0] = frame;
                app->index_aux = numOfDevices;
                numOfDevices++;
                first = false;

                furi_string_reset(app->textLabel);
                furi_string_cat_printf(app->textLabel, "0x%lx", currentId);
                view_dispatcher_send_custom_event(app->view_dispatcher, RefreshTest);
            } else {
                for(uint8_t i = 0; i < numOfDevices; i++) {
                    if(app->frameArray[i].canId == currentId) {
                        app->frameArray[i] = frame;
                        new = false;
                        break;
                    }
                }

                if(new&& condition) {
                    app->frameArray[numOfDevices] = frame;
                    app->index_aux = numOfDevices;
                    numOfDevices++;
                    furi_string_reset(app->textLabel);
                    furi_string_cat_printf(app->textLabel, "0x%lx", currentId);
                    view_dispatcher_send_custom_event(app->view_dispatcher, RefreshTest);
                }

                if(frame.canId == app->frameArray[app->index].canId) {
                    view_dispatcher_send_custom_event(app->view_dispatcher, ShowData);
                }
            }
        }
    }

    furi_hal_gpio_remove_int_callback(&gpio_swclk);
    free_mcp2515(mcp_can);
    return 0;
}

void sniffingTest_callback(void* context, uint32_t index) {
    App* app = context;
    app->index = index;
    scene_manager_handle_custom_event(app->scene_manager, EntryEvent);
}

void app_scene_SniffingTest_on_enter(void* context) {
    App* app = context;

    if(condition) {
        app->thread = furi_thread_alloc_ex("SniffingWork", 1024, workerSniffing, app);
        furi_thread_start(app->thread);
        submenu_reset(app->submenu);
        submenu_set_header(app->submenu, "CANBUS ADDRESS");
    }
    condition = true;
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
                app->submenu,
                furi_string_get_cstr(app->textLabel),
                app->index_aux,
                sniffingTest_callback,
                app);
            break;
        case EntryEvent:
            condition = false;
            scene_manager_next_scene(app->scene_manager, AppSceneboxSniffing);
            consumed = true;
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
    if(condition) {
        furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagStop);
        furi_thread_join(app->thread);
        furi_thread_free(app->thread);

        furi_string_reset(app->textLabel);
        submenu_reset(app->submenu);
    }
}

//-------------------------- FOR THE SNIFFING BOX --------------------------------------------------------

void app_scene_BoxSniffing_on_enter(void* context) {
    App* app = context;

    text_box_set_font(app->textBox, TextBoxFontText);

    furi_string_reset(app->text);

    furi_string_cat_printf(
        app->text,
        "ADDR: %lx DLC: %u \n",
        app->frameArray[app->index].canId,
        app->frameArray[app->index].data_lenght);

    for(uint8_t i = 0; i < (app->frameArray[app->index].data_lenght); i++) {
        furi_string_cat_printf(app->text, "[%u]:  %x ", i, app->frameArray[app->index].buffer[i]);
    }

    text_box_reset(app->textBox);
    view_dispatcher_switch_to_view(app->view_dispatcher, TextBoxView);
    text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
    text_box_set_focus(app->textBox, TextBoxFocusEnd);
}

bool app_scene_BoxSniffing_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    if(event.event == ShowData) {
        furi_string_reset(app->text);

        furi_string_cat_printf(
            app->text,
            "ADDR: %lx DLC: %u \n",
            app->frameArray[app->index].canId,
            app->frameArray[app->index].data_lenght);

        for(uint8_t i = 0; i < (app->frameArray[app->index].data_lenght); i++) {
            furi_string_cat_printf(
                app->text, "[%u]:  %x ", i, app->frameArray[app->index].buffer[i]);
        }

        text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
        text_box_set_focus(app->textBox, TextBoxFocusEnd);
        consumed = true;
    }
    return consumed;
}

void app_scene_BoxSniffing_on_exit(void* context) {
    App* app = context;
    furi_string_reset(app->text);
    text_box_reset(app->textBox);
}

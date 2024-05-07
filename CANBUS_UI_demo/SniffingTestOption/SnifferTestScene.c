#include "../app_user.h"

bool condition = true;

// --------------------- Functons to save the logs --------------------------------------------------
char* sequential_file_resolve_path(
    Storage* storage,
    const char* dir,
    const char* prefix,
    const char* extension) {
    if(storage == NULL || dir == NULL || prefix == NULL || extension == NULL) {
        return NULL;
    }

    char file_path[256];
    int file_index = 0;

    do {
        if(snprintf(
               file_path, sizeof(file_path), "%s/%s_%d.%s", dir, prefix, file_index, extension) <
           0) {
            return NULL;
        }
        file_index++;
    } while(storage_file_exists(storage, file_path));

    return strdup(file_path);
}

char* sequential_file_resolve_path_address(
    Storage* storage,
    uint32_t address,
    const char* dir,
    const char* prefix,
    const char* extension) {
    FuriHalRtcDateTime datatime;
    furi_hal_rtc_get_datetime(&datatime);
    char file_path[256];
    int file_index = 0;
    uint16_t year = datatime.year;
    uint8_t month = datatime.month;
    uint8_t day = datatime.day;

    if(storage == NULL || dir == NULL || prefix == NULL || extension == NULL) {
        return NULL;
    }

    do {
        if(snprintf(
               file_path,
               sizeof(file_path),
               "%s/%s_0x%lx_%u_%u_%i_%d.%s",
               dir,
               prefix,
               address,
               day,
               month,
               year,
               file_index,
               extension) < 0) {
            return NULL;
        }
        file_index++;
    } while(storage_file_exists(storage, file_path));

    return strdup(file_path);
}

void save_data_on_log(App* app) {
    strcpy(app->log_file_path, sequential_file_resolve_path(app->storage, PATHLOGS, "Log", "log"));
    if(app->log_file_path != NULL) {
        if(storage_file_open(app->log_file, app->log_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            app->log_file_ready = true;
        } else {
            dialog_message_show_storage_error(app->dialogs, "Cannot open log file");
        }
    } else {
        dialog_message_show_storage_error(app->dialogs, "Cannot resolve log path");
    }
}

void save_address_data_on_log(App* app) {
    uint32_t can_id = app->frameArray[app->sniffer_index].canId;
    strcpy(
        app->log_file_path,
        sequential_file_resolve_path_address(app->storage, can_id, PATHLOGS, "L", "log"));
    if(app->log_file_path != NULL) {
        if(storage_file_open(app->log_file, app->log_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            app->log_file_ready = true;
        } else {
            dialog_message_show_storage_error(app->dialogs, "Cannot open log file");
        }
    } else {
        dialog_message_show_storage_error(app->dialogs, "Cannot resolve log path");
    }
}

void close_file_on_data_log(App* app) {
    app->log_file_ready = false;
    if(app->log_file && storage_file_is_open(app->log_file)) {
        storage_file_close(app->log_file);
    }
}

static void write_data_on_file(CANFRAME frame, File* file, uint32_t time) {
    FuriString* text_file = furi_string_alloc();
    furi_string_cat_printf(text_file, "id:%lx \t\tlen: %u \t", frame.canId, frame.data_lenght);
    for(uint8_t i = 0; i < (frame.data_lenght); i++) {
        furi_string_cat_printf(text_file, "[%u]:%u \t\t", i, frame.buffer[i]);
    }
    furi_string_cat_printf(text_file, " Time(ms): %li\n", time);
    storage_file_write(file, furi_string_get_cstr(text_file), furi_string_size(text_file));
    furi_string_reset(text_file);
    furi_string_free(text_file);
}

// --------------------------- Thread on work ------------------------------------------------------
void timer_callback(void* context) {
    App* app = context;
    app->time++;
}

static void callback_interrupt(void* context) {
    App* app = context;
    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagReceived);
}

static int32_t worker_sniffing(void* context) {
    App* app = context;
    MCP2515* mcp_can = app->mcp_can;
    CANFRAME frame = app->can_frame;
    app->timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, app);

    uint32_t current_id = 0;
    uint8_t num_of_devices = 0;
    uint32_t time_select = 0;
    bool run = true;
    bool first = true;

    furi_hal_gpio_init(&gpio_swclk, GpioModeInterruptFall, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_add_int_callback(&gpio_swclk, callback_interrupt, app);

    ERROR_CAN debugStatus = mcp2515_init(mcp_can);

    if(debugStatus != ERROR_OK) {
        run = false;
        view_dispatcher_send_custom_event(app->view_dispatcher, DEVICE_NO_CONNECTED);
    }

    furi_timer_start(app->timer, 1);

    while(run) {
        bool new = true;
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        if(events & WorkerflagStop) break;
        if(events & WorkerflagReceived) {
            if(read_can_message(mcp_can, &frame) == ERROR_OK) {
                current_id = frame.canId;
            }

            if(first) {
                app->frameArray[0] = frame;
                app->sniffer_index_aux = num_of_devices;
                app->times[num_of_devices] = 0;
                app->current_time[num_of_devices] = app->time;
                num_of_devices++;
                first = false;

                furi_string_reset(app->textLabel);
                furi_string_cat_printf(app->textLabel, "0x%lx", current_id);

                view_dispatcher_send_custom_event(app->view_dispatcher, SaveDataOk);
                view_dispatcher_send_custom_event(app->view_dispatcher, RefreshTest);
                app->num_of_devices = num_of_devices;
                time_select = num_of_devices;
            } else {
                for(uint8_t i = 0; i < num_of_devices; i++) {
                    if(app->frameArray[i].canId == current_id) {
                        app->frameArray[i] = frame;
                        app->times[i] = (app->time - app->current_time[i]);
                        app->current_time[i] = app->time;
                        time_select = i;
                        new = false;
                        break;
                    }
                }

                if(new&& condition) {
                    app->frameArray[num_of_devices] = frame;
                    app->sniffer_index_aux = num_of_devices;
                    app->times[num_of_devices] = 0;
                    app->current_time[num_of_devices] = app->time;
                    num_of_devices++;
                    furi_string_reset(app->textLabel);
                    furi_string_cat_printf(app->textLabel, "0x%lx", current_id);
                    view_dispatcher_send_custom_event(app->view_dispatcher, RefreshTest);
                    app->num_of_devices = num_of_devices;
                    time_select = num_of_devices;
                }

                if(frame.canId == app->frameArray[app->sniffer_index].canId) {
                    view_dispatcher_send_custom_event(app->view_dispatcher, ShowData);
                }
            }

            if(app->log_file_ready && (app->save_logs == SaveAll)) {
                write_data_on_file(frame, app->log_file, app->times[time_select]);
            }
        }
    }

    furi_timer_stop(app->timer);
    furi_timer_free(app->timer);
    furi_hal_gpio_remove_int_callback(&gpio_swclk);
    free_mcp2515(mcp_can);
    return 0;
}

// ------------------------------------------------------ SNIFFING MENU SCENE ---------------------------

void sniffingTest_callback(void* context, uint32_t index) {
    App* app = context;
    app->sniffer_index = index;
    scene_manager_handle_custom_event(app->scene_manager, EntryEvent);
}

void app_scene_SniffingTest_on_enter(void* context) {
    App* app = context;

    if(condition) {
        app->thread = furi_thread_alloc_ex("SniffingWork", 1024, worker_sniffing, app);
        furi_thread_start(app->thread);
        submenu_reset(app->submenu);
        submenu_set_header(app->submenu, "CANBUS ADDRESS");
    }

    condition = true;

    if(scene_manager_get_scene_state(app->scene_manager, AppScenesniffingTestOption) == 1) {
        scene_manager_previous_scene(app->scene_manager);
        scene_manager_set_scene_state(app->scene_manager, AppScenesniffingTestOption, 0);
    }

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
                app->sniffer_index_aux,
                sniffingTest_callback,
                app);
            break;

        case EntryEvent:
            condition = false;
            scene_manager_next_scene(app->scene_manager, AppSceneboxSniffing);
            consumed = true;
            break;

        case SaveDataOk:
            if(app->save_logs == SaveAll) save_data_on_log(app);
            break;

        case DEVICE_NO_CONNECTED:
            scene_manager_next_scene(app->scene_manager, AppSceneDeviceNoConnected);
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

        if((app->save_logs == SaveAll) && (app->log_file_ready)) {
            close_file_on_data_log(app);
            app->log_file_ready = false;
        }

        submenu_reset(app->submenu);
    }
}

//-------------------------- FOR THE SNIFFING BOX --------------------------------------------------------

void draw_box_text(App* app) {
    furi_string_reset(app->text);

    furi_string_cat_printf(
        app->text,
        "ADDR: %lx DLC: %u \n",
        app->frameArray[app->sniffer_index].canId,
        app->frameArray[app->sniffer_index].data_lenght);

    for(uint8_t i = 0; i < (app->frameArray[app->sniffer_index].data_lenght); i++) {
        furi_string_cat_printf(
            app->text, "[%u]:  %x ", i, app->frameArray[app->sniffer_index].buffer[i]);
    }

    furi_string_cat_printf(app->text, "\ntime: %li ms", app->times[app->sniffer_index]);
    text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
    text_box_set_focus(app->textBox, TextBoxFocusEnd);
}

void app_scene_BoxSniffing_on_enter(void* context) {
    App* app = context;

    text_box_set_font(app->textBox, TextBoxFontText);

    if(app->save_logs == OnlyAddress) save_address_data_on_log(app);

    text_box_reset(app->textBox);
    draw_box_text(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, TextBoxView);
}

bool app_scene_BoxSniffing_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    if(event.event == ShowData) {
        draw_box_text(app);
        if(app->log_file_ready) {
            write_data_on_file(
                app->frameArray[app->sniffer_index],
                app->log_file,
                app->times[app->sniffer_index]);
        }
        consumed = true;
    }
    return consumed;
}

void app_scene_BoxSniffing_on_exit(void* context) {
    App* app = context;
    close_file_on_data_log(app);
    furi_string_reset(app->text);
    text_box_reset(app->textBox);
}

//-------------------------- FOR THE UNPLUG DEVICE --------------------------------------------------------

void app_scene_device_no_connected_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "DEVICE NO");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "CONNECTED");

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_device_no_connected_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void app_scene_device_no_connected_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
    scene_manager_set_scene_state(app->scene_manager, AppScenesniffingTestOption, 1);
}
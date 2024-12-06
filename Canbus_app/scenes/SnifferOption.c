#include "../app_user.h"

bool condition = true;

// Function to save logs
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

// Function to resolve the paths to save logs
char* sequential_file_resolve_path_address(
    Storage* storage,
    uint32_t address,
    const char* dir,
    const char* prefix,
    const char* extension) {
    DateTime datatime;
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

// Function to save logs
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

// Save the logs by address
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

// Fuction to close the files and stop saving
void close_file_on_data_log(App* app) {
    app->log_file_ready = false;
    if(app->log_file && storage_file_is_open(app->log_file)) {
        storage_file_close(app->log_file);
    }
}

// Write the frames
static void write_data_on_file(CANFRAME frame, File* file, uint32_t time) {
    FuriString* text_file = furi_string_alloc();

    furi_string_cat_printf(text_file, "(%li) %lx %u", time, frame.canId, frame.data_lenght);
    for(uint8_t i = 0; i < (frame.data_lenght); i++) {
        furi_string_cat_printf(text_file, " %x", frame.buffer[i]);
    }
    furi_string_cat_printf(text_file, "\n");
    storage_file_write(file, furi_string_get_cstr(text_file), furi_string_size(text_file));
    furi_string_reset(text_file);
    furi_string_free(text_file);
}

// Callback for event
void sniffing_callback(void* context, uint32_t index) {
    App* app = context;
    app->sniffer_index = index;
    scene_manager_handle_custom_event(app->scene_manager, EntryEvent);
}

// Draw the text for the sniffing
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

// Thread to sniff
static int32_t worker_sniffing(void* context);

/**
 * Scene to choose the id to sniff
 */
// Scene on enter
void app_scene_sniffing_on_enter(void* context) {
    App* app = context;

    if(condition) {
        app->thread = furi_thread_alloc_ex("SniffingWork", 1024, worker_sniffing, app);
        furi_thread_start(app->thread);
        submenu_reset(app->submenu);
        submenu_set_header(app->submenu, "CANBUS ADDRESS");
    }

    condition = true;

    if(scene_manager_get_scene_state(app->scene_manager, app_scene_sniffing_option) == 1) {
        scene_manager_previous_scene(app->scene_manager);
        scene_manager_set_scene_state(app->scene_manager, app_scene_sniffing_option, 0);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on event
bool app_scene_sniffing_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case EntryEvent:
            condition = false;
            scene_manager_next_scene(app->scene_manager, app_scene_box_sniffing);
            consumed = true;
            break;

        case DEVICE_NO_CONNECTED:
            scene_manager_set_scene_state(app->scene_manager, app_scene_sniffing_option, 1);
            scene_manager_next_scene(app->scene_manager, app_scene_device_no_connected);
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

// Scene on exit
void app_scene_sniffing_on_exit(void* context) {
    App* app = context;
    if(condition) {
        furi_thread_join(app->thread);
        furi_thread_free(app->thread);
        submenu_reset(app->submenu);
    }
}

/**
 * Scene for show the sniffing
 */

// Scene on enter
void app_scene_box_sniffing_on_enter(void* context) {
    App* app = context;

    text_box_set_font(app->textBox, TextBoxFontText);

    if(app->save_logs == OnlyAddress) save_address_data_on_log(app);

    text_box_reset(app->textBox);
    draw_box_text(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, TextBoxView);
}

// Scene on event
bool app_scene_box_sniffing_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);
    return consumed;
}

// Scene on exit
void app_scene_box_sniffing_on_exit(void* context) {
    App* app = context;
    close_file_on_data_log(app);
    furi_string_reset(app->text);
    text_box_reset(app->textBox);
}

/**
 * Thread to sniff
 */

static int32_t worker_sniffing(void* context) {
    App* app = context;
    MCP2515* mcp_can = app->mcp_can;
    CANFRAME frame = app->can_frame;
    FuriString* text_label = furi_string_alloc();

    uint8_t num_of_devices = 0;
    uint32_t current_time = 0;

    bool run = true;
    bool first_address = true;

    mcp_can->mode = MCP_LISTENONLY;
    ERROR_CAN debugStatus = mcp2515_init(mcp_can);

    if(debugStatus != ERROR_OK) {
        run = false;
        view_dispatcher_send_custom_event(app->view_dispatcher, DEVICE_NO_CONNECTED);
    }

    while(run) {
        bool new = true;

        if(read_can_message(mcp_can, &frame) == ERROR_OK) {
            current_time = furi_get_tick();

            if(first_address) {
                app->frameArray[num_of_devices] = frame;
                app->times[num_of_devices] = 0;
                app->sniffer_index_aux = num_of_devices;
                app->times[num_of_devices] = 0;
                app->time = 0;
                app->current_time[num_of_devices] = furi_get_tick();
                num_of_devices++;
                first_address = false;

                furi_string_reset(text_label);
                furi_string_cat_printf(
                    text_label, "0x%lx", app->frameArray[app->sniffer_index_aux].canId);

                submenu_add_item(
                    app->submenu,
                    furi_string_get_cstr(text_label),
                    app->sniffer_index_aux,
                    sniffing_callback,
                    app);

                if(app->save_logs == SaveAll) {
                    save_data_on_log(app);
                }
            }

            if(condition) {
                for(uint8_t i = 0; i < num_of_devices; i++) {
                    if(frame.canId == app->frameArray[i].canId) {
                        app->frameArray[i] = frame;
                        app->times[i] = furi_get_tick() - app->current_time[i];
                        app->time = app->times[i];
                        app->current_time[i] = furi_get_tick();
                        new = false;
                        break;
                    }
                }

                if(new && (num_of_devices < 100)) {
                    app->frameArray[num_of_devices] = frame;
                    app->times[num_of_devices] = 0;
                    app->sniffer_index_aux = num_of_devices;
                    app->times[num_of_devices] = 0;
                    app->time = 0;
                    app->current_time[num_of_devices] = furi_get_tick();
                    num_of_devices++;

                    furi_string_reset(text_label);
                    furi_string_cat_printf(
                        text_label, "0x%lx", app->frameArray[app->sniffer_index_aux].canId);

                    submenu_add_item(
                        app->submenu,
                        furi_string_get_cstr(text_label),
                        app->sniffer_index_aux,
                        sniffing_callback,
                        app);
                }

                if(app->log_file_ready && (app->save_logs == SaveAll)) {
                    app->can_frame = frame;
                    write_data_on_file(app->can_frame, app->log_file, current_time);
                }
            }

            if(!condition) {
                if(frame.canId == app->frameArray[app->sniffer_index].canId) {
                    app->frameArray[app->sniffer_index] = frame;
                    app->times[app->sniffer_index] =
                        furi_get_tick() - app->current_time[app->sniffer_index];
                    app->current_time[app->sniffer_index] = furi_get_tick();
                    draw_box_text(app);

                    if(app->log_file_ready && (app->save_logs == OnlyAddress)) {
                        write_data_on_file(
                            app->frameArray[app->sniffer_index],
                            app->log_file,
                            app->current_time[app->sniffer_index]);
                    }
                }
            }

            app->num_of_devices = num_of_devices;
        }

        if(condition && !furi_hal_gpio_read(&gpio_button_back)) {
            break;
        }
        furi_delay_ms(1);
    }

    if((app->save_logs == SaveAll) && (app->log_file_ready)) {
        close_file_on_data_log(app);
        app->log_file_ready = false;
    }

    furi_string_free(text_label);
    free_mcp2515(mcp_can);
    return 0;
}

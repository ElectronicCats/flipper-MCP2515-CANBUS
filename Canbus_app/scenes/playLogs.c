#include "../app_user.h"
#define TAG "PLAY "
#define MAX_UNIQUE_IDS 100

typedef enum {
  PLAY_OK,
  PLAY_ERROR,
} player_status;

typedef enum {
  TIMING_TIMESTAMP,
  TIMING_CUSTOM,
  TIMING_DEFAULT,
} timing;

//typedef struct {
//    uint32_t id;
//    bool enabled;
//} UniqueId;

// Timing configuration
const uint8_t config_timing_values[] = {
    0x00,
    0x01,
    0x02,
};
const char* const config_timing_names[] = {
    "DEFAULT",
    "CUSTOM",
    "TIMESTAMP",
};

uint32_t hex_to_int(const char* hex_str) {
    unsigned int result = 0;
    sscanf(hex_str, "%x", &result);
    return (uint32_t)result;
}

// Function prototypes
void select_log_file(App* app);
void get_unique_ids(App* app, UniqueId* unique_ids, int* unique_id_count);
void play_data_frames(App* app, UniqueId* unique_ids, int unique_id_count);

// Options Callback
void callback_timing_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    app->config_timing_index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, config_timing_names[app->config_timing_index]);
}

void callback_id_changed(VariableItem* item) {
    UniqueId* id = variable_item_get_context(item);
    id->enabled = !id->enabled;
    variable_item_set_current_value_index(item, id->enabled ? 1 : 0);
    variable_item_set_current_value_text(item, id->enabled ? "ON" : "OFF");
}

// Scene 1: File selection
void app_scene_file_select_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "Press OK to select log file");
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InputKeyOk) {
            select_log_file(app);
            scene_manager_next_scene(app->scene_manager, AppSceneIdSelection);
            consumed = true;
        }
    }
    return consumed;
}

// Scene 2: ID selection and timing configuration
void app_scene_id_selection_on_enter(void* context) {
    App* app = context;
    UniqueId unique_ids[MAX_UNIQUE_IDS];
    int unique_id_count = 0;

    get_unique_ids(app, unique_ids, &unique_id_count);

    variable_item_list_reset(app->varList);

    // Add timing option
    VariableItem* timing_item = variable_item_list_add(
        app->varList,
        "Timing",
        COUNT_OF(config_timing_names),
        callback_timing_changed,
        app);
    variable_item_set_current_value_index(timing_item, app->config_timing_index);
    variable_item_set_current_value_text(timing_item, config_timing_names[app->config_timing_index]);

    // Add ID options
    for(int i = 0; i < unique_id_count; i++) {
        char id_str[10];
        snprintf(id_str, sizeof(id_str), "0x%lX", unique_ids[i].id);
        VariableItem* id_item = variable_item_list_add(
            app->varList,
            id_str,
            2,
            callback_id_changed,
            &unique_ids[i]);
        variable_item_set_current_value_index(id_item, unique_ids[i].enabled ? 1 : 0);
        variable_item_set_current_value_text(id_item, unique_ids[i].enabled ? "ON" : "OFF");
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

bool app_scene_id_selection_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InputKeyOk) {
            play_data_frames(app, app->unique_ids, app->unique_id_count);
            consumed = true;
        }
    }
    return consumed;
}

// Function to select log file
void select_log_file(App* app) {
    FuriString* predefined_filepath = furi_string_alloc_set_str(PATHAPP);
    FuriString* selected_filepath = furi_string_alloc();
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".log", NULL);
    browser_options.base_path = PATHAPP;

    dialog_file_browser_show(app->dialogs, selected_filepath, predefined_filepath, &browser_options);

    if(storage_file_open(
        app->log_file, furi_string_get_cstr(selected_filepath), FSAM_READ, FSOM_OPEN_EXISTING)) {
        // File opened successfully
        //app->log_filepath = furi_string_alloc_set(selected_filepath);
    } else {
        dialog_message_show_storage_error(app->dialogs, "Cannot open File");
        // TODO: something
    }

    furi_string_free(selected_filepath);
    furi_string_free(predefined_filepath);
}

// Function to get unique IDs
void get_unique_ids(App* app, UniqueId* unique_ids, int* unique_id_count) {
    char buffer[256];
    size_t buffer_index = 0;
    char c;
    *unique_id_count = 0;

    storage_file_seek(app->log_file, 0, true);

    while(storage_file_read(app->log_file, &c, 1) == 1) {
        if(c == '\n' || buffer_index >= sizeof(buffer) - 1) {
            buffer[buffer_index] = '\0';

            char* saveptr;
            char* token = strtok_r(buffer, "() ", &saveptr);
            if(!token) continue;

            token = strtok_r(NULL, " ", &saveptr);
            if(!token) continue;

            uint32_t can_id = (uint32_t)strtoul(token, NULL, 16);

            bool id_exists = false;
            for(int i = 0; i < *unique_id_count; i++) {
                if(unique_ids[i].id == can_id) {
                    id_exists = true;
                    break;
                }
            }
            if(!id_exists && *unique_id_count < MAX_UNIQUE_IDS) {
                unique_ids[*unique_id_count].id = can_id;
                unique_ids[*unique_id_count].enabled = true;
                (*unique_id_count)++;
            }

            buffer_index = 0;
        } else {
            buffer[buffer_index++] = c;
        }
    }

    storage_file_seek(app->log_file, 0, true);
}


// Function to play data frames
void play_data_frames(App* app, UniqueId* unique_ids, int unique_id_count) {
    app->mcp_can->mode = MCP_NORMAL;
    ERROR_CAN debug = mcp2515_init(app->mcp_can);

    if(debug != ERROR_OK) {
        scene_manager_handle_custom_event(app->scene_manager, DEVICE_NO_CONNECTED);
        return;
    }

    char buffer[256];
    size_t buffer_index = 0;
    char c;
    uint32_t previous_timing = 0;

    while(storage_file_read(app->log_file, &c, 1) == 1) {
        if(c == '\n' || buffer_index >= sizeof(buffer) - 1) {
            buffer[buffer_index] = '\0';

            CANFRAME frame_to_send = {0};
            char* saveptr;
            char* token;
            int time_to_next_frame = 0;
            float timestamp;

            // Parse timestamp
            token = strtok_r(buffer, "() ", &saveptr);
            if(!token) continue;
            timestamp = atof(token);
            uint32_t current_timing = (uint32_t)((timestamp - previous_timing) * 1000);
            previous_timing = timestamp;

            // Parse CAN ID
            token = strtok_r(NULL, " ", &saveptr);
            if(!token) continue;
            frame_to_send.canId = (uint32_t)strtoul(token, NULL, 16);

            // Check if this ID is enabled
            bool id_enabled = false;
            for(int i = 0; i < unique_id_count; i++) {
                if(unique_ids[i].id == frame_to_send.canId && unique_ids[i].enabled) {
                    id_enabled = true;
                    break;
                }
            }
            if(!id_enabled) continue;

            // Parse data length
            token = strtok_r(NULL, " ", &saveptr);
            if(!token) continue;
            frame_to_send.data_lenght = (uint8_t)atoi(token);

            // Parse data
            for(int i = 0; i < frame_to_send.data_lenght && i < MAX_LEN; i++) {
                token = strtok_r(NULL, " ", &saveptr);
                if(!token) break;
                frame_to_send.buffer[i] = (uint8_t)strtoul(token, NULL, 16);
            }

            // Parse custom timing if available
            token = strtok_r(NULL, ",", &saveptr);
            if(token) {
                time_to_next_frame = atoi(token);
            }

            ERROR_CAN error = send_can_frame(app->mcp_can, &frame_to_send);

            switch(app->config_timing_index) {
                case TIMING_TIMESTAMP:
                    furi_delay_ms(current_timing);
                    break;
                case TIMING_CUSTOM:
                    furi_delay_ms(time_to_next_frame);
                    break;
                case TIMING_DEFAULT:
                    furi_delay_ms(500);
                    break;
            }

            if(error != ERROR_OK) {
                scene_manager_handle_custom_event(app->scene_manager, PLAY_ERROR);
            } else {
                scene_manager_handle_custom_event(app->scene_manager, PLAY_OK);
            }

            buffer_index = 0;
        } else {
            buffer[buffer_index++] = c;
        }
    }

    storage_file_seek(app->log_file, 0, true);
}

void play_data_frames_bk(void* context, int frame_interval) {

    App* app = context;

    app->mcp_can->mode = MCP_NORMAL;
    ERROR_CAN debug = ERROR_OK;
    ERROR_CAN error = ERROR_OK;
    debug = mcp2515_init(app->mcp_can);

    FuriString* predefined_filepath = furi_string_alloc_set_str(PATHAPP);
    FuriString* selected_filepath = furi_string_alloc();
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".log", NULL);
    browser_options.base_path = PATHAPP;

    dialog_file_browser_show(app->dialogs, selected_filepath, predefined_filepath, &browser_options);

    if(storage_file_open(
            app->log_file, furi_string_get_cstr(selected_filepath), FSAM_READ, FSOM_OPEN_EXISTING)) {

            char buffer[256];
            size_t buffer_index = 0;
            size_t bytes_read;
            char c;

            uint32_t previous_timing = 0;
            uint32_t current_timing;

            unsigned int unique_ids[MAX_UNIQUE_IDS] = {0};
            int unique_id_count = 0;

            // Primera pasada: recolectar IDs únicos
            while ((bytes_read = storage_file_read(app->log_file, &c, 1)) > 0) {
                if (c == '\n' || buffer_index >= 256 - 1) {
                    buffer[buffer_index] = '\0';

                    char *saveptr;
                    char *token;

                    // Pass timestamp
                    token = strtok_r(buffer, "() ", &saveptr);
                    if (!token) continue;

                    // Get ID
                    token = strtok_r(NULL, " ", &saveptr);
                    if (!token) continue;

                    uint32_t can_id = hex_to_int(token);

                    // Add ID to unique ID list if not present
                    bool id_exists = false;
                    for (int i = 0; i < unique_id_count; i++) {
                        if (unique_ids[i] == can_id) {
                            id_exists = true;
                            break;
                        }
                    }
                    if (!id_exists && unique_id_count < MAX_UNIQUE_IDS) {
                        unique_ids[unique_id_count++] = can_id;
                    }

                    buffer_index = 0;
                } else {
                    buffer[buffer_index++] = c;
                }
            }

            // Print unique IDs
            FURI_LOG_I(TAG, "IDs únicos encontrados:");
            for (int i = 0; i < unique_id_count; i++) {
                FURI_LOG_I(TAG, "0x%X", unique_ids[i]);
            }
            FURI_LOG_I(TAG, "Total de IDs únicos: %d", unique_id_count);

            // Restart file
            storage_file_seek(app->log_file, 0, true);
            

            buffer_index = 0;

            while ((bytes_read = storage_file_read(app->log_file, &c, 1)) > 0) { // && model->flag_signal) {
                if (c == '\n' || buffer_index >= 256 - 1) {
                    buffer[buffer_index] = '\0';

                    FURI_LOG_E(TAG,"%s\n", buffer);

                    buffer[sizeof(buffer) - 1] = '\0';  // Ensure the string is null-terminated

                    CANFRAME frame_to_send = {0};  // Initialize all fields to 0
                    char *saveptr;
                    char *token;
                    int time_to_next_frame = 0;
                    float timestamp;

                    UNUSED(timestamp);

                    // Timestamp
                    token = strtok_r(buffer, "() ", &saveptr);
                    if (!token) return;
                    timestamp = atof(token);
                    current_timing = timestamp - previous_timing;
                    previous_timing = timestamp;

                    // CAN bus ID
                    token = strtok_r(NULL, " ", &saveptr);
                    if (!token) return;
                    frame_to_send.canId = hex_to_int(token);

                    // Data length
                    token = strtok_r(NULL, " ", &saveptr);
                    if (!token) return;
                    frame_to_send.data_lenght = (uint8_t)atoi(token);

                    // Fill the data buffer
                    for (int i = 0; i < frame_to_send.data_lenght && i < MAX_LEN; i++) {
                        token = strtok_r(NULL, " ", &saveptr);
                        if (!token) break;
                        frame_to_send.buffer[i] = (uint8_t)hex_to_int(token);
                    }

                    token = strtok_r(NULL, ",", &saveptr);
                    if (token) {
                        time_to_next_frame = atoi(token);
                    }                    

                    if (debug == ERROR_OK) {
                        error = send_can_frame(app->mcp_can, app->frame_to_send);

                        // TODO: choose TIMING
                        switch(frame_interval) {
                            case TIMING_TIMESTAMP:
                                furi_delay_ms((uint32_t)(current_timing*1000));
                                break;
                            case TIMING_CUSTOM:
                                furi_delay_ms(time_to_next_frame);
                                break;                            
                            case TIMING_DEFAULT:
                                furi_delay_ms(500);
                                break;                           
                        }

                        if (error != ERROR_OK)
                        scene_manager_handle_custom_event(app->scene_manager, PLAY_ERROR);
                        else
                        scene_manager_handle_custom_event(app->scene_manager, PLAY_OK);
                    } else {
                        scene_manager_handle_custom_event(app->scene_manager, DEVICE_NO_CONNECTED);
                    }

                    buffer_index = 0;
                } else {
                    buffer[buffer_index++] = c;
                }
            }

    } else {
        dialog_message_show_storage_error(app->dialogs, "Cannot open File");
    }
    storage_file_close(app->log_file);

    furi_string_free(selected_filepath);
    furi_string_free(predefined_filepath);

    // furi_hal_gpio_write(pin_led, true);
    // furi_delay_ms(50);
    // furi_hal_gpio_write(pin_led, false);

}

static uint32_t callback_back_navigation_submenu(void* _context) {
    App* app = _context;
    UNUSED(app);
    //view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
    return SubmenuView;
}

// Option callback using button OK
void callback_input_player_options(void* context, uint32_t index) {
    App* app = context;
    UNUSED(index);
    UNUSED(app);
    //scene_manager_next_scene(app->scene_manager, app_scene_send_message);
    //play_data_frames(app, app->config_timing_index);
}

void app_scene_player_on_enter(void* context) {
    App* app = context;
    VariableItem* item;

    // Timing options
    item = variable_item_list_add(
        app->varList,
        "Timing",
        COUNT_OF(config_timing_values),
        callback_timing_changed,
        app);
    uint8_t config_timing_index = 0;
    variable_item_set_current_value_index(item, config_timing_index);
    variable_item_set_current_value_text(item, config_timing_names[config_timing_index]);
    
    variable_item_list_set_enter_callback(app->varList, callback_input_player_options, app);
    //variable_item_list_set_selected_item(app->varList, app->player_selected_item);
    view_set_previous_callback(
        variable_item_list_get_view(app->varList),
        callback_back_navigation_submenu);
    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);

    // widget_reset(app->widget);
    // widget_add_string_element(
    //     app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "Wait to send LOG file...");

    // view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);

    //app->thread = furi_thread_alloc_ex("Sender_on_work", 1024, sender_on_work, app);
    //furi_thread_start(app->thread);
}

void app_scene_player_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}

bool app_scene_player_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case PLAY_OK:
            widget_reset(app->widget);
            widget_add_string_element(
                app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "LOG PLAYBACK OK");
            break;

        case PLAY_ERROR:
            widget_reset(app->widget);
            widget_add_string_element(
                app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "LOG PLAYBACK ERROR");
            break;

        case DEVICE_NO_CONNECTED:
            widget_reset(app->widget);

            widget_add_string_element(
                app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "DEVICE NO");

            widget_add_string_element(
                app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "CONNECTED");
            break;

        default:
            break;
        }
    }
    return consumed;
}
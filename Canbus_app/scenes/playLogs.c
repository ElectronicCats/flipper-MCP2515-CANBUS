#include "../app_user.h"
#define TAG            "PLAY "
#define MAX_UNIQUE_IDS 100

typedef struct {
    uint32_t id;
    bool enabled;
} UniqueId;

typedef enum {
    PLAY_OK,
    PLAY_ERROR,
} player_status;

typedef enum {
    TIMING_CUSTOM,
    TIMING_DEFAULT,
    TIMING_TIMESTAMP,
} timing;

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

char* custom_strtok_r(char* str, const char* delim, char** saveptr) {
    if(str) {
        *saveptr = str;
    }
    if(!*saveptr) {
        return NULL;
    }

    char* start = *saveptr;
    while(*start && strchr(delim, *start)) {
        ++start;
    }
    if(*start == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    char* end = start;
    while(*end && !strchr(delim, *end)) {
        ++end;
    }

    if(*end) {
        *end = '\0';
        *saveptr = end + 1;
    } else {
        *saveptr = NULL;
    }

    return start;
}

void path_file_name(const char* path, FuriString* file_name);
void path_file_name(const char* path, FuriString* file_name) {
    uint8_t last_pos = 0;
    for(uint8_t i = 0; path[i] != '\0'; i++) {
        if(path[i] == '/') last_pos = i + 1;
    }

    furi_string_reset(file_name);

    for(uint8_t i = last_pos; path[i] != '\0'; i++) {
        furi_string_cat_printf(file_name, "%c", path[i]);
    }
}

void play_data_frames(App* app, UniqueId* unique_ids, int unique_id_count);
void play_data_frames_bk(void* context, int frame_interval);

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
            char* endptr;
            char* token;
            int time_to_next_frame = 0;
            double timestamp;

            // Parse timestamp
            token = custom_strtok_r(buffer, "() ", &saveptr);
            if(!token) continue;
            timestamp = strtod(token, &endptr);
            uint32_t current_timing = (uint32_t)((timestamp - previous_timing) * 1000);
            previous_timing = timestamp;

            // Parse CAN ID
            token = custom_strtok_r(NULL, " ", &saveptr);
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
            token = custom_strtok_r(NULL, " ", &saveptr);
            if(!token) continue;
            frame_to_send.data_lenght = (uint8_t)atoi(token);

            // Parse data
            for(int i = 0; i < frame_to_send.data_lenght && i < MAX_LEN; i++) {
                token = custom_strtok_r(NULL, " ", &saveptr);
                if(!token) break;
                frame_to_send.buffer[i] = (uint8_t)strtoul(token, NULL, 16);
            }

            // Parse custom timing if available
            token = custom_strtok_r(NULL, ",", &saveptr);
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

/*
void play_data_frames_bk(void* context, int frame_interval) {
    App* app = context;

    app->mcp_can->mode = MCP_NORMAL;
    ERROR_CAN debug = ERROR_OK;
    ERROR_CAN error = ERROR_OK;
    debug = mcp2515_init(app->mcp_can);

    if(storage_file_open(
           app->log_file, furi_string_get_cstr(app->data), FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buffer[256];
        size_t buffer_index = 0;
        size_t bytes_read;
        char c;

        uint32_t previous_timing = 0;

        unsigned int unique_ids[MAX_UNIQUE_IDS] = {0};
        int unique_id_count = 0;

        // Primera pasada: recolectar IDs únicos
        while((bytes_read = storage_file_read(app->log_file, &c, 1)) > 0) {
            if(c == '\n' || buffer_index >= 256 - 1) {
                buffer[buffer_index] = '\0';

                char* saveptr;
                char* token;

                // Pass timestamp
                token = custom_strtok_r(buffer, "() ", &saveptr);
                if(!token) continue;

                // Get ID
                token = custom_strtok_r(NULL, " ", &saveptr);
                if(!token) continue;

                uint32_t can_id = hex_to_int(token);

                // Add ID to unique ID list if not present
                bool id_exists = false;
                for(int i = 0; i < unique_id_count; i++) {
                    if(unique_ids[i] == can_id) {
                        id_exists = true;
                        break;
                    }
                }
                if(!id_exists && unique_id_count < MAX_UNIQUE_IDS) {
                    unique_ids[unique_id_count++] = can_id;
                }

                buffer_index = 0;
            } else {
                buffer[buffer_index++] = c;
            }
        }

        // Print unique IDs
        FURI_LOG_I(TAG, "IDs únicos encontrados:");
        for(int i = 0; i < unique_id_count; i++) {
            FURI_LOG_I(TAG, "0x%X", unique_ids[i]);
        }
        FURI_LOG_I(TAG, "Total de IDs únicos: %d", unique_id_count);

        // Restart file
        storage_file_seek(app->log_file, 0, true);

        buffer_index = 0;

        while((bytes_read = storage_file_read(app->log_file, &c, 1)) >
              0) { // && model->flag_signal) {
            if(c == '\n' || buffer_index >= 256 - 1) {
                buffer[buffer_index] = '\0';

                FURI_LOG_E(TAG, "%s\n", buffer);

                buffer[sizeof(buffer) - 1] = '\0'; // Ensure the string is null-terminated

                CANFRAME frame_to_send = {0}; // Initialize all fields to 0
                char* saveptr;
                char* endptr;
                char* token;
                int time_to_next_frame = 0;
                double timestamp;

                // Timestamp
                token = custom_strtok_r(buffer, "() ", &saveptr);
                if(!token) return;
                timestamp = strtod(token, &endptr);
                uint32_t current_timing = (uint32_t)((timestamp - previous_timing) * 1000);
                previous_timing = timestamp;

                // CAN bus ID
                token = custom_strtok_r(NULL, " ", &saveptr);
                if(!token) return;
                frame_to_send.canId = hex_to_int(token);

                // Data length
                token = custom_strtok_r(NULL, " ", &saveptr);
                if(!token) return;
                frame_to_send.data_lenght = (uint8_t)atoi(token);

                // Fill the data buffer
                for(int i = 0; i < frame_to_send.data_lenght && i < MAX_LEN; i++) {
                    token = custom_strtok_r(NULL, " ", &saveptr);
                    if(!token) break;
                    frame_to_send.buffer[i] = (uint8_t)hex_to_int(token);
                }

                token = custom_strtok_r(NULL, ",", &saveptr);
                if(token) {
                    time_to_next_frame = atoi(token);
                }

                if(debug == ERROR_OK) {
                    error = send_can_frame(app->mcp_can, app->frame_to_send);

                    // TODO: choose TIMING
                    switch(frame_interval) {
                    case TIMING_TIMESTAMP:
                        furi_delay_ms((uint32_t)(current_timing * 1000));
                        break;
                    case TIMING_CUSTOM:
                        furi_delay_ms(time_to_next_frame);
                        break;
                    case TIMING_DEFAULT:
                        furi_delay_ms(500);
                        break;
                    }

                    if(error != ERROR_OK)
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
    }
    storage_file_close(app->log_file);
}
*/

void play_data_frames_bk(void* context, int frame_interval) {
    App* app = context;

    log_info("Here 0");

    app->mcp_can->mode = MCP_NORMAL;
    ERROR_CAN debug = ERROR_OK;
    // ERROR_CAN error = ERROR_OK;
    debug = mcp2515_init(app->mcp_can);

    if(storage_file_open(
           app->log_file, furi_string_get_cstr(app->path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        log_info("File Open");
        char buffer[256];
        size_t buffer_index = 0;
        size_t bytes_read;
        char c;

        uint32_t previous_timing = 0;

        unsigned int unique_ids[MAX_UNIQUE_IDS] = {0};
        int unique_id_count = 0;

        // Primera pasada: recolectar IDs únicos
        while((bytes_read = storage_file_read(app->log_file, &c, 1)) > 0) {
            if(c == '\n' || buffer_index >= 256 - 1) {
                buffer[buffer_index] = '\0';

                char* saveptr;
                char* token;

                // Pass timestamp
                token = custom_strtok_r(buffer, "() ", &saveptr);
                if(!token) continue;

                // Get ID
                token = custom_strtok_r(NULL, " ", &saveptr);
                if(!token) continue;

                uint32_t can_id = hex_to_int(token);

                // Add ID to unique ID list if not present
                bool id_exists = false;
                for(int i = 0; i < unique_id_count; i++) {
                    if(unique_ids[i] == can_id) {
                        id_exists = true;
                        break;
                    }
                }
                if(!id_exists && unique_id_count < MAX_UNIQUE_IDS) {
                    unique_ids[unique_id_count++] = can_id;
                }

                buffer_index = 0;
            } else {
                buffer[buffer_index++] = c;
            }
        }

        // Print unique IDs
        FURI_LOG_I(TAG, "IDs únicos encontrados:");
        for(int i = 0; i < unique_id_count; i++) {
            FURI_LOG_I(TAG, "0x%X", unique_ids[i]);
        }
        FURI_LOG_I(TAG, "Total de IDs únicos: %d", unique_id_count);

        // Restart file
        storage_file_seek(app->log_file, 0, true);

        buffer_index = 0;

        while((bytes_read = storage_file_read(app->log_file, &c, 1)) >
              0) { // && model->flag_signal) {
            if(c == '\n' || buffer_index >= 256 - 1) {
                buffer[buffer_index] = '\0';

                FURI_LOG_E(TAG, "%s\n", buffer);

                buffer[sizeof(buffer) - 1] = '\0'; // Ensure the string is null-terminated

                CANFRAME frame_to_send = {0}; // Initialize all fields to 0
                char* saveptr;
                char* endptr;
                char* token;
                int time_to_next_frame = 0;
                double timestamp;

                // Timestamp
                token = custom_strtok_r(buffer, "() ", &saveptr);
                if(!token) return;
                timestamp = strtod(token, &endptr);
                uint32_t current_timing = (uint32_t)((timestamp - previous_timing) * 1000);
                previous_timing = timestamp;

                // CAN bus ID
                token = custom_strtok_r(NULL, " ", &saveptr);
                if(!token) return;
                frame_to_send.canId = hex_to_int(token);

                // Data length
                token = custom_strtok_r(NULL, " ", &saveptr);
                if(!token) return;
                frame_to_send.data_lenght = (uint8_t)atoi(token);

                // Fill the data buffer
                for(int i = 0; i < frame_to_send.data_lenght && i < MAX_LEN; i++) {
                    token = custom_strtok_r(NULL, " ", &saveptr);
                    if(!token) break;
                    frame_to_send.buffer[i] = (uint8_t)hex_to_int(token);
                }

                token = custom_strtok_r(NULL, ",", &saveptr);
                if(token) {
                    time_to_next_frame = atoi(token);
                }

                if(debug == ERROR_OK) {
                    log_info(
                        "%lx %u %x %x %x %x %x %x %x %x",
                        frame_to_send.canId,
                        frame_to_send.data_lenght,
                        frame_to_send.buffer[0],
                        frame_to_send.buffer[1],
                        frame_to_send.buffer[2],
                        frame_to_send.buffer[3],
                        frame_to_send.buffer[4],
                        frame_to_send.buffer[5],
                        frame_to_send.buffer[6],
                        frame_to_send.buffer[7]);

                    send_can_frame(app->mcp_can, &frame_to_send);

                    // TODO: choose TIMING
                    switch(frame_interval) {
                    case TIMING_TIMESTAMP:
                        furi_delay_ms((uint32_t)(current_timing));
                        break;
                    case TIMING_CUSTOM:
                        furi_delay_ms(time_to_next_frame);
                        break;
                    case TIMING_DEFAULT:
                        furi_delay_ms(1000);
                        break;
                    }
                }
                buffer_index = 0;
            } else {
                buffer[buffer_index++] = c;
            }
        }

    } else {
        log_info("File Not Open");
    }
    storage_file_close(app->log_file);
}

// Thread work
int32_t thread_play_logs(void* context) {
    App* app = context;

    log_info("Entra al hilo");

    play_data_frames_bk(app, TIMING_DEFAULT);

    return 0;
}

// Option callback using button OK
void callback_input_player_options(void* context, uint32_t index) {
    App* app = context;

    UNUSED(app);

    switch(index) {
    case 0:
        scene_manager_next_scene(app->scene_manager, app_scene_file_browser_option);
        break;

    case 2:
        scene_manager_next_scene(app->scene_manager, app_scene_play_logs_widget);
        break;

    default:
        break;
    }
}

// Options Callback
void callback_player_timing_options(VariableItem* item) {
    App* app = variable_item_get_context(item);

    uint8_t selected_index = variable_item_list_get_selected_item_index(app->varList);

    switch(selected_index) {
    case 1:
        uint8_t index = variable_item_get_current_value_index(item);
        variable_item_set_current_value_text(item, config_timing_names[index]);

        app->config_timing_index = index;
        break;

    default:
        break;
    }
}

// The function to enter a Scene
void app_scene_play_logs_on_enter(void* context) {
    App* app = context;

    VariableItem* item;

    // reset list
    variable_item_list_reset(app->varList);

    // Choose File
    item = variable_item_list_add(app->varList, "File", 0, NULL, app);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->data));

    // Timing options
    item = variable_item_list_add(
        app->varList,
        "Timing",
        COUNT_OF(config_timing_values),
        callback_player_timing_options,
        app);

    uint8_t config_timing_index = 0;
    app->config_timing_index = config_timing_index;
    variable_item_set_current_value_index(item, config_timing_index);
    variable_item_set_current_value_text(item, config_timing_names[config_timing_index]);

    // Play the logs
    item = variable_item_list_add(app->varList, "Play", 0, NULL, app);

    // Set the enter callback
    variable_item_list_set_enter_callback(app->varList, callback_input_player_options, app);

    // Switch View
    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

// The function to exit the scene
void app_scene_play_logs_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}

// The function for the events
bool app_scene_play_logs_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    UNUSED(event);
    UNUSED(app);
    return consumed;
}

// File browser callback
void file_browser_callback(void* context) {
    App* app = context;
    path_file_name(furi_string_get_cstr(app->path), app->data);
    scene_manager_previous_scene(app->scene_manager);
}

// File Browser
void app_scene_file_browser_on_enter(void* context) {
    App* app = context;
    file_browser_configure(app->file_browser, ".log", PATHLOGS, true, true, NULL, true);
    file_browser_set_callback(app->file_browser, file_browser_callback, app);

    furi_string_reset(app->text);
    furi_string_cat(app->text, PATHLOGS);

    file_browser_start(app->file_browser, app->text);
    view_dispatcher_switch_to_view(app->view_dispatcher, FileBrowserView);
}

bool app_scene_file_browser_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    UNUSED(event);
    UNUSED(app);
    return consumed;
}

void app_scene_file_browser_on_exit(void* context) {
    App* app = context;
    UNUSED(app);
    file_browser_stop(app->file_browser);
}

// Test widget
void app_scene_play_logs_widget_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 62, 32, AlignCenter, AlignCenter, FontPrimary, "On Dev");

    app->thread = furi_thread_alloc_ex("PlayLogs", 1024 * 20, thread_play_logs, app);
    furi_thread_start(app->thread);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_play_logs_widget_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    UNUSED(event);
    UNUSED(app);
    return consumed;
}

void app_scene_play_logs_widget_on_exit(void* context) {
    App* app = context;

    furi_thread_join(app->thread);
    furi_thread_free(app->thread);

    widget_reset(app->widget);
}

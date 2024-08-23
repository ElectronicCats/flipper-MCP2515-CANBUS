#include "../app_user.h"
#define TAG "PLAY "

typedef enum {
  PLAY_OK,
  PLAY_ERROR,
} player_status;

uint32_t hex_to_int(const char* hex_str) {
    unsigned int result = 0;
    sscanf(hex_str, "%x", &result);
    return (uint32_t)result;
}

void send_data_frame(void* context) {

    App* app = context;

    app->mcp_can->mode = MCP_NORMAL;
    ERROR_CAN debug = ERROR_OK;
    ERROR_CAN error = ERROR_OK;
    debug = mcp2515_init(app->mcp_can);

    // Storage* storage;
    // DialogsApp* dialogs;
    // File* log_file;

    FuriString* predefined_filepath = furi_string_alloc_set_str(PATHAPP);
    FuriString* selected_filepath = furi_string_alloc();
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".log", NULL);
    browser_options.base_path = PATHAPP;

    dialog_file_browser_show(app->dialogs, selected_filepath, predefined_filepath, &browser_options);

    if(storage_file_open(
            app->log_file, furi_string_get_cstr(selected_filepath), FSAM_READ, FSOM_OPEN_EXISTING)) {

            // app->flag_tx_file = true;
            // app->test = 1;

            char buffer[256];
            size_t buffer_index = 0;
            size_t bytes_read;
            char c;

            while ((bytes_read = storage_file_read(app->log_file, &c, 1)) > 0) { // && model->flag_signal) {
                if (c == '\n' || buffer_index >= 256 - 1) {
                    buffer[buffer_index] = '\0';

                    FURI_LOG_E(TAG,"%s\n", buffer);

                    buffer[sizeof(buffer) - 1] = '\0';  // Ensure the string is null-terminated

                    CANFRAME frame_to_send = {0};  // Initialize all fields to 0
                    char *saveptr;
                    char *token;

                    // Skip the timestamp
                    token = strtok_r(buffer, " ", &saveptr);
                    if (!token) return;

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

                    if (debug == ERROR_OK) {
                        error = send_can_frame(app->mcp_can, app->frame_to_send);
                        furi_delay_ms(500);

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
    //app->test = 0;
    furi_string_free(selected_filepath);
    furi_string_free(predefined_filepath);

    // furi_hal_gpio_write(pin_led, true);
    // furi_delay_ms(50);
    // furi_hal_gpio_write(pin_led, false);

    // app->flag_file = false;

}

void app_scene_play_logs_on_enter(void* context) {
    App* app = context;
    text_box_set_font(app->textBox, TextBoxFontText);
    text_box_reset(app->textBox);
    view_dispatcher_switch_to_view(app->view_dispatcher, TextBoxView);
    text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
}

bool app_scene_play_logs_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);
    return consumed;
}

void app_scene_play_logs_on_exit(void* context) {
    App* app = context;
    furi_string_reset(app->text);
    text_box_reset(app->textBox);
}

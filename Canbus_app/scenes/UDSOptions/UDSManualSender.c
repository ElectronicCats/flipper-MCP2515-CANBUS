#include "../../app_user.h"

static uint32_t id_request = DEFAULT_ECU_REQUEST;
static uint32_t id_response = DEFAULT_ECU_RESPONSE;

static uint8_t count_of_frames = 1;
static uint8_t count_of_bytes = 1;

static uint8_t data_to_send[7] = {0};

// Thread to work
static int32_t obdii_thread_response_manual_uds_sender_on_work(void* context);

/*
    Scene uds manual sender to set the values to send
*/

void callback_input_manual_sender_uds(void* context, uint32_t index) {
    App* app = context;
    switch(index) {
    case 4:
        scene_manager_next_scene(app->scene_manager, app_scene_uds_set_data_option);
        break;

    case 5:
        scene_manager_next_scene(app->scene_manager, app_scene_uds_response_sender_option);
        break;

    default:
        break;
    }
}

void callback_manual_sender_menu(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    uint8_t selected_index = variable_item_list_get_selected_item_index(app->varList);
    FuriString* text = app->text;

    furi_string_reset(text);

    switch(selected_index) {
    case 2:
        count_of_frames = index + 1;
        variable_item_set_current_value_index(item, index);
        furi_string_cat_printf(text, "%u", count_of_frames);
        variable_item_set_current_value_text(item, furi_string_get_cstr(text));
        break;

    case 3:
        count_of_bytes = index + 1;
        variable_item_set_current_value_index(item, index);
        furi_string_cat_printf(text, "%u", count_of_bytes);
        variable_item_set_current_value_text(item, furi_string_get_cstr(text));
        break;

    default:
        break;
    }
}

void app_scene_uds_manual_sender_on_enter(void* context) {
    App* app = context;
    FuriString* text = app->text;
    VariableItem* item;

    // ID REQUEST  0
    item = variable_item_list_add(app->varList, "ID REQUEST", 0, NULL, app);
    variable_item_set_current_value_index(item, 0);
    furi_string_reset(text);
    furi_string_cat_printf(text, "%lx", id_request);
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));

    // ID TO RESPONSE    1
    item = variable_item_list_add(app->varList, "ID RESPONSE", 0, NULL, app);
    variable_item_set_current_value_index(item, 0);
    furi_string_reset(text);
    furi_string_cat_printf(text, "%lx", id_response);
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));

    // COUNT OF FRAMES TO GET   2
    item = variable_item_list_add(
        app->varList, "Frames to get", 100, callback_manual_sender_menu, app);
    variable_item_set_current_value_index(item, count_of_frames - 1);
    furi_string_reset(text);
    furi_string_cat_printf(text, "%u", count_of_frames);
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));

    // SET COUNT OF BYTES   3
    item = variable_item_list_add(app->varList, "Bytes", 7, callback_manual_sender_menu, app);
    variable_item_set_current_value_index(item, count_of_bytes - 1);
    furi_string_reset(text);
    furi_string_cat_printf(text, "%u", count_of_bytes);
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));

    // Set DATA     4
    item = variable_item_list_add(app->varList, "Set Data", 0, NULL, app);

    // Send de message     5
    item = variable_item_list_add(app->varList, "Send Message", 0, NULL, app);

    // Set the callback to the View
    variable_item_list_set_enter_callback(app->varList, callback_input_manual_sender_uds, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

bool app_scene_uds_manual_sender_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void app_scene_uds_manual_sender_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}

/*
    Scene to set the Data
*/

void input_manual_uds(void* context) {
    App* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, ReturnEvent);
}

void app_scene_uds_set_data_on_enter(void* context) {
    App* app = context;
    ByteInput* scene = app->input_byte_value;

    for(int i = count_of_bytes; i < 7; i++) {
        data_to_send[i] = 0xAA;
    }

    byte_input_set_result_callback(
        scene, input_manual_uds, NULL, app, data_to_send, count_of_bytes);
    byte_input_set_header_text(scene, "SET DATA");

    view_dispatcher_switch_to_view(app->view_dispatcher, InputByteView);
}

bool app_scene_uds_set_data_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case ReturnEvent:
            scene_manager_previous_scene(app->scene_manager);
            break;

        default:
            break;
        }
    }
    return consumed;
}

void app_scene_uds_set_data_on_exit(void* context) {
    UNUSED(context);
}

/*
    Scene for the response of the uds services
*/

void app_scene_uds_response_sender_on_enter(void* context) {
    App* app = context;
    text_box_reset(app->textBox);
    text_box_set_focus(app->textBox, TextBoxFocusEnd);

    app->thread = furi_thread_alloc_ex(
        "ManualUDS", 1024, obdii_thread_response_manual_uds_sender_on_work, app);
    furi_thread_start(app->thread);

    view_dispatcher_switch_to_view(app->view_dispatcher, TextBoxView);
}

bool app_scene_uds_response_sender_on_event(void* context, SceneManagerEvent event) {
    bool consumed = false;
    UNUSED(context);
    UNUSED(event);
    return consumed;
}

void app_scene_uds_response_sender_on_exit(void* context) {
    App* app = context;
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
    text_box_reset(app->textBox);
}

/*
    Thread to work
*/

static int32_t obdii_thread_response_manual_uds_sender_on_work(void* context) {
    App* app = context;
    MCP2515* CAN = app->mcp_can;

    FuriString* text = app->text;

    furi_string_reset(text);

    UDS_SERVICE* uds_service =
        uds_service_alloc(id_request, id_response, CAN->mode, CAN->clck, CAN->bitRate);

    bool run = uds_init(uds_service);

    uint8_t data[8];

    memset(data, 0xaa, sizeof(data));

    data[0] = count_of_bytes;

    for(uint8_t i = 1; i < (count_of_bytes + 1); i++) {
        data[i] = data_to_send[i - 1];
    }

    CANFRAME canframes[count_of_frames];

    memset(canframes, 0, sizeof(canframes));

    if(run) {
        // Time delay added to initialize
        furi_delay_ms(500);
        furi_string_printf(text, "DEVICE CONNECTED!...\n");
        furi_string_cat_printf(text, "%lx ", id_request);

        for(uint8_t i = 0; i < 8; i++) {
            furi_string_cat_printf(text, "%x ", data[i]);
        }

        furi_string_cat_printf(text, "\n");

        text_box_set_text(app->textBox, furi_string_get_cstr(text));

        if(uds_manual_service_request(uds_service, data, canframes, count_of_frames)) {
            for(uint8_t i = 0; i < count_of_frames; i++) {
                CANFRAME frame_received = canframes[i];
                if(frame_received.canId == 0x00) break;
                furi_string_cat_printf(
                    text,
                    "<-(%u) %lx %x %x %x %x %x %x %x %x\n",
                    i,
                    frame_received.canId,
                    frame_received.buffer[0],
                    frame_received.buffer[1],
                    frame_received.buffer[2],
                    frame_received.buffer[3],
                    frame_received.buffer[4],
                    frame_received.buffer[5],
                    frame_received.buffer[6],
                    frame_received.buffer[7]);
            }
            text_box_reset(app->textBox);
            text_box_set_text(app->textBox, furi_string_get_cstr(text));
        } else {
            text_box_reset(app->textBox);
            furi_string_cat_printf(text, "FAILURE TRANSMITION!!\n");
            text_box_set_text(app->textBox, furi_string_get_cstr(text));
        }
    } else {
        furi_string_printf(text, "DEVICE NOT CONNECTED!");
        text_box_set_text(app->textBox, furi_string_get_cstr(text));
    }

    free_uds(uds_service);

    return 0;
}

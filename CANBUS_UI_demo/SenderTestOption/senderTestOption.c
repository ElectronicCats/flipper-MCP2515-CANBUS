#include "../app_user.h"

// LIST
typedef enum {
    CHOOSE_ID = 0,
    SET_ID = 1,
    SET_DATA_LENGHT = 2,
    SET_REQ = 3,
    SET_DATA = 4,
} list_of_items;

uint8_t address[4] = {0, 0, 0, 0};

// Option callback using button OK
void callback_input_sender_options(void* context, uint32_t index) {
    App* app = context;
    switch(index) {
    case SET_ID:
        scene_manager_set_scene_state(app->scene_manager, AppSceneinput_text_option, SET_ID);
        scene_manager_next_scene(app->scene_manager, AppSceneinput_text_option);
        break;

    case SET_DATA:
        scene_manager_set_scene_state(app->scene_manager, AppSceneinput_text_option, SET_DATA);
        scene_manager_next_scene(app->scene_manager, AppSceneinput_text_option);
        break;

    default:
        break;
    }
}

// Options Callback
void callback_sender_options(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t selected_index = variable_item_list_get_selected_item_index(app->varList);
    uint8_t index_item = variable_item_get_current_value_index(item);
    app->sender_selected_item = selected_index;

    switch(selected_index) {
    case SET_DATA_LENGHT:
        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "%u", index_item);
        variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));
        app->frame_to_send->data_lenght = index_item;
        break;
    case SET_REQ:
        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "%u", index_item);
        variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));
        app->frame_to_send->req = index_item;
        log_info("for sender req: %u", app->frame_to_send->req);
        break;
    default:
        break;
    }
}

// Sender Scene On enter
void app_scene_SenderTest_on_enter(void* context) {
    App* app = context;
    VariableItem* item;

    app->frame_to_send->canId = address[3] | (address[2] << 8) | (address[1] << 16) |
                                (address[0] << 24);

    log_info("data [0]: %u", address[0]);
    log_info("data [1]: %u", address[1]);
    log_info("data [2]: %u", address[2]);
    log_info("data [3]: %u", address[3]);

    uint8_t data_lenght = app->frame_to_send->data_lenght;
    uint32_t can_id = app->frame_to_send->canId;
    uint8_t request = app->frame_to_send->req;

    log_info("Id: %lu", can_id);

    variable_item_list_reset(app->varList);

    // 0
    item = variable_item_list_add(app->varList, "Choose Id", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "SEARCH");

    // 1
    item = variable_item_list_add(app->varList, "Id", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "0x%lx", can_id);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 2
    item = variable_item_list_add(app->varList, "DLC", 9, callback_sender_options, app);
    variable_item_set_current_value_index(item, data_lenght);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", data_lenght);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 3
    item = variable_item_list_add(app->varList, "Request", 2, callback_sender_options, app);
    variable_item_set_current_value_index(item, request);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", request);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 4
    item = variable_item_list_add(app->varList, "DATA", 2, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "SET");

    // 5
    item = variable_item_list_add(app->varList, "BYTE [0]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[0]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 6
    item = variable_item_list_add(app->varList, "BYTE [1]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[1]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 7
    item = variable_item_list_add(app->varList, "BYTE [2]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[2]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 8
    item = variable_item_list_add(app->varList, "BYTE [3]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[3]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 9
    item = variable_item_list_add(app->varList, "BYTE [4]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[4]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 10
    item = variable_item_list_add(app->varList, "BYTE [5]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[5]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 11
    item = variable_item_list_add(app->varList, "BYTE [6]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[6]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // 12
    item = variable_item_list_add(app->varList, "BYTE [7]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);

    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->buffer[7]);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    variable_item_list_set_enter_callback(app->varList, callback_input_sender_options, app);
    variable_item_list_set_selected_item(app->varList, app->sender_selected_item);
    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

bool app_scene_SenderTest_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SET_ID:
            scene_manager_next_scene(app->scene_manager, AppSceneinput_text_option);
            consumed = true;
            break;

        case SET_DATA:
            scene_manager_next_scene(app->scene_manager, AppSceneinput_text_option);
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

void app_scene_SenderTest_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}

// ---------------------------- WARNING TO GET THE ID'S ------------------------

// ---------------------------- MENU OF THE CAN ID's ---------------------------

void app_scene_id_list_on_enter(void* context) {
    UNUSED(context);
}

bool app_scene_id_list_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void app_scene_id_list_on_exit(void* context) {
    UNUSED(context);
}

// ---------------------------- TO SET THE VALUE OF THE FRAME ------------------

void input_byte_sender_callback(void* context) {
    App* app = context;
    scene_manager_handle_custom_event(app->scene_manager, ReturnEvent);
}

void app_scene_input_text_on_enter(void* context) {
    App* app = context;
    ByteInput* scene = app->input_byte_value;

    uint32_t state = scene_manager_get_scene_state(app->scene_manager, AppSceneinput_text_option);

    switch(state) {
    case SET_ID:
        byte_input_set_result_callback(scene, input_byte_sender_callback, NULL, app, address, 4);
        break;

    case SET_DATA:
        byte_input_set_result_callback(
            scene, input_byte_sender_callback, NULL, app, app->frame_to_send->buffer, 8);
        break;

    default:
        break;
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, InputByteView);
}

bool app_scene_input_text_on_event(void* context, SceneManagerEvent event) {
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

void app_scene_input_text_on_exit(void* context) {
    UNUSED(context);
}
#include "../app_user.h"

// Options Callback
void callback_sender_options(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t selected_index = variable_item_list_get_selected_item_index(app->varList);
    app->sender_selected_item = selected_index;
}
// Sender Scene On enter
void app_scene_SenderTest_on_enter(void* context) {
    App* app = context;
    VariableItem* item;

    variable_item_list_reset(app->varList);

    // 0
    item = variable_item_list_add(app->varList, "Choose Id", 2, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "SEARCH");
    // 1
    item = variable_item_list_add(app->varList, "Id", 2, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 2
    item = variable_item_list_add(app->varList, "DLC", 2, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 3
    item = variable_item_list_add(app->varList, "Request", 2, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0");

    // 4
    item = variable_item_list_add(app->varList, "DATA", 2, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "SET");

    // 5
    item = variable_item_list_add(app->varList, "BYTE [0]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 6
    item = variable_item_list_add(app->varList, "BYTE [1]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 7
    item = variable_item_list_add(app->varList, "BYTE [2]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 8
    item = variable_item_list_add(app->varList, "BYTE [3]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 9
    item = variable_item_list_add(app->varList, "BYTE [4]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 10
    item = variable_item_list_add(app->varList, "BYTE [5]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 11
    item = variable_item_list_add(app->varList, "BYTE [6]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // 12
    item = variable_item_list_add(app->varList, "BYTE [7]", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    variable_item_list_set_selected_item(app->varList, app->sender_selected_item);
    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

bool app_scene_SenderTest_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
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

void app_scene_input_text_on_enter(void* context) {
    UNUSED(context);
}

bool app_scene_input_text_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void app_scene_input_text_on_exit(void* context) {
    UNUSED(context);
}
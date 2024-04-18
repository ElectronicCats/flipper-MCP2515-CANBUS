#include "../app_user.h"


// Options Callback
void callback_sender_options(VariableItem* Item) {
    UNUSED(Item);
}

// Sender Scene On enter
void app_scene_SenderTest_on_enter(void* context) {
    App* app = context;
    VariableItem* item;

    // Reset the Variable Item List
    variable_item_list_reset(app->varList);

    // First item: Choose a Resent Message
    item = variable_item_list_add(app->varList, "Choose Id", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // Second item: Set the Id
    item = variable_item_list_add(app->varList, "Id", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // Third item: Set the data lenght of the message
    item = variable_item_list_add(app->varList, "DLC", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // Fourth item: Set Request
    item = variable_item_list_add(app->varList, "Request", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0");

    // Fifth item: Set first Byte
    item = variable_item_list_add(app->varList, "FIRTS", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // Sixth item: Set second Byte
    item = variable_item_list_add(app->varList, "SECOND", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // Seventh item: Set third Byte
    item = variable_item_list_add(app->varList, "THIRD", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // Eighth item: Set fourth Byte
    item = variable_item_list_add(app->varList, "FOURTH", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // Nineth item: Set fifth Byte
    item = variable_item_list_add(app->varList, "FIFTH", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // tenth item: Set sixth Byte
    item = variable_item_list_add(app->varList, "SIXTH", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // eleventh item: Set seventh Byte
    item = variable_item_list_add(app->varList, "SEVENTH", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    // twelfth item: Set eighth Byte
    item = variable_item_list_add(app->varList, "EIGHTH", 0, callback_sender_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, "0x00");

    variable_item_list_set_selected_item(app->varList, 0);
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
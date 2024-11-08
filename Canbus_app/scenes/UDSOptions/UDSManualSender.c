#include "../../app_user.h"

static uint32_t id_request = DEFAULT_ECU_REQUEST;
static uint32_t id_response = DEFAULT_ECU_RESPONSE;

/*
    Scene uds manual sender to set the values to send
*/

void app_scene_uds_manual_sender_on_enter(void* context) {
    App* app = context;
    FuriString* text = app->text;
    VariableItem* item;

    item = variable_item_list_add(app->varList, "ID REQUEST", 0, NULL, app);
    variable_item_set_current_value_index(item, 0);
    furi_string_reset(text);
    furi_string_cat_printf(text, "%lx", id_request);
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));

    item = variable_item_list_add(app->varList, "ID RESPONSE", 0, NULL, app);
    variable_item_set_current_value_index(item, 0);
    furi_string_reset(text);
    furi_string_cat_printf(text, "%lx", id_response);
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));

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
    Scene for the response of the uds services
*/

void app_scene_uds_response_sender_on_enter(void* context) {
    App* app = context;
    text_box_reset(app->textBox);
    text_box_set_focus(app->textBox, TextBoxFocusEnd);

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
    text_box_reset(app->textBox);
}

/*
    Thread to work
*/

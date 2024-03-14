#include "../app_user.h"

uint8_t exampleValues_1[] = {0, 1, 2, 3};
uint8_t exampleValues_2[] = {0, 1, 2};

static uint8_t pos_1 = 3;
static uint8_t pos_2 = 1;

static const char* bitratesValues[] = {"125KBPS", "250KBPS", "500KBPS", "1000KBPS"};

static const char* clockValues[] = {"8MHz", "16MHz", "20MHz"};

void callback_options(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    uint8_t selectedIndex = variable_item_list_get_selected_item_index(app->varList);

    switch(selectedIndex) {
    case BitrateOption:
        variable_item_set_current_value_text(item, bitratesValues[index]);
        pos_1 = index;
        break;

    case CristyalClkOption:
        variable_item_set_current_value_text(item, clockValues[index]);
        pos_2 = index;
        break;

    default:
        break;
    }
}

void app_scene_Settings_on_enter(void* context) {
    App* app = context;
    VariableItem* item;

    variable_item_list_reset(app->varList);

    // First Item
    item = variable_item_list_add(
        app->varList, "Bitrate", COUNT_OF(bitratesValues), callback_options, app);
    variable_item_set_current_value_index(item, pos_1);
    variable_item_set_current_value_text(item, bitratesValues[pos_1]);

    // Second Item
    item =
        variable_item_list_add(app->varList, "Clock", sizeof(clockValues), callback_options, app);
    variable_item_set_current_value_index(item, pos_2);
    variable_item_set_current_value_text(item, clockValues[pos_2]);

    variable_item_list_set_selected_item(app->varList, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

bool app_scene_Settings_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(event);
    UNUSED(app);
    bool consumed = false;
    return consumed;
}
void app_scene_Settings_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}
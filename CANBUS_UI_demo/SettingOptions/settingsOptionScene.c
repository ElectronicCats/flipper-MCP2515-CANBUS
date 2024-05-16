#include "../app_user.h"

static uint8_t currentClock = MCP_16MHZ;
static uint8_t currentBitrate = MCP_500KBPS;

static const char* bitratesValues[] = {"125KBPS", "250KBPS", "500KBPS", "1000KBPS"};

static const char* clockValues[] = {"8MHz", "16MHz", "20MHz"};

static const char* save_logs[] = {"No Save", "Save All", "Only Address"};

void callback_options(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    uint8_t selected_index = variable_item_list_get_selected_item_index(app->varList);

    switch(selected_index) {
    case BitrateOption:
        variable_item_set_current_value_text(item, bitratesValues[index]);
        currentBitrate = index;
        app->mcp_can->bitRate = index;
        break;

    case CristyalClkOption:
        variable_item_set_current_value_text(item, clockValues[index]);
        currentClock = index;
        app->mcp_can->clck = index;

        break;

    case SaveLogsOption:
        variable_item_set_current_value_text(item, save_logs[index]);
        app->save_logs = index;
        break;

    default:
        break;
    }
}

void app_scene_settings_on_enter(void* context) {
    App* app = context;
    VariableItem* item;

    variable_item_list_reset(app->varList);

    // First Item
    item = variable_item_list_add(
        app->varList, "Bitrate", COUNT_OF(bitratesValues), callback_options, app);
    variable_item_set_current_value_index(item, currentBitrate);
    variable_item_set_current_value_text(item, bitratesValues[currentBitrate]);

    // Second Item
    item = variable_item_list_add(app->varList, "Clock", 0, callback_options, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, clockValues[currentClock]);

    // Third Item
    item = variable_item_list_add(app->varList, "Save LOGS?", 3, callback_options, app);
    variable_item_set_current_value_index(item, app->save_logs);
    variable_item_set_current_value_text(item, save_logs[app->save_logs]);

    variable_item_list_set_selected_item(app->varList, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

bool app_scene_settings_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(event);
    UNUSED(app);
    bool consumed = false;
    return consumed;
}

void app_scene_settings_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}
#include "../app_user.h"

void callback_options(void* context, uint32_t index) {
    App* app = context;
    switch(index) {
    case BitrateOption:
        scene_manager_handle_custom_event(app->scene_manager, BitrateOptionEvent);
        break;
    case CristyalClkOption:
        scene_manager_handle_custom_event(app->scene_manager, CristyalClkOptionEvent);
        break;
    }
}

void app_scene_Settings_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Settings");

    submenu_add_item(app->submenu, "Bitrate", BitrateOption, callback_options, app);

    submenu_add_item(app->submenu, "Clock", CristyalClkOption, callback_options, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool app_scene_Settings_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(app);
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case BitrateOptionEvent:
            break;

        case CristyalClkOptionEvent:
            break;
        }
        break;
    default:
        break;
    }

    return consumed;
}
void app_scene_Settings_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
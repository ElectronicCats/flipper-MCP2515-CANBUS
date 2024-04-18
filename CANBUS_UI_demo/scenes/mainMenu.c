#include "../app_user.h"

void basic_scenes_menu_callback(void* context, uint32_t index) {
    App* app = context;
    switch(index) {
    case SniffingTestOption:
        scene_manager_handle_custom_event(app->scene_manager, SniffingOptionEvent);
        break;

    case SenderOption:
        scene_manager_handle_custom_event(app->scene_manager, SenderOptionEvent);
        break;
    case SettingsOption:
        scene_manager_handle_custom_event(app->scene_manager, SettingsOptionEvent);
        break;
    }
}

void app_scene_Menu_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);

    submenu_set_header(app->submenu, "MENU CANBUS");

    submenu_add_item(
        app->submenu, "Sniffing", SniffingTestOption, basic_scenes_menu_callback, app);

    submenu_add_item(app->submenu, "Sender", SenderOption, basic_scenes_menu_callback, app);

    submenu_add_item(app->submenu, "Settings", SettingsOption, basic_scenes_menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool app_scene_Menu_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case SniffingOptionEvent:
            scene_manager_next_scene(app->scene_manager, AppScenesniffingTestOption);
            consumed = true;
            break;

        case SenderOptionEvent:
            scene_manager_next_scene(app->scene_manager, AppScenesenderTest);
            break;

        case SettingsOptionEvent:
            scene_manager_next_scene(app->scene_manager, AppScenesettingsOption);
            consumed = true;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return consumed;
}

void app_scene_Menu_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
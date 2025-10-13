#include "app_user.h"

#define TAG "OPTIONS LOGS"

void submenu_callback_logs_options(void* context, uint32_t index) {
    App* app = context;

    scene_manager_handle_custom_event(app->scene_manager, index);
}

void app_scene_logs_options_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "LOGS OPTIONS");

    submenu_add_item(
        app->submenu, "View frames from the log", VIEW_LOG, submenu_callback_logs_options, app);
    submenu_add_item(
        app->submenu, "Export log as CSV", EXPORT_LOG, submenu_callback_logs_options, app);
    submenu_add_item(
        app->submenu, "Transmit log to SavvyCAN", TRANSMIT_LOG, submenu_callback_logs_options, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuLogView);
}

bool app_scene_logs_options_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case VIEW_LOG:
            scene_manager_next_scene(app->scene_manager, app_scene_dialog_scene);
            consumed = true;
            break;
        case EXPORT_LOG:
            export_log_as_csv(app->storage, app->file_active->path);
            consumed = true;
            break;
        case TRANSMIT_LOG:
            scene_manager_next_scene(app->scene_manager, app_scene_lawicel_options_scene);
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

void app_scene_logs_options_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

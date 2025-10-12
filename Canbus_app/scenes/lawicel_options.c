#include "app_user.h"

#define TAG "LAWICEL OPTIONS SCENE"

uint32_t select_index_lawicel_options = 0;

void submenu_callback_lawicel_options(void* context, uint32_t index) {
    App* app = context;

    scene_manager_handle_custom_event(app->scene_manager, index);
}

void app_scene_lawicel_options_on_enter(void* context) {
    App* app = context;

    lawicel_open_port();

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "LAWICEL OPTIONS");

    submenu_add_item(
        app->submenu, "Send frame", LAWICEL_SEND_FRAME, submenu_callback_lawicel_options, app);
    submenu_add_item(
        app->submenu, "Send log", LAWICEL_SEND_LOG, submenu_callback_lawicel_options, app);

    submenu_set_selected_item(app->submenu, select_index_lawicel_options);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuLogView);
}

bool app_scene_lawicel_options_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case LAWICEL_SEND_FRAME:
            lawicel_send_frame(app->frame_active);
            break;
        case LAWICEL_SEND_LOG:
            lawicel_send_log(app->storage, app->file_active->path);
            break;
        default:
            break;
        }
        break;
    case SceneManagerEventTypeBack:
        select_index_lawicel_options = 0;
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
        break;
    default:
        break;
    }

    return consumed;
}

void app_scene_lawicel_options_on_exit(void* context) {
    App* app = context;
    lawicel_close_port();
    submenu_reset(app->submenu);
}

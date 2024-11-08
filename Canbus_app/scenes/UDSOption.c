#include "../app_user.h"

// Scene on enter to submenu
void app_scene_uds_menu_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "UDS Services");
    submenu_add_item(app->submenu, "Send Manual UDS Service", 0, NULL, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on event
bool app_scene_uds_menu_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

// Scene on Exit
void app_scene_uds_menu_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

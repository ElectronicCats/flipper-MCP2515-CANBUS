#include "../app.h"

void main_menu_callback(void* context, uint32_t index) {
    App* app = (App*)context;

    scene_manager_set_scene_state(app->scene_manager, app_scene_testing_option, index);
    scene_manager_next_scene(app->scene_manager, app_scene_testing_option);
}

void app_scene_main_menu_on_enter(void* context) {
    App* app = (App*)context;

    submenu_reset(app->submenu);

    submenu_set_header(app->submenu, "TEST CANBUS");

    submenu_add_item(app->submenu, "NODO 1", 0, main_menu_callback, app);
    submenu_add_item(app->submenu, "NODO 2", 1, main_menu_callback, app);
    submenu_add_item(app->submenu, "AUTOTEST", 2, main_menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool app_scene_main_menu_on_event(void* context, SceneManagerEvent event) {
    bool consumed = false;
    App* app = (App*)context;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void app_scene_main_menu_on_exit(void* context) {
    App* app = (App*)context;
    UNUSED(app);
}

#include "../app_user.h"

// Variable to set the item selected
uint32_t selector_option = 0;

// Callback function
void uds_menu_callback(void* context, uint32_t index) {
    App* app = context;

    // Selector option set
    selector_option = index;

    switch(index) {
    case 0: // Manual Sender UDS service
        scene_manager_next_scene(
            app->scene_manager, app_scene_uds_single_frame_request_sender_option);
        break;

    case 2: // Request VIN from car
        scene_manager_next_scene(app->scene_manager, app_scene_uds_request_vin_option);
        break;

    default:
        break;
    }
}

// Scene on enter to submenu
void app_scene_uds_menu_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "UDS Services");
    submenu_add_item(app->submenu, "Send Single Frame Request", 0, uds_menu_callback, app);
    submenu_add_item(app->submenu, "Send Multiple Frame Request", 1, uds_menu_callback, app);
    submenu_add_item(app->submenu, "Get VIN Number", 2, uds_menu_callback, app);

    submenu_set_selected_item(app->submenu, selector_option);

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

#include "../app_user.h"

/*
    This the menu of OBDII SCANNER
    IN A FUTURE WE WILL ADD FEATURES AS:
        - SCAN PID CODES
        - HACKING PID CODES
        - SCAN AND ERASE DTC ERRORS
        - ALL THE FUNCTIONALITIES AS OBDII SCANNER HAS
*/

void obdii_menu_callback(void* context, uint32_t index) {
  App* app = context;
  UNUSED(app);
  switch (index) {
    case 0:
      //
      break;

    default:
      break;
  }
}

void app_scene_obdii_menu_on_enter(void* context) {
  App* app = context;

  submenu_reset(app->submenu);

  submenu_set_header(app->submenu, "OBDII SCANNER");

  // Example
  submenu_add_item(app->submenu, "Engine Speed", 0, obdii_menu_callback, app);

  submenu_set_selected_item(app->submenu, 0);

  view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool app_scene_obdii_menu_on_event(void* context, SceneManagerEvent event) {
  UNUSED(context);
  UNUSED(event);
  return false;
}

void app_scene_obdii_menu_on_exit(void* context) {
  App* app = context;
  submenu_reset(app->submenu);
}

/*
    EXAMPLE WITH A ENGINE SPEED
*/

void app_scene_engine_rpm_speed_on_enter(void* context) {
  App* app = context;

  UNUSED(app);
}

bool app_scene_engine_rpm_speed_on_event(void* context,
                                         SceneManagerEvent event) {
  UNUSED(context);
  UNUSED(event);
  return false;
}

void app_scene_engine_rpm_speed_on_exit(void* context) {
  App* app = context;
  submenu_reset(app->submenu);
}

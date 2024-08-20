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

  app->obdii_aux_index = index;

  switch (index) {
    case 0:
      // scene_manager_next_scene(app->scene_manager,
      // app_scene_draw_obii_option);
      break;

    case 1:
      scene_manager_next_scene(app->scene_manager,
                               app_scene_obdii_typical_codes_option);
      break;

    case 2:
      // scene_manager_next_scene(app->scene_manager,
      // app_scene_draw_obii_option);
      break;

    case 3:
      // scene_manager_next_scene(app->scene_manager,
      // app_scene_draw_obii_option);
      break;

    case 4:
      // scene_manager_next_scene(app->scene_manager,
      // app_scene_draw_obii_option);
      break;

    default:
      break;
  }
}

void app_scene_obdii_menu_on_enter(void* context) {
  App* app = context;

  submenu_reset(app->submenu);

  submenu_set_header(app->submenu, "OBDII SCANNER");

  // Examples
  submenu_add_item(app->submenu, "Get Supported PID Codes", 0,
                   obdii_menu_callback, app);
  submenu_add_item(app->submenu, "Show Typical Data", 1, obdii_menu_callback,
                   app);
  submenu_add_item(app->submenu, "Show DTC", 2, obdii_menu_callback, app);
  submenu_add_item(app->submenu, "Delete DTC", 3, obdii_menu_callback, app);
  submenu_add_item(app->submenu, "Manual Sender PID", 4, obdii_menu_callback,
                   app);

  submenu_set_selected_item(app->submenu, app->obdii_aux_index);

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

  SHOW TYPICAL DATA ON PID CODES

*/

// Scene on enter
void app_scene_obdii_typical_codes_on_enter(void* context) {
  App* app = context;
  submenu_reset(app->submenu);
  view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on Event
bool app_scene_obdii_typical_codes_on_event(void* context,
                                            SceneManagerEvent event) {
  App* app = context;

  bool consumed = false;
  UNUSED(app);
  UNUSED(event);

  return consumed;
}

// Scene on exit
void app_scene_obdii_typical_codes_on_exit(void* context) {
  App* app = context;
  submenu_reset(app->submenu);
}

/*

  THIS SCENE WORKS TO DISPLAY THE RESPECTIVE DATA ON THE FLIPPER
  SHOWING THE TYPICAL DATA

*/

// Scene on enter
void app_scene_draw_obdii_on_enter(void* context) {
  App* app = context;

  // app->thread = furi_thread_alloc_ex("SniffingWork", 1024,
  // obdii_thread_on_work, app); furi_thread_start(app->thread);

  widget_reset(app->widget);
  view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

// Scene on Event
bool app_scene_draw_obdii_on_event(void* context, SceneManagerEvent event) {
  App* app = context;

  bool consumed = false;
  UNUSED(app);
  UNUSED(event);

  return consumed;
}

// Scene on exit
void app_scene_draw_obdii_on_exit(void* context) {
  App* app = context;
  // furi_thread_join(app->thread);
  // furi_thread_free(app->thread);

  widget_reset(app->widget);
}

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
      scene_manager_next_scene(app->scene_manager, app_scene_draw_obii_option);
      break;

    case 1:
      scene_manager_next_scene(app->scene_manager, app_scene_draw_obii_option);

    default:
      break;
  }
}

void app_scene_obdii_menu_on_enter(void* context) {
  App* app = context;

  submenu_reset(app->submenu);

  submenu_set_header(app->submenu, "OBDII SCANNER");

  // Examples
  submenu_add_item(app->submenu, "Engine Speed", 0, obdii_menu_callback, app);
  submenu_add_item(app->submenu, "Speed", 1, obdii_menu_callback, app);

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
    EXAMPLE WITH A ENGINE SPEED AND CAR SPEED
*/

// Selection of codes
void code_select(App* app, CANFRAME* frame) {
  frame->canId = 0x7DF;

  frame->data_lenght = 8;

  switch (app->obdii_aux_index) {
    case 0:
      frame->buffer[0] = 2;
      frame->buffer[1] = SHOW_DATA;
      frame->buffer[2] = ENGINE_SPEED;
      break;

    case 1:
      frame->buffer[0] = 2;
      frame->buffer[1] = SHOW_DATA;
      frame->buffer[2] = VEHICLE_SPEED;
      break;

    default:
      break;
  }
}

void draw_waitng_for_data(App* app) {
  widget_reset(app->widget);
  widget_add_string_element(app->widget, 65, 20, AlignCenter, AlignBottom,
                            FontPrimary, "WAITING FOR");

  widget_add_string_element(app->widget, 65, 35, AlignCenter, AlignBottom,
                            FontPrimary, "DATA");
}

// Draws When the device is not connected
void draw_device_no_connected(App* app) {
  widget_reset(app->widget);

  widget_add_string_element(app->widget, 65, 20, AlignCenter, AlignBottom,
                            FontPrimary, "DEVICE NO");

  widget_add_string_element(app->widget, 65, 35, AlignCenter, AlignBottom,
                            FontPrimary, "CONNECTED");
}

// Draws the engine speed
void draw_engine_speed(App* app, uint16_t engine_speed) {
  FuriString* text_label = app->text;

  furi_string_reset(text_label);
  widget_reset(app->widget);

  widget_add_string_element(app->widget, 65, 25, AlignCenter, AlignBottom,
                            FontPrimary, "ENGINE SPEED");

  furi_string_cat_printf(text_label, "%u RPM", engine_speed);
  widget_add_string_element(app->widget, 65, 45, AlignCenter, AlignBottom,
                            FontPrimary, furi_string_get_cstr(text_label));
}

// Draws the vehicle speed
void draw_vehicle_speed(App* app, uint16_t velocity) {
  FuriString* text_label = app->text;

  furi_string_reset(text_label);
  widget_reset(app->widget);

  widget_add_string_element(app->widget, 65, 25, AlignCenter, AlignBottom,
                            FontPrimary, "VEHICLE SPEED");

  furi_string_cat_printf(text_label, "%u KM/H", velocity);
  widget_add_string_element(app->widget, 65, 45, AlignCenter, AlignBottom,
                            FontPrimary, furi_string_get_cstr(text_label));
}

// This function works to set the frame to send
void engine_speed_option(App* app, CANFRAME frame) {
  if (frame.buffer[2] != 0x0C)
    return;

  uint16_t operation_a = 0;
  uint16_t operation_b = 0;

  if (frame.buffer[3] != 0)
    operation_a = frame.buffer[3] * 64;
  if (frame.buffer[4] != 0)
    operation_b = frame.buffer[4] / 4;
  uint16_t result = operation_a + operation_b;

  draw_engine_speed(app, result);
}

// The thread that will be on work
static int32_t obdii_thread_on_work(void* context) {
  App* app = context;
  UNUSED(app);

  MCP2515* mcp_can = app->mcp_can;
  CANFRAME frame_to_received;

  mcp_can->mode = MCP_NORMAL;

  ERROR_CAN debug_status = mcp2515_init(mcp_can);

  bool run = true;

  if (debug_status != ERROR_OK) {
    run = false;
    draw_device_no_connected(app);
  }

  CANFRAME frame_to_send = {0};  // CAN id to request pid codes to the car

  code_select(app, &frame_to_send);

  init_mask(mcp_can, 0, 0x7FF);
  init_mask(mcp_can, 1, 0x7FF);

  init_filter(mcp_can, 0, 0x7E8);
  init_filter(mcp_can, 1, 0x7E8);

  uint32_t time_delay_to_send = furi_get_tick();

  draw_waitng_for_data(app);

  while (furi_hal_gpio_read(&gpio_button_back) && (run == true)) {
    if ((furi_get_tick() - time_delay_to_send) > 10) {
      send_can_frame(mcp_can, &frame_to_send);

      time_delay_to_send = furi_get_tick();
    }

    if (check_receive(mcp_can) == ERROR_OK) {
      read_can_message(mcp_can, &frame_to_received);

      switch (frame_to_received.buffer[2]) {
        case ENGINE_SPEED:
          draw_engine_speed(app, get_engine_speed(frame_to_received.buffer[3],
                                                  frame_to_received.buffer[4]));
          break;

        case VEHICLE_SPEED:
          draw_vehicle_speed(app, frame_to_received.buffer[3]);
          break;
        default:
          break;
      }
    }
  }

  free_mcp2515(mcp_can);
  return 0;
}

// The scene of the draw
void app_scene_draw_obdii_on_enter(void* context) {
  App* app = context;

  app->thread =
      furi_thread_alloc_ex("SniffingWork", 1024, obdii_thread_on_work, app);
  furi_thread_start(app->thread);

  widget_reset(app->widget);
  view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_draw_obdii_on_event(void* context, SceneManagerEvent event) {
  App* app = context;

  bool consumed = false;
  UNUSED(app);
  UNUSED(event);

  return consumed;
}

void app_scene_draw_obdii_on_exit(void* context) {
  App* app = context;
  furi_thread_join(app->thread);
  furi_thread_free(app->thread);

  widget_reset(app->widget);
}

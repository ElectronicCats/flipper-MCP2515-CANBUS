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

    switch(index) {
    case 0:
        // scene_manager_next_scene(app->scene_manager,
        // app_scene_draw_obii_option);
        break;

    case 1:
        scene_manager_next_scene(app->scene_manager, app_scene_obdii_typical_codes_option);
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
    submenu_add_item(app->submenu, "Get Supported PID Codes", 0, obdii_menu_callback, app);
    submenu_add_item(app->submenu, "Show Typical Data", 1, obdii_menu_callback, app);
    submenu_add_item(app->submenu, "Show DTC", 2, obdii_menu_callback, app);
    submenu_add_item(app->submenu, "Delete DTC", 3, obdii_menu_callback, app);
    submenu_add_item(app->submenu, "Manual Sender PID", 4, obdii_menu_callback, app);

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
    Thread
*/
static int32_t obdii_thread_on_work(void* context);

/*

  SHOW TYPICAL DATA ON PID CODES

*/

// Selector of option
void typical_menu_callback(void* context, uint32_t index) {
    App* app = context;

    app->obdii_aux_index = index;
    scene_manager_set_scene_state(app->scene_manager, app_scene_obdii_typical_codes_option, index);

    switch(index) {
    case 0:
        scene_manager_next_scene(app->scene_manager, app_scene_draw_obii_option);
        break;

    case 1:
        scene_manager_next_scene(app->scene_manager, app_scene_draw_obii_option);
        break;

    case 2:
        scene_manager_next_scene(app->scene_manager, app_scene_draw_obii_option);
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

// Scene on enter
void app_scene_obdii_typical_codes_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_add_item(app->submenu, "Engine Speed", 0, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Vehicle Speed", 1, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Calculated Engine Load", 2, typical_menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
    scene_manager_set_scene_state(app->scene_manager, app_scene_obdii_typical_codes_option, 0);
}

// Scene on Event
bool app_scene_obdii_typical_codes_on_event(void* context, SceneManagerEvent event) {
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

// Draws device not connected
void draw_device_no_connected(App* app) {
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "DEVICE NO");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "CONNECTED");
}

// Draws engine speed
void draw_engine_speed(App* app, uint16_t engine_speed) {
    FuriString* text_label = app->text;

    furi_string_reset(text_label);
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 25, AlignCenter, AlignBottom, FontPrimary, "ENGINE SPEED");

    furi_string_cat_printf(text_label, "%u RPM", engine_speed);
    widget_add_string_element(
        app->widget,
        65,
        45,
        AlignCenter,
        AlignBottom,
        FontPrimary,
        furi_string_get_cstr(text_label));
}

// Draws the vehicle speed
void draw_vehicle_speed(App* app, uint16_t velocity) {
    FuriString* text_label = app->text;

    furi_string_reset(text_label);
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 25, AlignCenter, AlignBottom, FontPrimary, "VEHICLE SPEED");

    furi_string_cat_printf(text_label, "%u KM/H", velocity);
    widget_add_string_element(
        app->widget,
        65,
        45,
        AlignCenter,
        AlignBottom,
        FontPrimary,
        furi_string_get_cstr(text_label));
}

// Draws the calculated engine load
void draw_calculated_engine_load(App* app, uint16_t load_engine) {
    FuriString* text_label = app->text;

    furi_string_reset(text_label);
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 15, AlignCenter, AlignBottom, FontPrimary, "CALCULATED");

    widget_add_string_element(
        app->widget, 65, 30, AlignCenter, AlignBottom, FontPrimary, "ENGINE LOAD");

    furi_string_cat_printf(text_label, "%u %%", load_engine);
    widget_add_string_element(
        app->widget,
        65,
        45,
        AlignCenter,
        AlignBottom,
        FontPrimary,
        furi_string_get_cstr(text_label));
}

// Scene on enter
void app_scene_draw_obdii_on_enter(void* context) {
    App* app = context;

    app->thread = furi_thread_alloc_ex("SniffingWork", 1024, obdii_thread_on_work, app);
    furi_thread_start(app->thread);

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
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);

    widget_reset(app->widget);
}

/*
    Thread
*/

static int32_t obdii_thread_on_work(void* context) {
    App* app = context;
    OBDII scanner = app->obdii;

    scanner.bitrate = app->mcp_can->bitRate;

    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t request = 0;

    bool state;
    bool message;

    uint32_t option =
        scene_manager_get_scene_state(app->scene_manager, app_scene_obdii_typical_codes_option);

    switch(option) {
    case 0:
        request = ENGINE_SPEED;
        break;

    case 1:
        request = VEHICLE_SPEED;
        break;

    case 2:
        request = CALCULATED_ENGINE_LOAD;
        break;

    default:
        break;
    }

    state = pid_init(&scanner);

    uint32_t time_delay = furi_get_tick();

    if(!state) {
        draw_device_no_connected(app);
    }

    while(state && furi_hal_gpio_read(&gpio_button_back)) {
        if((furi_get_tick() - time_delay) > 10) {
            message = pid_show_data(&scanner, request, data, 8);

            if(message) {
                switch(option) {
                case 0:
                    draw_engine_speed(app, calculate_engine_speed(data[3], data[4]));
                    break;

                case 1:
                    draw_vehicle_speed(app, data[3]);
                    break;

                case 2:
                    draw_calculated_engine_load(app, 0);
                    break;

                default:
                    break;
                }
            }

            time_delay = furi_get_tick();
        }
    }
    pid_deinit(&scanner);

    UNUSED(app);
    return 0;
}

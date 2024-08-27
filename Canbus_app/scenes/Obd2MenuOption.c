#include "../app_user.h"
/*
    This the menu of OBDII SCANNER
    IN A FUTURE WE WILL ADD FEATURES AS:
        - SCAN PID CODES
        - HACKING PID CODES
        - SCAN AND ERASE DTC ERRORS
        - ALL THE FUNCTIONALITIES AS OBDII SCANNER HAS
*/

#define MESSAGE_ERROR 0xF0

// odbii menu casllback
void obdii_menu_callback(void* context, uint32_t index) {
    App* app = context;

    app->obdii_aux_index = index;

    switch(index) {
    case 0:
        scene_manager_next_scene(app->scene_manager, app_scene_supported_pid_option);
        break;

    case 1:
        scene_manager_next_scene(app->scene_manager, app_scene_obdii_typical_codes_option);
        break;

    case 2:
        scene_manager_next_scene(app->scene_manager, app_scene_obdii_get_errors_option);
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
static int32_t obdii_thread_getting_pid_supported_on_work(void* context);
static int32_t obdii_thread_dtc_on_work(void* context);

/*

  SHOW TYPICAL DATA ON PID CODES

*/

// Selector of option
void typical_menu_callback(void* context, uint32_t index) {
    App* app = context;

    app->obdii_aux_index = index;
    scene_manager_set_scene_state(app->scene_manager, app_scene_obdii_typical_codes_option, index);
    scene_manager_next_scene(
        app->scene_manager,
        app_scene_draw_obii_option); // It will go directly to the respective scene
}

// Scene on enter
void app_scene_obdii_typical_codes_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_add_item(app->submenu, "Engine Speed", 0, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Vehicle Speed", 1, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Calculated Engine Load", 2, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Thortle Position", 3, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Fuel Tank Input Level", 4, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Thortle Relative Position", 5, typical_menu_callback, app);
    submenu_add_item(app->submenu, "Time Engine Running", 6, typical_menu_callback, app);

    submenu_set_selected_item(
        app->submenu,
        scene_manager_get_scene_state(app->scene_manager, app_scene_obdii_typical_codes_option));
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

// Draws a developmet
void draw_in_development(App* app) {
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "SCENE IN");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "DEVELOPMENT");
}

// Draws device not connected
void draw_device_no_connected(App* app) {
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "DEVICE NO");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "CONNECTED");
}

// draw when a message is not recognized
void draw_send_wrong(App* app) {
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "TRANSMITION");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "FAILURE");
}

// Draw the unit medition
void draw_value(App* app, const char* text) {
    widget_add_string_element(app->widget, 65, 45, AlignCenter, AlignBottom, FontPrimary, text);
}

// Draws the calculated engine load
void draw_scene(App* app, uint8_t selector, uint16_t variable) {
    FuriString* text_label = app->text;

    char* text = "";

    furi_string_reset(text_label);
    widget_reset(app->widget);

    if(selector == 0) { // Engine Speed
        widget_add_string_element(
            app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "ENGINE SPEED");
        text = "RPM";
    } else if(selector == 1) { // Vehicle Speed
        widget_add_string_element(
            app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "VEHICLE SPEED");
        text = "KM/H";

    } else if(selector == 2) { // CALCULATED ENGINE LOAD
        widget_add_string_element(
            app->widget, 65, 15, AlignCenter, AlignBottom, FontPrimary, "CALCULATED");

        widget_add_string_element(
            app->widget, 65, 30, AlignCenter, AlignBottom, FontPrimary, "ENGINE LOAD");

        text = "%";

    } else if(selector == 3) { // Thortle Position
        widget_add_string_element(
            app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "THORLE POSITION");

        text = "%";

    } else if(selector == 4) { // Fuel Tank Input Level
        widget_add_string_element(
            app->widget, 65, 15, AlignCenter, AlignBottom, FontPrimary, "FUEL TANK");
        widget_add_string_element(
            app->widget, 65, 30, AlignCenter, AlignBottom, FontPrimary, "INPUT LEVEL");

        text = "%";

    } else if(selector == 5) { // Thortle Relative Position
        widget_add_string_element(
            app->widget, 65, 15, AlignCenter, AlignBottom, FontPrimary, "THORTLE");
        widget_add_string_element(
            app->widget, 65, 30, AlignCenter, AlignBottom, FontPrimary, "RELATIVE POSITION");

        text = "%";
    } else if(selector == 6) {
        widget_add_string_element(
            app->widget, 65, 15, AlignCenter, AlignBottom, FontPrimary, "TIME SINCE");
        widget_add_string_element(
            app->widget, 65, 30, AlignCenter, AlignBottom, FontPrimary, "ENGINE START");

        text = "seg";
    }

    furi_string_cat_printf(text_label, "%u %s", variable, text);

    draw_value(app, furi_string_get_cstr(text_label));
}

// Scene on enter
void app_scene_draw_obdii_on_enter(void* context) {
    App* app = context;

    app->thread = furi_thread_alloc_ex("GetDataPID", 1024, obdii_thread_on_work, app);
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
    This scene works to Display the respective PID codes supported by the car
*/

// Callback for the supported menu
void supported_pid_callback(void* context, uint32_t index) {
    App* app = context;

    UNUSED(app);

    switch(index) {
    case 0x100:
        app->flags = BLOCK_A;
        scene_manager_next_scene(app->scene_manager, app_scene_list_supported_pid_option);
        break;

    case 0x101:
        app->flags = BLOCK_B;
        scene_manager_next_scene(app->scene_manager, app_scene_list_supported_pid_option);
        break;

    case 0x102:
        app->flags = BLOCK_C;
        scene_manager_next_scene(app->scene_manager, app_scene_list_supported_pid_option);
        break;

    case 0x103:
        app->flags = BLOCK_D;
        scene_manager_next_scene(app->scene_manager, app_scene_list_supported_pid_option);
        break;

    default:
        break;
    }
}

// Scene on enter
void app_scene_menu_supported_pid_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Get Supported PID");
    submenu_add_item(app->submenu, "Block A", 0x100, supported_pid_callback, app);
    submenu_add_item(app->submenu, "Block B", 0x101, supported_pid_callback, app);
    submenu_add_item(app->submenu, "Block C", 0x102, supported_pid_callback, app);
    submenu_add_item(app->submenu, "Block D", 0x103, supported_pid_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on Event
bool app_scene_menu_supported_pid_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);
    return consumed;
}

// Scene on exit
void app_scene_menu_supported_pid_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

/*
    Scene of the pid list
*/

// Scene on enter
void app_scene_list_supported_pid_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);

    if(scene_manager_get_scene_state(app->scene_manager, app_scene_list_supported_pid_option) ==
       1) {
        scene_manager_set_scene_state(app->scene_manager, app_scene_list_supported_pid_option, 0);
        scene_manager_previous_scene(app->scene_manager);
    } else {
        app->thread = furi_thread_alloc_ex(
            "GetSupportedPID", 1024, obdii_thread_getting_pid_supported_on_work, app);
        furi_thread_start(app->thread);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on event
bool app_scene_list_supported_pid_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    if(event.event == DEVICE_NO_CONNECTED) {
        consumed = true;

        // Go to the scene
        scene_manager_set_scene_state(app->scene_manager, app_scene_list_supported_pid_option, 0);

        // Set the scene to know the error
        scene_manager_set_scene_state(app->scene_manager, app_scene_obdii_warning_scenes, 0);
        scene_manager_next_scene(app->scene_manager, app_scene_obdii_warning_scenes);
    }
    if(event.event == MESSAGE_ERROR) {
        scene_manager_set_scene_state(app->scene_manager, app_scene_list_supported_pid_option, 0);

        // Set the scene to know the error
        scene_manager_set_scene_state(app->scene_manager, app_scene_list_supported_pid_option, 1);
        scene_manager_set_scene_state(app->scene_manager, app_scene_obdii_warning_scenes, 1);
        scene_manager_next_scene(app->scene_manager, app_scene_obdii_warning_scenes);
    }
    return consumed;
}

// Scene on exit
void app_scene_list_supported_pid_on_exit(void* context) {
    App* app = context;
    if(scene_manager_get_scene_state(app->scene_manager, app_scene_list_supported_pid_option) ==
       0) {
        furi_thread_join(app->thread);
        furi_thread_free(app->thread);
    }

    submenu_reset(app->submenu);
}

/*
    Scene to watch the errors in the car
*/

void app_scene_obdii_get_errors_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    app->thread = furi_thread_alloc_ex("ShowDTC", 1024, obdii_thread_dtc_on_work, app);
    furi_thread_start(app->thread);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_obdii_get_errors_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(app);
    UNUSED(event);
    return false;
}

void app_scene_obdii_get_errors_on_exit(void* context) {
    App* app = context;
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
    widget_reset(app->widget);
}

/*
    Manual Sender Scene
*/
void app_scene_manual_sender_pid_on_enter(void* context) {
    App* app = context;
    UNUSED(app);
}

bool app_scene_manual_sender_pid_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void app_scene_manual_sender_pid_on_exit(void* context) {
    App* app = context;
    UNUSED(app);
}

/*
    Scene to set if the device wasnt sent okay or the device is not connected
*/

void app_scene_obdii_warnings_on_enter(void* context) {
    App* app = context;

    uint32_t state =
        scene_manager_get_scene_state(app->scene_manager, app_scene_obdii_warning_scenes);

    if(state == 0) draw_device_no_connected(app);
    if(state == 1) draw_send_wrong(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_obdii_warnings_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void app_scene_obdii_warnings_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
}

/*
    Thread to get the supported PID
*/

static int32_t obdii_thread_getting_pid_supported_on_work(void* context) {
    App* app = context;

    OBDII scanner;

    scanner.bitrate = app->mcp_can->bitRate;

    bool run = pid_init(&scanner);

    bool draw_list = false;

    FuriString* text = app->text;

    // if the device is not detected the thread send an custom event
    if(!run) {
        scene_manager_set_scene_state(app->scene_manager, app_scene_supported_pid_option, 1);
        view_dispatcher_send_custom_event(app->view_dispatcher, DEVICE_NO_CONNECTED);
    }

    // if it runs
    if(run) {
        uint32_t flag = app->flags;

        if(pid_get_supported_pid(&scanner, flag)) {
            draw_list = true;

        } else {
            view_dispatcher_send_custom_event(app->view_dispatcher, MESSAGE_ERROR);
        }

        if(draw_list) {
            for(uint8_t i = 1; i <= 32; i++) {
                furi_string_reset(text);

                if(scanner.codes[i + flag].is_supported) {
                    furi_string_cat_printf(text, "0x%lx Supported", i + flag);
                }

                if(!scanner.codes[i + flag].is_supported) {
                    furi_string_cat_printf(text, "0x%lx Not Supported", i + flag);
                }

                submenu_add_item(
                    app->submenu, furi_string_get_cstr(text), i, supported_pid_callback, app);
            }
        }
    }

    pid_deinit(&scanner);

    return 0;
}

/*
    Thread To Show DATA
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

    case 3:
        request = THROTTLE_POSITION;
        break;

    case 4:
        request = FUEL_INPUT_POSITION;
        break;

    case 5:
        request = THROTTLE_RELATIVE_POSITION;
        break;

    case 6:
        request = RUN_TIME_SINCE_ENGINE_START;
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
                case 0: // Engine Speed RPM
                    draw_scene(app, option, calculate_engine_speed(data[3], data[4]));
                    break;

                case 1: // Vehcile Speed
                    draw_scene(app, option, data[3]);
                    break;

                case 2: // Calculated Engine Load
                    draw_scene(app, option, calculate_engine_load(data[3]));
                    break;

                case 3: // Thortle Position
                    draw_scene(app, option, calculate_engine_load(data[3]));
                    break;

                case 4: // Fuel Tank Input Level
                    draw_scene(app, option, calculate_engine_load(data[3]));
                    break;

                case 5: // Thortle Relative Position
                    draw_scene(app, option, calculate_engine_load(data[3]));
                    break;

                case 6: // Time Engine Running
                    draw_scene(app, option, sum_value(data[3], data[4]));
                    break;

                default:
                    draw_in_development(app);
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

/*
    Thread to request the DTC (Diagnostic Trouble Codes)
*/

static int32_t obdii_thread_dtc_on_work(void* context) {
    App* app = context;
    UNUSED(app);

    OBDII scanner;
    pid_init(&scanner);

    uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};

    UNUSED(data);

    while(furi_hal_gpio_read(&gpio_button_back)) {
        /*if(furi_hal_gpio_read(&gpio_button_ok)) {
            if(pid_manual_request(&scanner, SHOW_STORAGE_DTC, 0X00, data)) {
                log_info("Get DTC");
            } else {
                log_exception("Error");
            }
        }

        furi_delay_ms(1);*/
    }
    return 0;
}

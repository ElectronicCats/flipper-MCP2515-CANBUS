#include "../../app_user.h"

// Function for the thread
static int32_t obdii_get_car_data(void* context);

/*
    Scene to get the VIN and ECU Name
*/
// Scene on enter
void app_scene_get_car_data_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);

    app->thread = furi_thread_alloc_ex("CarDatas", 1024, obdii_get_car_data, app);
    furi_thread_start(app->thread);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

// Scene on event
bool app_scene_get_car_data_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

// Scene on exit
void app_scene_get_car_data_on_exit(void* context) {
    App* app = context;
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
    widget_reset(app->widget);
}

/*
    Thread to get the VIN number
*/

static int32_t obdii_get_car_data(void* context) {
    App* app = context;
    OBDII scanner;

    scanner.bitrate = app->mcp_can->bitRate;

    bool run = pid_init(&scanner);

    if(run) {
        if(app->request_data == 1) {
            if(get_VIN(&scanner, app->text)) {
                widget_add_string_element(
                    app->widget, 64, 20, AlignCenter, AlignCenter, FontPrimary, "VIN Number:");

                widget_add_string_element(
                    app->widget,
                    64,
                    32,
                    AlignCenter,
                    AlignCenter,
                    FontSecondary,
                    furi_string_get_cstr(app->text));
            } else {
                draw_send_wrong(app);
            }
        }
        if(app->request_data == 2) {
            if(get_ECU_name(&scanner, app->text)) {
                widget_add_string_element(
                    app->widget, 64, 20, AlignCenter, AlignCenter, FontPrimary, "ECU Name:");

                widget_add_string_element(
                    app->widget,
                    64,
                    32,
                    AlignCenter,
                    AlignCenter,
                    FontSecondary,
                    furi_string_get_cstr(app->text));
            } else {
                draw_send_wrong(app);
            }
        }
    } else
        draw_device_no_connected(app);

    pid_deinit(&scanner);

    return 0;
}

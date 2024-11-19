#include "../../app_user.h"

static int32_t uds_get_vin_thread(void* context);

/*
    UDS get Vin Scene
*/

// Scene on enter
void app_scene_uds_request_vin_on_enter(void* context) {
    App* app = context;

    widget_reset(app->widget);

    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "On\nDevelopment");

    app->thread = furi_thread_alloc_ex("ManualUDS", 1024, uds_get_vin_thread, app);
    furi_thread_start(app->thread);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

// Scene on event
bool app_scene_uds_request_vin_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

// Scene on exit
void app_scene_uds_request_vin_on_exit(void* context) {
    App* app = context;

    furi_thread_join(app->thread);
    furi_thread_free(app->thread);

    widget_reset(app->widget);
}

/*
    Thread to work with
*/

static int32_t uds_get_vin_thread(void* context) {
    App* app = context;

    FuriString* text = app->text;

    UNUSED(text);

    return 0;
}

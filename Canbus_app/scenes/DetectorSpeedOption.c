#include "../app_user.h"

static int32_t thread_to_detect_speed(void* context);

void app_scene_speed_detector_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    app->thread = furi_thread_alloc_ex("Auto-detector Speed", 1024, thread_to_detect_speed, app);
    furi_thread_start(app->thread);

    draw_in_development(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_speed_detector_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(app);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void app_scene_speed_detector_on_exit(void* context) {
    App* app = context;
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
    widget_reset(app->widget);
}

static int32_t thread_to_detect_speed(void* context) {
    App* app = context;
    UNUSED(app);
    return 0;
}

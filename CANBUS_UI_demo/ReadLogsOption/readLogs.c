#include "../app_user.h"

void app_scene_read_logs_on_enter(void* context) {
    App* app = context;
    UNUSED(app);
}

bool app_scene_read_logs_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);
    return consumed;
}

void app_scene_read_logs_on_exit(void* context) {
    App* app = context;
    UNUSED(app);
}
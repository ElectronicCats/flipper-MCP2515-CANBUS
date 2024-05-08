#include "../app_user.h"

void app_scene_about_us_on_enter(void* context) {
    App* app = context;
    UNUSED(context);
}

bool app_scene_about_us_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);
    return consumed;
}

void app_scene_about_us_on_exit(void* context) {
    App* app = context;
    UNUSED(context);
}
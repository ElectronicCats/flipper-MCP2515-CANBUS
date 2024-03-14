#include "../app_user.h"

void app_scene_Sniffing_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 25, 15, AlignLeft, AlignCenter, FontPrimary, "Sniffing");
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}
bool app_scene_Sniffing_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void app_scene_Sniffing_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
}
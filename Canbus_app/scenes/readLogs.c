#include "../app_user.h"

void app_scene_read_logs_on_enter(void* context) {
    App* app = context;
    text_box_set_font(app->textBox, TextBoxFontText);
    text_box_reset(app->textBox);
    view_dispatcher_switch_to_view(app->view_dispatcher, TextBoxView);
    text_box_set_text(app->textBox, furi_string_get_cstr(app->text));
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
    furi_string_reset(app->text);
    text_box_reset(app->textBox);
}

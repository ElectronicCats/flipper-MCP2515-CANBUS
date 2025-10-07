#include "app_user.h"

#define TAG "DIALOG LOG"

void callback(DialogExResult result, void* context) {
    App* app = context;

    switch(result) {
    case DialogExPressLeft:
        FURI_LOG_I(TAG, "SE PRESIONÓ EL BOTÓN IZQUIERDO");
        app->file_active->frame_index--;
        app_scene_dialog_on_enter(app);
        break;
    case DialogExPressRight:
        FURI_LOG_I(TAG, "SE PRESIONÓ EL BOTÓN DERECHO");
        app->file_active->frame_index++;
        app_scene_dialog_on_enter(app);
        break;
    case DialogExReleaseLeft:
        FURI_LOG_I(TAG, "SE SOLTÓ EL BOTÓN IZQUIERDO");
        break;
    case DialogExReleaseRight:
        FURI_LOG_I(TAG, "SE SOLTÓ EL BOTÓN DERECHO");
        break;
    default:
        break;
    }
}

void app_scene_dialog_on_enter(void* context) {
    furi_assert(context);
    App* app = context;

    dialog_ex_reset(app->dialog_ex);
    dialog_ex_set_context(app->dialog_ex, app);
    dialog_ex_set_result_callback(app->dialog_ex, callback);
    dialog_ex_enable_extended_events(app->dialog_ex);

    frame_extractor(
        app->storage,
        furi_string_get_cstr(app->file_active->path),
        app->frame_active,
        app->file_active->frame_index);

    dialog_ex_set_header(
        app->dialog_ex,
        furi_string_get_cstr(app->frame_active->can_id),
        64,
        12,
        AlignCenter,
        AlignCenter);

    dialog_ex_set_text(
        app->dialog_ex,
        furi_string_get_cstr(app->frame_active->can_data),
        64,
        32,
        AlignCenter,
        AlignCenter);

    if(app->file_active->frame_index > 0) {
        dialog_ex_set_left_button_text(app->dialog_ex, "Prev");
    }

    if(app->file_active->frame_index < app->file_active->frames_count - 1) {
        dialog_ex_set_right_button_text(app->dialog_ex, "Next");
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, DialogViewScene);
}

bool app_scene_dialog_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void app_scene_dialog_on_exit(void* context) {
    furi_assert(context);
    App* app = context;

    dialog_ex_reset(app->dialog_ex);
}

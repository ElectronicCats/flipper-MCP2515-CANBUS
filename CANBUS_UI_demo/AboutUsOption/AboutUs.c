#include "../app_user.h"

void button_callback(GuiButtonType result, InputType input, void* context) {
    App* app = context;
    UNUSED(app);
    UNUSED(result);

    if(input == InputTypeRelease) {
        if(result == GuiButtonTypeRight)
            scene_manager_handle_custom_event(app->scene_manager, RightButtonEvent);

        if(result == GuiButtonTypeLeft)
            scene_manager_handle_custom_event(app->scene_manager, LeftButtonEvent);
    }
}

void draw_first_view(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "ELECTRONIC CATS");
    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignCenter, FontPrimary, "Presents:");

    widget_add_button_element(app->widget, GuiButtonTypeRight, "Next", button_callback, app);
}

void draw_second_view(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "CANBUS APP");
    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignCenter, FontSecondary, "By: Adonai Diaz");

    widget_add_button_element(app->widget, GuiButtonTypeRight, "Next", button_callback, app);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Prev", button_callback, app);
}

void draw_third_view(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 10, AlignCenter, AlignCenter, FontPrimary, "More info:");
    widget_add_string_element(
        app->widget,
        65,
        25,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "https://github.com");
    
    widget_add_string_element(
        app->widget,
        65,
        35,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "/ElectronicCats");

    widget_add_string_element(
        app->widget, 65, 45, AlignCenter, AlignCenter, FontSecondary, "/flipper-MCP2515-CANBUS");
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Prev", button_callback, app);
}

void app_scene_about_us_on_enter(void* context) {
    App* app = context;
    scene_manager_set_scene_state(app->scene_manager, AppSceneAboutUs, 0);
    draw_first_view(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_about_us_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    uint32_t state = scene_manager_get_scene_state(app->scene_manager, AppSceneAboutUs);
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LeftButtonEvent) {
            state = state - 1;
            scene_manager_set_scene_state(app->scene_manager, AppSceneAboutUs, state);
        }
        if(event.event == RightButtonEvent) {
            state = state + 1;
            scene_manager_set_scene_state(app->scene_manager, AppSceneAboutUs, state);
        }

        if(state == 0) draw_first_view(app);
        if(state == 1) draw_second_view(app);
        if(state == 2) draw_third_view(app);
    }
    return consumed;
}

void app_scene_about_us_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
}
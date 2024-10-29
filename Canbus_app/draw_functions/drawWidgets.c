#include "../app_user.h"

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
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "DEVICE NOT");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "CONNECTED");
}

// draw when a message is not recognized
void draw_transmition_failure(App* app) {
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "TRANSMITION");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "FAILURE");
}

// draw when a message is send OK
void draw_send_wrong(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "MESSAGE SEND ERROR");
}

// draw when a message is send ok
void draw_send_ok(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignCenter, FontPrimary, "MESSAGE SEND OK");
}

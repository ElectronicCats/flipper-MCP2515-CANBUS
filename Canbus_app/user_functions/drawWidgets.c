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
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "DEVICE NO");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "CONNECTED");
}

// draw when a message is not recognized
void draw_send_wrong(App* app) {
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "TRANSMITION");

    widget_add_string_element(
        app->widget, 65, 35, AlignCenter, AlignBottom, FontPrimary, "FAILURE");
}

#include "../app_user.h"

// Draws a developmet
void draw_in_development(App* app) {
    widget_reset(app->widget);

    widget_add_string_multiline_element(
        app->widget, 65, 40, AlignCenter, AlignBottom, FontPrimary, "SCENE IN\nDEVELOPMENT");

}

// Draws device not connected
void draw_device_no_connected(App* app) {
    widget_reset(app->widget);
    widget_add_icon_element(app->widget, 4, 0, &I_NOC119x38);
    widget_add_string_multiline_element(
        app->widget, 65, 60, AlignCenter, AlignBottom, FontPrimary, "DEVICE NOT\nCONNECTED");

}

// draw when a message is not recognized
void draw_transmition_failure(App* app) {
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 40, AlignCenter, AlignBottom, FontPrimary, "TRANSMITION\nFAILURE");

}

// draw when a message is send OK
void draw_send_wrong(App* app) {
    widget_reset(app->widget);
    widget_add_string_multiline_element(
        app->widget, 65, 40, AlignCenter, AlignCenter, FontPrimary, "ERROER SENDING\nMESSAGE");
}

// draw when a message is send ok
void draw_send_ok(App* app) {
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 24, AlignCenter, AlignCenter, FontPrimary, "MESSAGE SEND OK");
}

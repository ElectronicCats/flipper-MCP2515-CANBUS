#include "../../app_user.h"

bool delete_dtc = false;

// Thread to work with for dtc
static int32_t thread_uds_protocol_dtc(void* context);

// Callback for the menu
void storage_dtc_menu_callback(void* context, uint32_t index) {
    App* app = context;
    delete_dtc = (index == 1) ? true : false;

    // Go to the response scene
    scene_manager_next_scene(app->scene_manager, app_scene_uds_dtc_response_option);
}

// Scene on enter
void app_scene_uds_get_dtc_menu_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);

    submenu_set_header(app->submenu, "MENU DTC STORAGED");

    submenu_add_item(app->submenu, "Read DTC", 0, storage_dtc_menu_callback, app);
    submenu_add_item(app->submenu, "Delete DTC", 1, storage_dtc_menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on event
bool app_scene_uds_get_dtc_menu_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

// Scene on exit
void app_scene_uds_get_dtc_menu_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

//Scene on enter for dtc response
void app_scene_uds_dtc_response_on_enter(void* context) {
    App* app = context;

    widget_reset(app->widget);

    app->thread = furi_thread_alloc_ex("Get DTCs", 2 * 1024, thread_uds_protocol_dtc, app);
    furi_thread_start(app->thread);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

// Scene on event for dtc response
bool app_scene_uds_dtc_response_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

// Scene on exit
void app_scene_uds_dtc_response_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
}

/**
 * Functions to Draw the responses
 */

// To show no DTC read it
void draw_no_dtc(App* app) {
    widget_reset(app->widget);
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "No DTC\nStored to show");
}

/**
 * Thread to work with DTC and UDS protocol
 */

static int32_t thread_uds_protocol_dtc(void* context) {
    App* app = context;
    MCP2515* CAN = app->mcp_can;

    FuriString* text = app->text;

    furi_string_reset(text);

    UDS_SERVICE* uds_service = uds_service_alloc(
        UDS_REQUEST_ID_DEFAULT, UDS_RESPONSE_ID_DEFAULT, CAN->mode, CAN->clck, CAN->bitRate);

    if(!uds_init(uds_service)) {
        draw_device_no_connected(app);
        free_uds(uds_service);
        return 0;
    }

    if(delete_dtc) {
        delete_dtc = 0;

        free_uds(uds_service);
        return 0;
    }

    uint16_t count_of_dtc = 0;

    if(!uds_get_count_storaged_dtc(uds_service, &count_of_dtc)) {
        draw_transmition_failure(app);
        free_uds(uds_service);
        return 0;
    }

    if(count_of_dtc > 0) {
    } else {
        draw_no_dtc(app);
    }

    free_uds(uds_service);

    return 0;
}

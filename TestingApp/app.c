#include "app.h"

static bool app_scene_costum_callback(void* context, uint32_t costum_event) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, costum_event);
}

static bool app_scene_back_event(void* context) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void app_tick_event(void* context) {
    furi_assert(context);
    App* app = context;
    UNUSED(app);
}

App* app_alloc() {
    App* app = (App*)malloc(sizeof(App));

    // Alloc
    app->mcp_can = mcp_alloc(MCP_NORMAL, MCP_16MHZ, MCP_500KBPS);

    // Alloc the scene manager and view dispatcher
    app->scene_manager = scene_manager_alloc(&app_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();

    // Set the navegation on the view dispatcher
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, app_scene_costum_callback);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, app_scene_back_event);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, app_tick_event, 100);

    // Alloc the GUI Modules and add the view in the view dispatcher
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, WidgetView, widget_get_view(app->widget));

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, SubmenuView, submenu_get_view(app->submenu));

    app->text = furi_string_alloc();

    return app;
}

void app_free(App* app) {
    view_dispatcher_remove_view(app->view_dispatcher, SubmenuView);
    view_dispatcher_remove_view(app->view_dispatcher, WidgetView);

    // Free memory of Scene Manager and View Dispatcher
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    // Free memory of GUI modules
    widget_free(app->widget);
    submenu_free(app->submenu);

    // Free memory of the text
    furi_string_free(app->text);

    //  Deinit CAN
    deinit_mcp2515(app->mcp_can);

    free(app);
}

int32_t app_main(void* p) {
    UNUSED(p);

    App* app = app_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);

    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    scene_manager_next_scene(app->scene_manager, app_scene_main_menu);

    view_dispatcher_run(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    app_free(app);

    return 0;
}

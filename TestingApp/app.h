#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>

#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

// #include "scenes_config/app_scene_functions.h"

#include "scenes_config/app_scene_functions.h"
#include "libraries/mcp_can_2515.h"

typedef struct {
    MCP2515* mcp_can;

    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    FuriThread* thread;

    Widget* widget;
    Submenu* submenu;
    FuriString* text;
} App;

typedef enum {
    SubmenuView,
    WidgetView
} scenesViews;

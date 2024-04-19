#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/text_box.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/text_input.h>

#include "scenes/app_scene_functions.h"

#include "libraries/mcp_can_2515.h"

typedef enum {
    WorkerflagStop = (1 << 0),
    WorkerflagReceived = (1 << 1),
} WorkerEvtFlags;

#define WORKER_ALL_RX_EVENTS (WorkerflagStop | WorkerflagReceived)

typedef struct {
    MCP2515* mcp_can;
    CANFRAME can_frame;
    CANFRAME* frameArray;
    CANFRAME* frame_to_send;

    FuriThread* thread;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    Submenu* submenu;
    VariableItemList* varList;
    TextBox* textBox;
    DialogEx* dialog_info;
    ByteInput* input_byte_value;

    FuriString* text;
    FuriString* textLabel;

    uint32_t sniffer_index;
    uint32_t sniffer_index_aux;

    uint8_t num_of_devices;
    uint8_t sender_selected_item;
} App;

// This is for the menu Options
typedef enum { SniffingTestOption, SenderOption, SettingsOption } MainMenuOptions;
typedef enum { SniffingOptionEvent, SenderOptionEvent, SettingsOptionEvent } MainMenuEvents;

// This is for the Setting Options
typedef enum { Refresh } SnifferEvents;
typedef enum { BitrateOption, CristyalClkOption } OptionSettings;
typedef enum { BitrateOptionEvent, CristyalClkOptionEvent } SettingsMenuEvent;
typedef enum { ChooseIdEvent, SetIdEvent, ReturnEvent } SenderEvents;

// This is for SniffingTest Options
typedef enum { RefreshTest, EntryEvent, ShowData } SniffingTestEvents;

// Views in the App
typedef enum {
    SubmenuView,
    ViewWidget,
    VarListView,
    TextBoxView,
    DialogInfoView,
    InputByteView,
} scenesViews;
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>

#include <dialogs/dialogs.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <gui/modules/file_browser.h>
#include <gui/modules/file_browser_worker.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>

#include "scenes_config/app_scene_functions.h"

#include "libraries/mcp_can_2515.h"
#include "libraries/pid_library.h"

#define PATHAPP    "apps_data/canbus"
#define PATHAPPEXT EXT_PATH(PATHAPP)
#define PATHLOGS   PATHAPPEXT "/logs"

#define DEVICE_NO_CONNECTED (0xFF)

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

    OBDII obdii;

    uint32_t time;
    uint32_t times[100];
    uint32_t current_time[100];

    FuriThread* thread;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    Submenu* submenu;
    VariableItemList* varList;
    TextBox* textBox;
    ByteInput* input_byte_value;
    FileBrowser* file_browser;

    FuriString* text;
    FuriString* data;
    FuriString* path;

    Storage* storage;
    DialogsApp* dialogs;
    File* log_file;
    char* log_file_path;
    bool log_file_ready;
    uint8_t save_logs;

    uint32_t sniffer_index;
    uint32_t sniffer_index_aux;

    uint8_t config_timing_index;

    uint8_t num_of_devices;
    uint8_t sender_selected_item;
    uint8_t sender_id_compose[4];

    uint32_t obdii_aux_index;

    uint64_t size_of_storage;
} App;

// This is for the menu Options
typedef enum {
    SniffingTestOption,
    SenderOption,
    ObdiiOption,
    ReadLOGOption,
    PlayLOGOption,
    SettingsOption,
    AboutUsOption,
} MainMenuOptions;

// These are the events on the main menu
typedef enum {
    SniffingOptionEvent,
    SenderOptionEvent,
    SettingsOptionEvent,
    ObdiiOptionEvent,
    ReadLOGOptionEvent,
    PlayLOGOptionEvent,
    AboutUsEvent,
} MainMenuEvents;

// This is for the Setting Options
typedef enum {
    BitrateOption,
    CristyalClkOption,
    SaveLogsOption
} OptionSettings;

// These are the events on the settings menu
typedef enum {
    BitrateOptionEvent,
    CristyalClkOptionEvent
} SettingsMenuEvent;

// These are the sender events
typedef enum {
    ChooseIdEvent,
    SetIdEvent,
    ReturnEvent
} SenderEvents;

// These are the player events
typedef enum {
    ChooseTimingEvent,
    ReturnTimingEvent
} PlayerEvents;

// These are the options to save
typedef enum {
    NoSave,
    SaveAll,
    OnlyAddress
} SaveOptions;

// This is for SniffingTest Options
typedef enum {
    RefreshTest,
    EntryEvent,
    ShowData
} SniffingTestEvents;

// These are the events in AboutOption
typedef enum {
    ButtonGetPressed
} ButtonPressedEvent;

// Views in the App
typedef enum {
    SubmenuView,
    ViewWidget,
    VarListView,
    TextBoxView,
    DialogInfoView,
    InputByteView,
    FileBrowserView,
} scenesViews;

char* sequential_file_resolve_path(
    Storage* storage,
    const char* dir,
    const char* prefix,
    const char* extension);

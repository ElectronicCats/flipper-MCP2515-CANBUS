#include "../app_user.h"

typedef enum {
    SEND_OK,
    SEND_ERROR,
} sender_status;

// This part is for the time
enum {
    ONCE,
    PERIODIC
} sender_periods;

static uint8_t timing = 0;

static const char* timing_texts[] = {"ONCE", "PERIODIC"};
static const char* timing_multiply[] = {"x1", "x10", "x100", "x1000"};

static uint8_t time = 1;
static uint8_t multiply = 0;

// Only solution I think for the text with
// the dynamic menu in set data to get the bytes
static const char* text_bytes[] = {
    "Byte [0]",
    "Byte [1]",
    "Byte [2]",
    "Byte [3]",
    "Byte [4]",
    "Byte [5]",
    "Byte [6]",
    "Byte [7]",
};

// Data for the can Id
uint8_t can_id[4];

// Thread to work with the sender
static int32_t sender_on_work(void* context);

// Empty Callback
void empty_input_callback(void* context, uint32_t index) {
    UNUSED(context);
    UNUSED(index);
}

/**
 *  Scene for the Menu Sender
 */

// Option callback using button OK
void callback_input_sender_options(void* context, uint32_t index) {
    App* app = context;
    app->sender_selected_item = index;

    switch(index) {
    case 1:
        scene_manager_next_scene(app->scene_manager, app_scene_set_timing_option);
        break;

    case 2:
        scene_manager_next_scene(app->scene_manager, app_scene_set_data_sender_option);
        break;

    case 3:
        scene_manager_set_scene_state(app->scene_manager, app_scene_input_data_option, 0xff);
        scene_manager_next_scene(app->scene_manager, app_scene_input_data_option);

    default:
        break;
    }
}

// To display the variable list
void default_list_for_sender_menu(App* app) {
    VariableItem* item;
    variable_item_list_reset(app->varList);

    // First Item [0]
    item = variable_item_list_add(app->varList, "SEND MESSAGE", 0, NULL, app);
    variable_item_set_current_value_index(item, 0);

    // Second Item [1]
    item = variable_item_list_add(app->varList, "Timing", 0, NULL, app);
    variable_item_set_current_value_index(item, timing);
    variable_item_set_current_value_text(item, timing_texts[timing]);

    // Third Item [2]
    item = variable_item_list_add(app->varList, "DATA", 0, NULL, app);
    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "0x%lx", app->frame_to_send->canId);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // This is for the
    furi_string_reset(app->text);

    for(uint8_t i = 0; i < app->frame_to_send->data_lenght; i++) {
        if(app->frame_to_send->buffer[i] < 0x10) {
            furi_string_cat_printf(app->text, "0%x ", app->frame_to_send->buffer[i]);
        } else {
            furi_string_cat_printf(app->text, "%x ", app->frame_to_send->buffer[i]);
        }
    }

    if(app->frame_to_send->data_lenght == 0 || app->frame_to_send->req == 1) {
        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "---");
    }

    // Fourth Item [3]
    item = variable_item_list_add(app->varList, furi_string_get_cstr(app->text), 0, NULL, app);

    variable_item_list_set_enter_callback(app->varList, callback_input_sender_options, app);

    variable_item_list_set_selected_item(app->varList, app->sender_selected_item);
}

// Menu sender Scene On enter
void app_scene_sender_on_enter(void* context) {
    App* app = context;

    default_list_for_sender_menu(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

// Menu Sender On event
bool app_scene_sender_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

// Menu Sender On exit
void app_scene_sender_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}

/**
 * Scene to set the timming
 */

void set_timing_view(App* app);

void set_timings_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    timing = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, timing_texts[timing]);

    set_timing_view(app);
}

void set_time_sender(VariableItem* item) {
    App* app = variable_item_get_context(item);
    time = variable_item_get_current_value_index(item) + 1;

    furi_string_reset(app->text);

    furi_string_cat_printf(app->text, "%u ms", time);

    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));
}

void set_multiply_time(VariableItem* item) {
    multiply = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, timing_multiply[multiply]);
}

// View to display
void set_timing_view(App* app) {
    VariableItem* item;
    variable_item_list_reset(app->varList);
    variable_item_list_set_enter_callback(app->varList, empty_input_callback, app);

    // First Item
    item = variable_item_list_add(app->varList, "Timing", 2, set_timings_callback, app);
    variable_item_set_current_value_index(item, timing);
    variable_item_set_current_value_text(item, timing_texts[timing]);

    // Second Item
    if(timing) {
        item = variable_item_list_add(app->varList, "Time", 255, set_time_sender, app);
        variable_item_set_current_value_index(item, time - 1);
        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "%u ms", time);
        variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));
    } else {
        item = variable_item_list_add(app->varList, "After", 255, set_time_sender, app);
        variable_item_set_current_value_index(item, time - 1);
        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "%u ms", time);
        variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));
    }

    // Third Item

    item = variable_item_list_add(app->varList, "Multiply by", 4, set_multiply_time, app);
    variable_item_set_current_value_index(item, multiply);
    variable_item_set_current_value_text(item, timing_multiply[multiply]);
}

// Menu sender Scene On enter
void app_scene_set_timing_on_enter(void* context) {
    App* app = context;

    set_timing_view(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

// Menu Sender On event
bool app_scene_set_timing_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

// Menu Sender On exit
void app_scene_set_timing_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}

/**
 * Scene to set the timing
 */

void set_data_view(App* app);

// Go to set the option
void input_set_data(void* context, uint32_t index) {
    App* app = context;

    switch(index) {
    case 0:
        if(app->num_of_devices > 0) {
            scene_manager_next_scene(app->scene_manager, app_scene_id_list_option);
        } else {
            scene_manager_next_scene(app->scene_manager, app_scene_warning_log_sender);
        }
        break;

    default:
        scene_manager_set_scene_state(app->scene_manager, app_scene_input_data_option, index);
        scene_manager_next_scene(app->scene_manager, app_scene_input_data_option);
        break;
    }
}

// Callback for the frame
void set_frame_request_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);

    app->frame_to_send->req = variable_item_get_current_value_index(item);

    set_data_view(app);
}

// Callback to set the data length
void set_data_length_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);

    app->frame_to_send->data_lenght = variable_item_get_current_value_index(item);

    set_data_view(app);
}

// View to set the Data
void set_data_view(App* app) {
    VariableItem* item;
    variable_item_list_reset(app->varList);

    variable_item_list_set_enter_callback(app->varList, input_set_data, app);

    // first item [0]
    item = variable_item_list_add(app->varList, "Choose last ID", 0, NULL, app);

    // second item [1]
    item = variable_item_list_add(app->varList, "Set Id", 0, NULL, app);
    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "0x%lx", app->frame_to_send->canId);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    // third item [2]
    item =
        variable_item_list_add(app->varList, "Frame request", 2, set_frame_request_callback, app);
    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->req);
    variable_item_set_current_value_index(item, app->frame_to_send->req);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    if(app->frame_to_send->req) return;

    // fourth item [4]
    item = variable_item_list_add(app->varList, "Data Length", 9, set_data_length_callback, app);
    furi_string_reset(app->text);
    furi_string_cat_printf(app->text, "%u", app->frame_to_send->data_lenght);
    variable_item_set_current_value_index(item, app->frame_to_send->data_lenght);
    variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));

    for(uint8_t i = 0; i < app->frame_to_send->data_lenght; i++) {
        item = variable_item_list_add(app->varList, text_bytes[i], 0, NULL, app);

        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "0x");
        if(app->frame_to_send->buffer[i] < 0x10) {
            furi_string_cat_printf(app->text, "0");
        }
        furi_string_cat_printf(app->text, "%x", app->frame_to_send->buffer[i]);
        variable_item_set_current_value_text(item, furi_string_get_cstr(app->text));
    }
}

// Menu sender Scene On enter
void app_scene_set_data_sender_on_enter(void* context) {
    App* app = context;

    variable_item_list_set_selected_item(app->varList, 0);
    set_data_view(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, VarListView);
}

// Menu Sender On event
bool app_scene_set_data_sender_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

// Menu Sender On exit
void app_scene_set_data_sender_on_exit(void* context) {
    App* app = context;
    variable_item_list_reset(app->varList);
}

/**
 * Scene to display the when there are not ids recorded
 */

void app_scene_warning_log_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);

    widget_add_string_element(
        app->widget, 65, 20, AlignCenter, AlignBottom, FontPrimary, "W A R N I N G !");

    widget_add_string_multiline_element(
        app->widget,
        65,
        35,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "First go to the Sniffing option\nin the menu and get the ID's");

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_warning_log_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void app_scene_warning_log_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
}

/**
 *  Scene for the id list
 */

// Callback for menus id
void menu_ids_callback(void* context, uint32_t index) {
    App* app = context;
    app->frame_to_send->canId = app->frameArray[index].canId;
    app->frame_to_send->data_lenght = app->frameArray[index].data_lenght;
    app->frame_to_send->ext = app->frameArray[index].ext;
    app->frame_to_send->req = app->frameArray[index].req;

    for(uint8_t i = 0; i < (app->frame_to_send->data_lenght); i++) {
        app->frame_to_send->buffer[i] = app->frameArray[index].buffer[i];
    }

    scene_manager_handle_custom_event(app->scene_manager, ReturnEvent);
}

void app_scene_id_list_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "CAN ADDRESS TO SEND");

    for(uint8_t i = 0; i < app->num_of_devices; i++) {
        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "0x%lx", app->frameArray[i].canId);
        submenu_add_item(app->submenu, furi_string_get_cstr(app->text), i, menu_ids_callback, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

bool app_scene_id_list_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }
    return consumed;
}

void app_scene_id_list_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

/**
 * Scene used for the input on the menu sender
 */

void input_byte_sender_callback(void* context) {
    App* app = context;

    uint32_t state =
        scene_manager_get_scene_state(app->scene_manager, app_scene_input_data_option);

    switch(state) {
    case 1:
        app->frame_to_send->canId = (can_id[0] << 24) | (can_id[1] << 16) | (can_id[2] << 8) |
                                    (can_id[3]);
        break;

    default:
        break;
    }

    scene_manager_previous_scene(app->scene_manager);
}

void app_scene_input_data_on_enter(void* context) {
    App* app = context;
    ByteInput* scene = app->input_byte_value;

    uint32_t state =
        scene_manager_get_scene_state(app->scene_manager, app_scene_input_data_option);

    if(state == 1) {
        can_id[3] = app->frame_to_send->canId;
        can_id[2] = app->frame_to_send->canId >> 8;
        can_id[1] = app->frame_to_send->canId >> 16;
        can_id[0] = app->frame_to_send->canId >> 24;

        byte_input_set_header_text(scene, "SET ID");
        byte_input_set_result_callback(scene, input_byte_sender_callback, NULL, app, can_id, 4);
    }
    if(state < 0xff) {
        state = state - 4;

        furi_string_reset(app->text);
        furi_string_cat_printf(app->text, "Set Byte [%lu]", state);
        byte_input_set_header_text(scene, furi_string_get_cstr(app->text));
        byte_input_set_result_callback(
            scene, input_byte_sender_callback, NULL, app, &(app->frame_to_send->buffer[state]), 1);
    }

    if(state == 0xff) {
        byte_input_set_header_text(scene, "Set Data");
        byte_input_set_result_callback(
            scene,
            input_byte_sender_callback,
            NULL,
            app,
            app->frame_to_send->buffer,
            app->frame_to_send->data_lenght);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, InputByteView);
}

bool app_scene_input_data_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    UNUSED(event);
    UNUSED(app);

    return consumed;
}

void app_scene_input_data_on_exit(void* context) {
    UNUSED(context);
}

/**
 *  Scene for display the sender
 */

// Sender on enter
void app_scene_send_message_on_enter(void* context) {
    App* app = context;

    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 65, 32, AlignCenter, AlignCenter, FontPrimary, "Waiting to send...");

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);

    app->thread = furi_thread_alloc_ex("Sender_on_work", 1024, sender_on_work, app);
    furi_thread_start(app->thread);
}

// Sender on event
bool app_scene_send_message_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SEND_OK:
            draw_send_ok(app);
            break;

        case SEND_ERROR:
            draw_send_wrong(app);
            break;

        case DEVICE_NO_CONNECTED:
            draw_device_no_connected(app);
            break;

        default:
            break;
        }
    }

    return consumed;
}

// Sender on exit
void app_scene_send_message_on_exit(void* context) {
    App* app = context;
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);

    widget_reset(app->widget);
}

/**
 * Thread to work with
 */

static int32_t sender_on_work(void* context) {
    App* app = context;
    app->mcp_can->mode = MCP_NORMAL;
    ERROR_CAN debug = ERROR_OK;
    ERROR_CAN error = ERROR_OK;
    debug = mcp2515_init(app->mcp_can);

    furi_delay_ms(10);

    if(debug == ERROR_OK) {
        error = send_can_frame(app->mcp_can, app->frame_to_send);
        furi_delay_ms(500);

        if(error != ERROR_OK)
            scene_manager_handle_custom_event(app->scene_manager, SEND_ERROR);
        else
            scene_manager_handle_custom_event(app->scene_manager, SEND_OK);
    } else {
        scene_manager_handle_custom_event(app->scene_manager, DEVICE_NO_CONNECTED);
    }

    free_mcp2515(app->mcp_can);
    return 0;
}

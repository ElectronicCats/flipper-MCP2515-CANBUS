#include "../app.h"

uint32_t state = 0;

typedef enum {
    STATE_START,
    STATE_WAITING,
    STATE_EXIT
} STATES;

// Times
uint32_t time_delay = 1500;
uint32_t time_waiting = 3000;

// Call for the thread
int32_t testing_thread(void* context);

/**
 * Scene Functions
 */

void app_scene_testing_on_enter(void* context) {
    App* app = (App*)context;

    state = scene_manager_get_scene_state(app->scene_manager, app_scene_testing_option);

    widget_reset(app->widget);

    app->thread = furi_thread_alloc_ex("TestingScene", 2 * 1024, testing_thread, app);
    furi_thread_start(app->thread);

    view_dispatcher_switch_to_view(app->view_dispatcher, WidgetView);
}

bool app_scene_testing_on_event(void* context, SceneManagerEvent event) {
    bool consumed = false;
    App* app = (App*)context;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void app_scene_testing_on_exit(void* context) {
    App* app = (App*)context;
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
}

/**
 * Draws
 */
// Draw for the state
void draw_start(Widget* widget, uint32_t node) {
    widget_reset(widget);

    if(node == 0)
        widget_add_string_element(widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Nodo 1");
    if(node == 1)
        widget_add_string_element(widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Nodo 2");
    if(node == 2)
        widget_add_string_element(
            widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "AutoTest");

    widget_add_button_element(widget, GuiButtonTypeCenter, "Start", NULL, NULL);
}

void draw_no_connected(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "No Reconocido");

    furi_delay_ms(time_waiting);
}

void draw_sending(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Enviando Msg");
}

void draw_waiting(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Esperando Msg");
}

void draw_message_received(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Recibido OK");
    furi_delay_ms(time_waiting);
}

void draw_message_sent(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Enviado OK");
    furi_delay_ms(time_waiting);
}

void draw_message_received_fail(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Recibido Fail");
    furi_delay_ms(time_waiting);
}

void draw_message_sent_fail(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Enviado Fail");
    furi_delay_ms(time_waiting);
}

void drawe_all_okay(Widget* widget) {
    widget_reset(widget);
    widget_add_string_element(widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "FUNCIONA!!");
    furi_delay_ms(time_waiting);
}

/**
 *  Funciones para cada uno
 */

void function_node_one(App* app) {
    CANFRAME frame;
    CANFRAME frame_to_received;

    Widget* widget = app->widget;

    frame.canId = 0x200;

    frame.data_lenght = 8;

    for(uint8_t i = 0; i < 8; i++) {
        frame.buffer[i] = i;
    }

    draw_start(widget, state);

    /**
     * Estado Normal para empezar
     */
    app->mcp_can->mode = MCP_NORMAL;

    /**
     * Esperando para comenzar
     */
    while(true) {
        if(furi_hal_gpio_read(&gpio_button_ok)) break;

        if(!furi_hal_gpio_read(&gpio_button_back)) return;
    }

    /**
     * Comienza la comunicacion con el MCP2515
     */

    if(mcp2515_init(app->mcp_can) != ERROR_OK) {
        draw_no_connected(widget);
        return;
    }

    uint32_t time_pass = furi_get_tick();

    /**
     * Leer mensaje
     */
    draw_waiting(widget);
    time_pass = furi_get_tick();
    while(read_can_message(app->mcp_can, &frame_to_received) != ERROR_OK) {
        if((furi_get_tick() - time_pass) > time_waiting) {
            return;
        }
    }

    // comprobacion del mensaje
    for(uint8_t i = 0; i < frame_to_received.data_lenght; i++) {
        if(frame_to_received.buffer[i] != i) {
            return;
        }
    }

    draw_message_received(widget);

    /**
     * Enviar mensaje
     */

    draw_sending(widget);
    time_pass = furi_get_tick();
    while(send_can_frame(app->mcp_can, &frame) != ERROR_OK) {
        if((furi_get_tick() - time_pass) > time_waiting) {
            draw_message_sent_fail(widget);
            return;
        }
    }

    draw_message_sent(widget);

    drawe_all_okay(widget);
}

void function_node_two(App* app) {
    CANFRAME frame;
    CANFRAME frame_to_received;

    Widget* widget = app->widget;

    frame.canId = 0x100;

    frame.data_lenght = 8;

    for(uint8_t i = 0; i < 8; i++) {
        frame.buffer[i] = i;
    }

    draw_start(widget, state);

    /**
     * Estado Normal para empezar
     */
    app->mcp_can->mode = MCP_NORMAL;

    /**
     * Esperando para comenzar
     */
    while(true) {
        if(furi_hal_gpio_read(&gpio_button_ok)) break;

        if(!furi_hal_gpio_read(&gpio_button_back)) return;
    }

    /**
     * Comienza la comunicacion con el MCP2515
     */

    if(mcp2515_init(app->mcp_can) != ERROR_OK) {
        draw_no_connected(widget);
        return;
    }

    /**
     * Estado para hacer el envio de mensaje
     */

    draw_sending(widget);
    uint32_t time_pass = furi_get_tick();
    while(send_can_frame(app->mcp_can, &frame) != ERROR_OK) {
        if((furi_get_tick() - time_pass) > time_waiting) {
            draw_message_sent_fail(widget);
            return;
        }
    }

    draw_message_sent(widget);

    /**
     * Estado esperando
     */

    draw_waiting(widget);
    time_pass = furi_get_tick();
    while(read_can_message(app->mcp_can, &frame_to_received) != ERROR_OK) {
        if((furi_get_tick() - time_pass) > time_waiting) {
            draw_message_received_fail(widget);
            return;
        }
    }

    // comprobacion del mensaje
    for(uint8_t i = 0; i < frame_to_received.data_lenght; i++) {
        if(frame_to_received.buffer[i] != i) {
            draw_message_received_fail(widget);
            return;
        }
    }

    draw_message_received(widget);

    drawe_all_okay(widget);
}

void auto_test(App* app) {
    CANFRAME frame;
    CANFRAME frame_to_received;

    Widget* widget = app->widget;

    frame.canId = 0x100;

    frame.data_lenght = 8;

    for(uint8_t i = 0; i < 8; i++) {
        frame.buffer[i] = i;
    }

    draw_start(widget, state);

    /**
     * Estado Normal para empezar
     */
    app->mcp_can->mode = MCP_LOOPBACK;

    /**
     * Esperando para comenzar
     */
    while(true) {
        if(furi_hal_gpio_read(&gpio_button_ok)) break;

        if(!furi_hal_gpio_read(&gpio_button_back)) return;
    }

    /**
     * Comienza la comunicacion con el MCP2515
     */

    if(mcp2515_init(app->mcp_can) != ERROR_OK) {
        draw_no_connected(widget);
        return;
    }

    /**
     * Estado para hacer el envio de mensaje
     */

    draw_sending(widget);
    uint32_t time_pass = furi_get_tick();
    while(send_can_frame(app->mcp_can, &frame) != ERROR_OK) {
        if((furi_get_tick() - time_pass) > time_waiting) {
            draw_message_sent_fail(widget);
            return;
        }
    }

    draw_message_sent(widget);

    /**
     * Estado esperando
     */

    draw_waiting(widget);
    time_pass = furi_get_tick();
    while(read_can_message(app->mcp_can, &frame_to_received) != ERROR_OK) {
        if((furi_get_tick() - time_pass) > time_waiting) {
            draw_message_received_fail(widget);
            return;
        }
    }

    // comprobacion del mensaje
    for(uint8_t i = 0; i < frame_to_received.data_lenght; i++) {
        if(frame_to_received.buffer[i] != i) {
            draw_message_received_fail(widget);
            return;
        }
    }

    draw_message_received(widget);

    drawe_all_okay(widget);
}

/**
 * Thread for the testing
 */

int32_t testing_thread(void* context) {
    App* app = (App*)context;

    while(furi_hal_gpio_read(&gpio_button_back)) {
        if(state == 0) function_node_one(app);
        if(state == 1) function_node_two(app);
        if(state == 2) auto_test(app);
    }

    return 0;
}

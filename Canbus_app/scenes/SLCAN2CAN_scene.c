#include "app_user.h"

#define TAG "SLCAN 2 CAN"

/*void frames_dialog_callback(DialogExResult result, void* context) {

}*/

void app_scene_SLCAN_2_CAN_on_enter(void* context) {
    furi_assert(context);
    App* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, LoadingView);
    /**/
    FuriString* SLCAN_received = furi_string_alloc();
    FuriString* can_id_received = furi_string_alloc();
    FuriString* dlc_received = furi_string_alloc();
    CANFRAME frame_received;
    uint8_t* buffer = calloc(30, sizeof(buffer));

    while(true) {
        uint32_t num_recived = furi_hal_cdc_receive(LAWICEL_CDC_NUM, buffer, 30);

        if(num_recived && (buffer[0] == 't' || buffer[0] == 'T')) {
            furi_string_set_str(SLCAN_received, (char*)buffer);

            furi_string_set(can_id_received, SLCAN_received);
            furi_string_set(dlc_received, SLCAN_received);

            frame_received.ext = buffer[0] == 'T';

            furi_string_mid(can_id_received, 1, frame_received.ext ? 8 : 3);

            frame_received.data_lenght =
                (uint8_t)(furi_string_get_char(SLCAN_received, frame_received.ext ? 9 : 4) - '0');

            furi_string_mid(
                dlc_received, frame_received.ext ? 10 : 5, frame_received.data_lenght * 2);

            if(frame_received.ext) *(char*)furi_string_get_cstr(can_id_received) -= 8;

            for(int i = 0; i < frame_received.data_lenght * 2; i += 2) {
                frame_received.buffer[(i / 2) + 1] = hex2uint8(
                    (char*)furi_string_get_cstr(dlc_received) + i,
                    (char*)furi_string_get_cstr(dlc_received) + i + 1);
            }

            frame_received.canId = 0;
            for(int i = 0; i < (frame_received.ext ? 8 : 3); i++) {
                frame_received.canId +=
                    hex2uint8_nibble((char*)(furi_string_get_cstr(can_id_received) + i))
                    << 4 * ((frame_received.ext ? 7 : 2) - i);
            }
        }

        for(uint8_t i = 0; i < num_recived; i++)
            buffer[i] = '\0';
    }

    free(buffer);
    furi_string_free(dlc_received);
    furi_string_free(can_id_received);
    furi_string_free(SLCAN_received);
    /**/
}

bool app_scene_SLCAN_2_CAN_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void app_scene_SLCAN_2_CAN_on_exit(void* context) {
    furi_assert(context);
}

#include "uds_library.h"

// Function to malloc the instance
UDS_SERVICE* uds_service_alloc(
    uint32_t id_to_send,
    uint32_t id_to_received,
    MCP_MODE mode,
    MCP_CLOCK clk,
    MCP_BITRATE bitrate) {
    UDS_SERVICE* instance = malloc(sizeof(UDS_SERVICE));
    instance->CAN = mcp_alloc(mode, clk, bitrate);
    instance->id_to_send = id_to_send;
    instance->id_to_received = id_to_received;

    return instance;
}

// Init the mcp2515 with it respeclty mask and filters
bool uds_init(UDS_SERVICE* uds_instance) {
    MCP2515* CAN = uds_instance->CAN;
    if(mcp2515_init(CAN) != ERROR_OK) return false;

    uint32_t mask = 0x7ff;

    if(uds_instance->id_to_received > 0x7ff) {
        mask = 0x1FFFFFFF;
    }

    init_mask(CAN, 0, mask);
    init_filter(CAN, 0, uds_instance->id_to_received);

    init_mask(CAN, 1, mask);
    init_filter(CAN, 1, uds_instance->id_to_received);

    return true;
}

// Free instance
void free_uds(UDS_SERVICE* uds_instance) {
    free_mcp2515(uds_instance->CAN);
    free(uds_instance->CAN);
    free(uds_instance);
}

// Get Frames
bool read_frames_uds(MCP2515* CAN, uint32_t id, CANFRAME* frame) {
    uint32_t time_delay = 0;

    do {
        if(read_can_message(CAN, frame) == ERROR_OK) {
            if(frame->canId == id) {
                return true;
            }
        }
        furi_delay_us(1);
        time_delay++;

    } while((time_delay < 6000));

    // log_exception("Error");

    return false;
}

// Function to send a service
bool uds_single_frame_request(
    UDS_SERVICE* uds_instance,
    uint8_t* data_to_send,
    uint8_t count_of_bytes,
    CANFRAME* frames_to_received,
    uint8_t count_of_frames) {
    MCP2515* CAN = uds_instance->CAN;
    CANFRAME frame_to_send = {0};
    frame_to_send.canId = uds_instance->id_to_send;
    frame_to_send.data_lenght = count_of_bytes + 1;
    uint32_t id_to_received = uds_instance->id_to_received;
    ERROR_CAN ret = ERROR_OK;

    for(uint8_t i = 0; i < frame_to_send.data_lenght; i++)
        frame_to_send.buffer[i] = data_to_send[i];

    ret = send_can_frame(CAN, &frame_to_send);

    if(ret != ERROR_OK) return false;

    memset(frame_to_send.buffer, 0, sizeof(frame_to_send.buffer));
    frame_to_send.buffer[0] = 0x30;
    frame_to_send.data_lenght = 3;

    for(uint8_t i = 0; i < count_of_frames; i++) {
        if(i == 1) {
            ret = send_can_frame(CAN, &frame_to_send);
            if(ret != ERROR_OK) return false;
        }

        if(!read_frames_uds(CAN, id_to_received, &(frames_to_received[i]))) {
            if(i == 0)
                return false;
            else
                break;
        }
    }

    return true;
}

// Function to get VIN
bool uds_get_vin(UDS_SERVICE* uds_instance, FuriString* text) {
    CANFRAME canframes[3];

    memset(canframes, 0, sizeof(canframes));

    uint8_t request[4] = {0x3, 0x22, 0xf1, 0x90};

    if(!uds_single_frame_request(uds_instance, request, 3, canframes, 3)) return false;

    if(canframes[0].buffer[2] != 0x62) return false;

    char vin_name[17] = {'\0'};

    uint8_t pos = 0;

    for(uint8_t i = 0; i < 3; i++) {
        uint8_t start_num = (i == 0) ? 5 : 1;

        // Save bytes in the char array
        for(uint8_t j = start_num; j < 8; j++) {
            vin_name[pos] = (char)canframes[i].buffer[j];
            if(pos >= sizeof(vin_name) - 1) break;
            pos++;
        }
    }

    furi_string_reset(text);

    furi_string_cat_printf(text, "%.17s", vin_name);

    return true;
}

// Function to send multiframes
// This will be on development
bool uds_multi_frame_request(
    UDS_SERVICE* uds_instance,
    uint8_t* data,
    uint8_t length,
    CANFRAME* canframes_to_send,
    uint8_t count_of_frames_to_received,
    CANFRAME* canframes_to_received) {
    uint8_t size_frames_to_send = 1;

    // Condition to know the count of frames to send if the data need more frames
    if(length > 7) {
        if(((length - 6) % 7) != 0) {
            size_frames_to_send = ((length - 6) / 7) + 2;
        } else {
            size_frames_to_send = ((length - 6) / 7) + 1;
        }
    }

    // log_info("count of frames to send: %u", size_frames_to_send);

    // log_info("Start"); // 0

    // Set the frames if the data need more than one can frame
    if(size_frames_to_send > 1) {
        canframes_to_send[0].buffer[0] = 0x10; // Set the byte to indicate the first frame
        canframes_to_send[0].buffer[1] = length; // The length of the data

        // This counter works as a pivot to save the data in it respective byte of any frame
        uint8_t counter = 0;

        for(uint8_t i = 0; i < size_frames_to_send; i++) {
            canframes_to_send[i].canId = uds_instance->id_to_send; // Set the id to be sent
            canframes_to_send[i].data_lenght = 8; // Set the lenght

            if(i >= 1) canframes_to_send[i].buffer[0] = (0x20) + (i & 0xf); // every

            uint8_t start_num = (i == 0) ? 2 : 1;

            for(uint8_t j = start_num; j < 8; j++) {
                // log_info("counter: %u data: %x", counter, data[counter]);
                canframes_to_send[i].buffer[j] = data[counter++];

                if(counter == length) {
                    canframes_to_send[i].data_lenght = j + 1;
                }
            }
        }
    }

    // If the data only needs one frame
    else {
        canframes_to_send[0].canId = uds_instance->id_to_send;
        canframes_to_send[0].data_lenght = length + 1;
        canframes_to_send[0].buffer[0] = length;
        for(uint8_t i = 1; i < (length + 1); i++) {
            canframes_to_send[0].buffer[i] = data[i - 1];
        }
    }

    canframes_to_send[size_frames_to_send].canId = uds_instance->id_to_send;
    canframes_to_send[size_frames_to_send].data_lenght = 3;
    canframes_to_send[size_frames_to_send].buffer[0] = 0x30;

    log_info("Messages already set, count of frames: %u", size_frames_to_send); // 1

    //  Just for debbug
    FuriString* text = furi_string_alloc();

    log_info("------------To send--------------------------");

    for(uint8_t j = 0; j < size_frames_to_send; j++) {
        furi_string_reset(text);
        furi_string_cat_printf(text, "%lx\t", canframes_to_send[j].canId);
        for(uint8_t i = 0; i < canframes_to_send[j].data_lenght; i++) {
            furi_string_cat_printf(text, "%x ", canframes_to_send[j].buffer[i]);
        }

        log_info(furi_string_get_cstr(text));
    }

    furi_string_free(text);

    // From here is the work to send de canbus data

    // Send the first frame
    if(send_can_frame(uds_instance->CAN, &canframes_to_send[0]) != ERROR_OK) {
        log_exception("First message wasnt send");
        return false;
    }

    // Wait message of the response
    if(!read_frames_uds(
           uds_instance->CAN, uds_instance->id_to_received, &canframes_to_received[0])) {
        log_exception("message wasnt received");
        return false;
    }

    // To received multiple frames
    if(canframes_to_received[0].buffer[0] == 0x10 && count_of_frames_to_received > 1) {
        // If there no more data will pass this code
        send_can_frame(uds_instance->CAN, &canframes_to_send[size_frames_to_send]);

        for(uint8_t i = 1; i < count_of_frames_to_received; i++) {
            if(!read_frames_uds(
                   uds_instance->CAN, uds_instance->id_to_received, &canframes_to_received[i]))
                break;
        }
    }

    // To know if it is only one frame to send
    if(size_frames_to_send == 1) {
        canframes_to_send[size_frames_to_send].canId = 0;
        return true;
    }

    /*
        Here is the end for only one frame received.
        The next code describe how to send multple frames
    */

    // If the flow control is not ok
    if(canframes_to_received[0].buffer[0] != 0x30) {
        log_exception("Message to flow_control was a error");
        return false;
    }

    // Send the rest of the data
    for(uint8_t i = 1; i < size_frames_to_send; i++) {
        send_can_frame(uds_instance->CAN, &canframes_to_send[i]);
    }

    // Read the first ECU's response
    if(!read_frames_uds(
           uds_instance->CAN, uds_instance->id_to_received, &canframes_to_received[0])) {
        log_exception("Error to read the frame");
        return false;
    }

    // To received multiple frames
    if(canframes_to_received[0].buffer[0] == 0x10 && count_of_frames_to_received > 1) {
        // If there no more data will pass this code
        send_can_frame(uds_instance->CAN, &canframes_to_send[size_frames_to_send]);

        for(uint8_t i = 1; i < count_of_frames_to_received; i++) {
            if(!read_frames_uds(
                   uds_instance->CAN, uds_instance->id_to_received, &canframes_to_received[i]))
                break;
        }
    }

    canframes_to_send[size_frames_to_send].canId = 0;

    return true;
}

// Set diagnostic session
bool uds_set_diagnostic_session(UDS_SERVICE* uds_instance, diagnostic_session session) {
    uint8_t data[2] = {0x10, (uint8_t)session};

    if(session == 0) return false;

    CANFRAME frame_to_send = {0};
    CANFRAME frame_to_received = {0};

    if(!uds_multi_frame_request(
           uds_instance, data, COUNT_OF(data), &frame_to_send, 1, &frame_to_received))
        return false;

    if(frame_to_received.buffer[1] != 0x50) return false;

    return true;
}

// Reset the ECU
bool uds_reset_ecu(UDS_SERVICE* uds_instance, type_ecu_reset type) {
    uint8_t data[2] = {0x11, (uint8_t)type};

    if(type == 0) return false;

    CANFRAME frame_to_send = {0};
    CANFRAME frame_to_received = {0};

    if(!uds_multi_frame_request(
           uds_instance, data, COUNT_OF(data), &frame_to_send, 1, &frame_to_received))
        return false;

    if(frame_to_received.buffer[1] != 0x51) return false;

    return true;
}

// Get count of DTC
bool uds_get_count_stored_dtc(UDS_SERVICE* uds_instance, uint16_t* count_of_dtc) {
    uint8_t data[3] = {0x19, 0x1, 0xff};

    CANFRAME frame_to_send = {0};
    CANFRAME frame_to_received = {0};

    if(!uds_multi_frame_request(
           uds_instance, data, COUNT_OF(data), &frame_to_send, 1, &frame_to_received))
        return false;

    if(frame_to_received.buffer[1] != 0x59) return false;

    *count_of_dtc = (uint16_t)frame_to_received.buffer[5] << 8 | frame_to_received.buffer[6];

    return true;
}

// Get the DTC
bool uds_get_stored_dtc(UDS_SERVICE* uds_instance, uint8_t* codes, uint16_t* count_of_dtc) {
    // To get the count of DTC stored
    if(!uds_get_count_stored_dtc(uds_instance, count_of_dtc)) {
        log_exception("Salimos Aqui 1");
        return true;
    }

    UNUSED(codes);

    log_info("Vamos aqui 1");

    uint8_t data[3] = {0x19, 0x2, 0xff};

    CANFRAME frame_to_send = {0};
    CANFRAME* frame_to_received = calloc(5, sizeof(CANFRAME));

    log_info("Vamos aqui 2");

    // Get the canframes with the data
    if(!uds_multi_frame_request(
           uds_instance, data, COUNT_OF(data), &frame_to_send, 5, frame_to_received)) {
        log_exception("Salimos Aqui 2");
        free(frame_to_received);
        return false;
    }

    log_info("Vamos aqui 3");

    // If the message has error
    if(frame_to_received[0].buffer[0] == 0x7F) {
        log_exception("Salimos Aqui 3");
        free(frame_to_received);
        return false;
    }

    log_info("Vamos aqui 4");

    // If the data has only 1 DTC code
    if(*count_of_dtc == 1) {
        log_info("Pues practicamente esto esta bien");

        for(uint8_t i = 3; i < frame_to_received[0].data_lenght; i++) {
            log_info("value of %u %x", i - 3, frame_to_received[0].buffer[i]);
            codes[i - 3] = frame_to_received[0].buffer[i];
        }

        log_info("Casi Sale");

        free(frame_to_received);

        log_info("Debe Sale");

        return true;
    }

    // If the data has more than only one dtc

    log_info("Vamos aqui 5");

    uint8_t count_of_bytes = frame_to_received[0].buffer[1] - 2;

    uint8_t data_codes[count_of_bytes];

    UNUSED(data_codes);

    for(uint8_t i = 0; i < 5; i++) {
        if(frame_to_received[i].canId != uds_instance->id_to_received) break;

        uint32_t start_num = (i == 0) ? 4 : 1;

        uint8_t counter = 0;

        for(uint8_t j = start_num; j < frame_to_received[i].data_lenght; j++) {
            data_codes[counter++] = frame_to_received[i].buffer[j];
        }
    }

    log_info("Vamos aqui 6");

    free(frame_to_received);
    return true;
}

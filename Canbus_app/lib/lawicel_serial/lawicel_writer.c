#include "lawicel_serial.h"

#include <files_scaner.h>

#define TAG "LAWICEL WRITER"

void lawicel_send_frame(FrameCAN* frame) {
    FuriString* lawicel_frame = furi_string_alloc();

    furi_string_replace_all_str(frame->dlc, " ", "");

    furi_string_printf(
        lawicel_frame,
        "T%s%s%s%04X\r",
        furi_string_get_cstr(frame->can_id),
        furi_string_get_cstr(frame->len),
        furi_string_get_cstr(frame->dlc),
        (uint16_t)atoi(furi_string_get_cstr(frame->timestamp)));

    furi_hal_cdc_send(
        LAWICEL_CDC_NUM,
        (uint8_t*)furi_string_get_cstr(lawicel_frame),
        furi_string_size(lawicel_frame));

    furi_string_free(lawicel_frame);
}

void lawicel_send_log(Storage* storage, FuriString* log_path) {
    Stream* stream = file_stream_alloc(storage);
    FuriString* line = furi_string_alloc();
    FrameCAN* frame = frame_can_alloc();

    file_stream_open(stream, furi_string_get_cstr(log_path), FSAM_READ, FSOM_OPEN_EXISTING);

    while(stream_read_line(stream, line)) {
        frame_splitter(frame, line);
        lawicel_send_frame(frame);
    }

    frame_can_free(frame);
    furi_string_free(line);
    stream_free(stream);
}

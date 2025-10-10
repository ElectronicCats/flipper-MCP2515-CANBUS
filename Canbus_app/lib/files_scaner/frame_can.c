#include "frame_can.h"

FrameCAN* frame_can_alloc() {
    FrameCAN* frame = malloc(sizeof(FrameCAN));

    frame->timestamp = furi_string_alloc();
    frame->type = furi_string_alloc();
    frame->can_id = furi_string_alloc();
    frame->len = furi_string_alloc();
    frame->dlc = furi_string_alloc();

    return frame;
}

void frame_can_free(FrameCAN* frame) {
    furi_string_free(frame->timestamp);
    furi_string_free(frame->type);
    furi_string_free(frame->can_id);
    furi_string_free(frame->len);
    furi_string_free(frame->dlc);

    free(frame);
}

#include "frame_can.h"

FrameCAN* frame_can_alloc() {
    FrameCAN* frame = malloc(sizeof(FrameCAN));

    frame->can_id = furi_string_alloc();
    frame->can_data = furi_string_alloc();

    return frame;
}

void frame_can_free(FrameCAN* frame) {
    furi_string_free(frame->can_id);
    furi_string_free(frame->can_data);

    free(frame);
}

#pragma once

#include <furi.h>

typedef struct {
    FuriString* can_id;
    FuriString* can_data;
} FrameCAN;

FrameCAN* frame_can_alloc();

void frame_can_free(FrameCAN* frame);

#pragma once

#include <furi.h>

typedef struct {
    FuriString* timestamp;
    FuriString* type;
    FuriString* can_id;
    FuriString* len;
    FuriString* dlc;
} FrameCAN;

FrameCAN* frame_can_alloc(void);

void frame_can_free(FrameCAN* frame);

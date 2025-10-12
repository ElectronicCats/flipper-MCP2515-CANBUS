#pragma once

#include <furi_hal.h>
#include <furi_hal_usb_cdc.h>

#include <storage/storage.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>

#include <frame_can.h>

#define LAWICEL_CDC_NUM 1

typedef enum {
    LAWICEL_SEND_FRAME,
    LAWICEL_SEND_LOG,
} LAWICEL_OPTIONS;

bool lawicel_open_port(void);
bool lawicel_close_port(void);

void lawicel_send_frame(FrameCAN* frame);
void lawicel_send_log(Storage* storage, FuriString* log_path);
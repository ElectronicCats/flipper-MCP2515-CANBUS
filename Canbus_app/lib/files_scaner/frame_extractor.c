#include "files_scaner.h"
#include "frame_can.h"

#define STREAM_BUFFER_SIZE (32U)

#define TAG "FRAMES EXTRACTOR"

uint64_t stream_get_lines_count(Stream* stream) {
    uint64_t lines_count = 0;

    for(;;) {
        if(stream_seek_to_char(stream, '\n', StreamDirectionForward)) {
            lines_count++;
        } else {
            if(stream_tell(stream) < stream_size(stream) - 1) {
                lines_count++;
            }
            break;
        }
    }

    stream_rewind(stream);

    return lines_count;
}

bool stream_read_line_index(Stream* stream, FuriString* str_result, uint64_t line_index) {
    furi_string_reset(str_result);

    uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * STREAM_BUFFER_SIZE);

    uint64_t lines_count = 0;

    if(line_index < stream_get_lines_count(stream)) {
        do {
            bool error = false;
            bool result = false;
            uint16_t bytes_were_read = stream_read(stream, buffer, STREAM_BUFFER_SIZE);
            if(bytes_were_read == 0) {
                error = true;
                break;
            }

            for(uint16_t i = 0; i < bytes_were_read; i++) {
                if(buffer[i] == '\n') {
                    lines_count++;
                    if(!stream_seek(stream, i - bytes_were_read + 1, StreamOffsetFromCurrent)) {
                        error = true;
                        break;
                    }
                    furi_string_push_back(str_result, buffer[i]);
                    if(lines_count - 1 == line_index) {
                        result = true;
                    } else {
                        furi_string_reset(str_result);
                    }
                    break;
                } else if(buffer[i] == '\r') {
                    // Ignore
                } else {
                    furi_string_push_back(str_result, buffer[i]);
                }
            }

            if(result || error) {
                break;
            }
        } while(true);
    }

    stream_rewind(stream);

    free(buffer);

    return furi_string_size(str_result) != 0;
}

void frame_spliter(FrameCAN* frame, FuriString* frame_line) {
    size_t position = furi_string_search_char(frame_line, ':', 0);

    if(position != FURI_STRING_FAILURE) {
        furi_string_set(frame->can_id, frame_line);
        furi_string_set(frame->can_data, frame_line);

        furi_string_left(frame->can_id, position);
        furi_string_right(frame->can_data, position + 1);
    } else {
        FURI_LOG_E(TAG, "Error: can't read CAN format");
    }
}

void frame_extractor(Storage* storage, const char* path, FrameCAN* frame, uint64_t index) {
    Stream* stream = file_stream_alloc(storage);
    FuriString* line = furi_string_alloc();

    if(file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(stream_read_line_index(stream, line, index)) {
            frame_spliter(frame, line);
        } else {
            FURI_LOG_E(TAG, "Failed to read line");
        }
    } else {
        FURI_LOG_E(TAG, "Failed to open file");
    }

    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
}

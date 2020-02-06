#include "pbm_decoder.h"
#include "baselib.h"

static struct Pixel pixel;

void decode_pbm(void* pbm_data, size_t size, struct PbmImage* pbm_struct) {
    uint8_t* pbm_data_ptr = (uint8_t*) pbm_data;
    uint8_t* previous_pbm_data_ptr;
    size_t remaining_bytes = size - 2;

    unsigned int width;
    unsigned int height;
    unsigned int row_bytes;

    // PBM format specification is at http://netpbm.sourceforge.net/doc/pbm.html
    if (size >= 7 && *pbm_data_ptr++ == 'P' && *pbm_data_ptr++ == '4') {
        // Size and magic looks OK. Perform actual decoding

        // Ignore whitespaces after magic
        while (isspace(*pbm_data_ptr) && remaining_bytes > 0) {
            ++pbm_data_ptr;
            --remaining_bytes;
        }

        // Nothing more to do if end of input is reached
        if (remaining_bytes == 0) {
            return;
        }

        previous_pbm_data_ptr = pbm_data_ptr;
        width = strtoui(&pbm_data_ptr, remaining_bytes);
        remaining_bytes -= pbm_data_ptr - previous_pbm_data_ptr;
        if (width == 0 || remaining_bytes == 0) {
            return; // Not an integer, or invalid image
        }

        // More whitespace
        while (isspace(*pbm_data_ptr) && remaining_bytes > 0) {
            ++pbm_data_ptr;
            --remaining_bytes;
        }

        if (remaining_bytes == 0) {
            return;
        }

        previous_pbm_data_ptr = pbm_data_ptr;
        height = strtoui(&pbm_data_ptr, remaining_bytes);
        remaining_bytes -= pbm_data_ptr - previous_pbm_data_ptr;
        if (height == 0 || remaining_bytes == 0) {
            return;
        }

        // Consume raster delimiter
        if (--remaining_bytes < 1) {
            return;
        }

        // Raster data delimiter whitespace
        ++pbm_data_ptr;

        // Header decoded successfully, raster data from now on.
        // Check its size is OK
        row_bytes = width / 8 + (width % 8 == 0 ? 0 : 1);
        if (remaining_bytes == row_bytes * height) {
            pbm_struct->width = width;
            pbm_struct->height = height;
            pbm_struct->raster_pos = pbm_data_ptr;
            pbm_struct->current_row_pixel = 0;
        }
    }
}

struct Pixel* next_pixel(struct PbmImage* pbm_image) {
    uint8_t current_byte = *(uint8_t*) pbm_image->raster_pos;
    uint8_t current_byte_index = 7 - (pbm_image->current_row_pixel % 8);

    if (((current_byte >> current_byte_index) & 1) == 0) {
        pixel.r = pbm_image->palette->low_r;
        pixel.g = pbm_image->palette->low_g;
        pixel.b = pbm_image->palette->low_b;
    } else {
        pixel.r = pbm_image->palette->high_r;
        pixel.g = pbm_image->palette->high_g;
        pixel.b = pbm_image->palette->high_b;
    }

    unsigned int new_row_pixel = pbm_image->current_row_pixel + 1;
    bool moved_to_next_row = new_row_pixel == pbm_image->width;

    // We need to advance raster_pos (and reset the byte index)
    // if the row ends here, or there are no more bits left in this byte
    if (current_byte_index == 0 || moved_to_next_row) {
        ++pbm_image->raster_pos;
    }

    // We need to reset row pixel count if we moved to the next row
    if (moved_to_next_row) {
        new_row_pixel = 0;
    }

    pbm_image->current_row_pixel = new_row_pixel;

    return &pixel;
}

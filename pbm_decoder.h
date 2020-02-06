#pragma once

#include <stddef.h>
#include <stdint.h>

struct PbmPalette {
    uint8_t low_r;
    uint8_t low_g;
    uint8_t low_b;
    uint8_t high_r;
    uint8_t high_g;
    uint8_t high_b;
};

struct PbmImage {
    unsigned int width;
    unsigned int height;
    void* raster_pos;
    unsigned int current_row_pixel;
    struct PbmPalette* palette;
};

struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/*
 * Decodes a raw Portable Bit Map image, as defined by Netpbm, filling
 * the provided pbm_struct with appropriate values. It is assumed that
 * the palette is initialized by the caller. If the image couldn't be
 * decoded, pbm_struct is unchanged.
 *
 * This decoder doesn't support comments embedded in the PBM.
 * This simplifies the decoder and forces users to delete useless
 * comments from the PBM, reducing payload size further.
 */
void decode_pbm(void* pbm_data, size_t size, struct PbmImage* pbm_struct);

/*
 * Returns the next pixel of a previously decoded PBM image and
 * advances pointers.
 * If there are no more pixels, the behavior of this function is
 * undefined.
 */
struct Pixel* next_pixel(struct PbmImage* pbm_image);

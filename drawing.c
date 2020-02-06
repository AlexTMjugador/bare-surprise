#include "drawing.h"
#include "vbe.h"

static struct Pixel constant_pixel;
static struct PbmImage* current_image;
static uint8_t current_x_scale;

static struct Pixel* constant_pixel_producer(void);
static struct Pixel* image_pixel_producer(void);

static void internal_fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, struct Pixel*(*pixel_producer)(void));

struct Pixel* constant_pixel_producer(void) {
    return &constant_pixel;
}

struct Pixel* image_pixel_producer(void) {
    static uint8_t i = 0;
    static struct Pixel* current_pixel = NULL;

    if (i == 0 || i >= current_x_scale) {
        current_pixel = next_pixel(current_image);
    }

    if (i >= current_x_scale) {
        i = 0;
    }

    ++i;
    return current_pixel;
}

static void internal_fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, struct Pixel*(*pixel_producer)(void)) {
    for (uint16_t j = y; j < modeInfoBlockPtr->YResolution && j < y + height; ++j) {
        uint8_t* ccPtr = modeInfoBlockPtr->PhysBasePtr + j * modeInfoBlockPtr->BytesPerScanLine + x * 3;

        for (uint16_t i = x; i < modeInfoBlockPtr->XResolution && i < x + width; ++i) {
            // Little endian order, so MSB goes last
            struct Pixel* pixel = (*pixel_producer)();
            *ccPtr++ = pixel->b;
            *ccPtr++ = pixel->g;
            *ccPtr++ = pixel->r;
        }
    }
}

void fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b) {
    constant_pixel.r = r;
    constant_pixel.g = g;
    constant_pixel.b = b;
    internal_fill(x, y, width, height, &constant_pixel_producer);
}

void draw_pbm_image(struct PbmImage* image, uint16_t x, uint16_t y, uint8_t x_scale) {
    current_image = image;
    current_x_scale = x_scale;
    internal_fill(x, y, image->width * x_scale, image->height, &image_pixel_producer);
}

void replace_color(
    uint8_t r, uint8_t g, uint8_t b, uint8_t new_r, uint8_t new_g, uint8_t new_b,
    uint16_t x, uint16_t y, uint16_t width, uint16_t height
) {
    for (uint16_t j = y; j < modeInfoBlockPtr->YResolution && j < y + height; ++j) {
        uint8_t* ccPtr = modeInfoBlockPtr->PhysBasePtr + j * modeInfoBlockPtr->BytesPerScanLine + x * 3;

        for (uint16_t i = x; i < modeInfoBlockPtr->XResolution && i < x + width; ++i) {
            if (*ccPtr == b && *(ccPtr + 1) == g && *(ccPtr + 2) == r) {
                *ccPtr = new_b;
                *(ccPtr + 1) = new_g;
                *(ccPtr + 2) = new_r;
            }

            ccPtr += 3;
        }
    }
}

#pragma once

#include <stdint.h>
#include "pbm_decoder.h"

/*
 * Fills a rectangle with the specified color, whose left-upper vertex is at (x, y).
 */
void fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b);

/*
 * Draws the specified PBM image on the screen, starting at (x, y).
 * x_scale represents how many times a pixel will be repeated horizontally,
 * effectively stretching the image. It should be at least one.
 */
void draw_pbm_image(struct PbmImage* image, uint16_t x, uint16_t y, uint8_t x_scale);

/*
 * Replaces the specified color with another color, inside a rectangle
 * whose left-upper vertex is at (x, y).
 */
void replace_color(
    uint8_t r, uint8_t g, uint8_t b, uint8_t new_r, uint8_t new_g, uint8_t new_b,
    uint16_t x, uint16_t y, uint16_t width, uint16_t height
);

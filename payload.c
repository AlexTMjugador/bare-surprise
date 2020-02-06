#include "vbe.h"
#include "drawing.h"
#include "rle.h"
#include "baselib.h"
#include "interrupts.h"
#include "assets/build/assets.h"

#define TICKS_INTERVAL 33 // 16.5 ms = 60.61 Hz (FPS for our purposes)

static uint16_t remaining_ticks = TICKS_INTERVAL;
static uint8_t fade_cc = 0;

static struct PbmImage balloons_image;
static struct PbmImage happy_text_image;
static struct PbmImage birthday_text_image;
static struct PbmImage smile_image;
static struct PbmPalette image_palette;

// 64 KiB maximum size for these buffers.
// The first address is 64 Ki positions below ISRs
static void* balloons_decompress_buf = (void*) 0x6FEF0;
#define BALLOONS_DECOMPRESS_BUF_SIZE 32768
static void* happy_text_decompress_buf = (void*) 0x77EF0;
#define HAPPY_TEXT_DECOMPRESS_BUF_SIZE 4096
static void* birthday_text_decompress_buf = (void*) 0x78EF0;
#define BIRTHDAY_TEXT_DECOMPRESS_BUF_SIZE 4096
static void* smile_decompress_buf = (void*) 0x79EF0;
#define SMILE_DECOMPRESS_BUF_SIZE 512

static void initial_fade(void);
static void happy_text_fade(void);
static void birthday_text_fade(void);
static void random_balloons_color(void);

/*
 * Decompresses the specified RLE data to the given buffer, and decodes it as a PBM image.
 * If not successful, this function draws error color codes and never returns.
 */
static void decompress_and_decode_pbm(void* data, size_t size, void* buf, size_t buf_size, struct PbmImage* image);

/**
 * Entry point of the application. The bootloader will jump to the first instruction
 * of this function, at 0x8000.
 */
void start(void) {
	// Load images
	decompress_and_decode_pbm(
		balloons_pbm_stripped_rle, balloons_pbm_stripped_rle_len,
		balloons_decompress_buf, BALLOONS_DECOMPRESS_BUF_SIZE, &balloons_image
	);
	decompress_and_decode_pbm(
		happy_text_pbm_stripped_rle, happy_text_pbm_stripped_rle_len,
		happy_text_decompress_buf, HAPPY_TEXT_DECOMPRESS_BUF_SIZE, &happy_text_image
	);
	decompress_and_decode_pbm(
		birthday_text_pbm_stripped_rle, birthday_text_pbm_stripped_rle_len,
		birthday_text_decompress_buf, BIRTHDAY_TEXT_DECOMPRESS_BUF_SIZE, &birthday_text_image
	);
	decompress_and_decode_pbm(
		smile_pbm_stripped_rle, smile_pbm_stripped_rle_len,
		smile_decompress_buf, SMILE_DECOMPRESS_BUF_SIZE, &smile_image
	);

	// Make sure everything is black
	fill(0, 0, modeInfoBlockPtr->XResolution, modeInfoBlockPtr->YResolution, 0, 0, 0);

	setup_interrupts(&initial_fade);

	// Wait indefinitely until the next tick
	while (true) {
		halt(false);
	}
}

void decompress_and_decode_pbm(void* data, size_t size, void* buf, size_t buf_size, struct PbmImage* image) {
	image->width = 0;
	image->palette = &image_palette;

	size_t rle_decompressed_data_size = decompress(data, size, buf, buf_size);
	if (rle_decompressed_data_size == 0) {
		fill(0, 0, modeInfoBlockPtr->XResolution, modeInfoBlockPtr->YResolution, 255, 255, 0);
		halt(true);
	}

	decode_pbm(buf, rle_decompressed_data_size, image);
	if (image->width == 0) {
		fill(0, 0, modeInfoBlockPtr->XResolution, modeInfoBlockPtr->YResolution, 255, 127, 0);
		halt(true);
	}
}

void initial_fade(void) {
	if (--remaining_ticks == 0) {
		uint8_t old_fade_cc = fade_cc;
		fade_cc += 3; // So fade lasts 1.402 s at 60 FPS

		// Replace previous fading color
		replace_color(
			old_fade_cc, old_fade_cc, old_fade_cc,
			fade_cc, fade_cc, fade_cc,
			0, 0, modeInfoBlockPtr->XResolution, modeInfoBlockPtr->YResolution
		);

		if (fade_cc == 3) {
			// Draw background image
			image_palette.low_r = 0;
			image_palette.low_g = 0;
			image_palette.low_b = 0;
			image_palette.high_r = fade_cc;
			image_palette.high_g = fade_cc;
			image_palette.high_b = fade_cc;

			draw_pbm_image(
				&balloons_image,
				modeInfoBlockPtr->XResolution / 2 - balloons_image.width,
				modeInfoBlockPtr->YResolution / 2 - balloons_image.height / 2,
				2
			);
		}

		// Proceed to the next phase
		if (fade_cc == 255) {
			fade_cc = 252;

			image_palette.low_r = fade_cc;
			image_palette.low_g = fade_cc;
			image_palette.low_b = fade_cc;
			image_palette.high_r = 255;
			image_palette.high_g = 255;
			image_palette.high_b = 255;

			draw_pbm_image(
				&happy_text_image,
				modeInfoBlockPtr->XResolution / 2 - 305,
				modeInfoBlockPtr->YResolution / 2 - 228,
				2
			);

			setup_interrupts(&happy_text_fade);
		}

		remaining_ticks = TICKS_INTERVAL;
	}
}

static void happy_text_fade(void) {
	if (--remaining_ticks == 0) {
		uint8_t old_fade_cc = fade_cc;
		fade_cc -= 3;

		// Replace previous fading color
		replace_color(
			old_fade_cc, old_fade_cc, old_fade_cc,
			fade_cc, fade_cc, fade_cc,
			modeInfoBlockPtr->XResolution / 2 - 305, modeInfoBlockPtr->YResolution / 2 - 228,
			happy_text_image.width * 2, happy_text_image.height
		);

		if (fade_cc == 0) {
			fade_cc = 252;

			setup_interrupts(&birthday_text_fade);

			remaining_ticks = 3000; // 1.5 s for fade start
		} else {
			remaining_ticks = TICKS_INTERVAL;
		}
	}
}

void birthday_text_fade(void) {
	if (--remaining_ticks == 0) {
		uint8_t old_fade_cc = fade_cc;
		fade_cc -= 3;

		// First tick, draw image
		if (old_fade_cc == 252) {
			draw_pbm_image(
				&birthday_text_image,
				modeInfoBlockPtr->XResolution / 2 - 30,
				modeInfoBlockPtr->YResolution / 2 + 175,
				2
			);
		}

		// Replace previous fading color
		replace_color(
			old_fade_cc, old_fade_cc, old_fade_cc,
			fade_cc, fade_cc, fade_cc,
			modeInfoBlockPtr->XResolution / 2 - 30, modeInfoBlockPtr->YResolution / 2 + 175,
			birthday_text_image.width * 2, birthday_text_image.height
		);

		if (fade_cc == 0) {
			setup_interrupts(&random_balloons_color);
		}

		remaining_ticks = TICKS_INTERVAL;
	}
}

void random_balloons_color(void) {
	static uint8_t previous_r = 0;
	static uint8_t previous_g = 0;
	static uint8_t previous_b = 0;

	static bool smile_not_drawn = true;

	if (--remaining_ticks == 0) {
		uint8_t new_r = (uint8_t) (rand() % 200);
		uint8_t new_g = (uint8_t) (rand() % 200);
		uint8_t new_b = (uint8_t) (rand() % 200);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 - 154, modeInfoBlockPtr->YResolution / 2 - 240,
			330, 301
		);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 - 176, modeInfoBlockPtr->YResolution / 2 - 92,
			22, 111
		);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 + 31, modeInfoBlockPtr->YResolution / 2 + 61,
			72, 25
		);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 - 42, modeInfoBlockPtr->YResolution / 2 + 98,
			56, 22
		);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 + 14, modeInfoBlockPtr->YResolution / 2 + 88,
			15, 17
		);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 + 28, modeInfoBlockPtr->YResolution / 2 + 82,
			4, 6
		);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 - 48, modeInfoBlockPtr->YResolution / 2 + 103,
			6, 5
		);

		replace_color(
			previous_r, previous_g, previous_b,
			new_r, new_g, new_b,
			modeInfoBlockPtr->XResolution / 2 - 52, modeInfoBlockPtr->YResolution / 2 + 98,
			4, 4
		);

		if (rand() % 60 == 3 && smile_not_drawn) {
			image_palette.low_r = 0;
			image_palette.low_g = 0;
			image_palette.low_b = 0;
			image_palette.high_r = 255;
			image_palette.high_g = 255;
			image_palette.high_b = 255;

			draw_pbm_image(
				&smile_image,
				modeInfoBlockPtr->XResolution / 2 - 246,
				modeInfoBlockPtr->YResolution / 2 + 126,
				2
			);

			smile_not_drawn = false;
		}

		previous_r = new_r;
		previous_g = new_g;
		previous_b = new_b;

		remaining_ticks = (uint16_t) (rand() % 600) + 400; // 0.5 seconds maximum, 0.2 seconds minimum
	}
}

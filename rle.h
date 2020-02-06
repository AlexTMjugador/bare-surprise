#pragma once

#include <stdint.h>
#include <stddef.h>

/*
 * Decompresses the given data, compressed with RLE.
 * Returns the size of the decompressed data, which will
 * be stored at the buf buffer. If the decompressed data
 * is bigger than the buffer size, the excess bytes will be
 * discarded. A return value of 0 indicates error.
 */
size_t decompress(void* data, size_t size, void* buf, size_t buf_size);

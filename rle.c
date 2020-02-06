#include "rle.h"
#include "drawing.h"

size_t decompress(void* data, size_t size, void* buf, size_t buf_size) {
    uint8_t* data_ptr = (uint8_t*) data;
    uint8_t* decode_buf_ptr = (uint8_t*) buf;

    uint32_t decoded_data_size = 0;
    uint16_t previous_byte = 0xFFFF;

    while (size-- > 0 && decoded_data_size < buf_size) {
        *decode_buf_ptr++ = *data_ptr;
        ++decoded_data_size;

        if (*data_ptr != previous_byte) {
            previous_byte = *data_ptr;
            ++data_ptr;
        } else if (size > 0) {
            uint8_t run_length = *(data_ptr + 1);
            --size;

            while (run_length > 0 && decoded_data_size < buf_size) {
                *decode_buf_ptr++ = *data_ptr;
                --run_length; ++decoded_data_size;
            }

            data_ptr += 2;
            previous_byte = 0xFFFF;
        }
    }

    return decoded_data_size;
}

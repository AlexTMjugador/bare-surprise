#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Halts the execution of instructions until an interrupt is received. If forever is true, then interrupts
 * will be disabled, effectively freezing instruction execution forever.
 */
void halt(bool forever);

/*
 * Writes a byte value to an I/O port.
 */
void outb(uint16_t port, uint8_t value);

/*
 * Delays execution for the specified number of microseconds, approximately, depending
 * on the underlying 0x80 I/O port characteristics. This function is only suitable for
 * low-level I/O synchronization; use the PIT for actual time measurement purposes.
 */
void approximate_udelay(uint16_t usecs);

/*
 * Converts an unsigned integer string to a unsigned integer. It works in a similar
 * manner to the standard C function atoi, but it doesn't allow whitespace or sign
 * symbols in the beginning, and limits the characters read at most to size. Also,
 * it advances the caller pointer by as many positions as characters consumed.
 * Returns 0 if the string couldn't be converted to an integer.
 */
unsigned int strtoui(uint8_t** s, size_t size);

/*
 * Checks whether c is a whitespace character. It works just like isspace standard
 * C library function, assuming a C locale.
 */
bool isspace(char c);

/*
 * Generates a random integer between 0 and 2^32 - 1, inclusive.
 */
uint32_t rand(void);

#include "baselib.h"
#include "interrupts.h"

static bool prng_seeded = false;
static uint64_t prng_seed;

void halt(bool forever) {
	if (forever) {
		cli();
	}

	hlt: __asm__ volatile("HLT"); goto hlt;
}

// Function based on
// https://github.com/torvalds/linux/blob/94f2630b18975bb56eee5d1a36371db967643479/arch/x86/boot/boot.h#L38
inline void outb(uint16_t port, uint8_t value) {
	__asm__ volatile("OUTB %0, %1" :: "dN"(port), "a"(value));
}

void approximate_udelay(uint16_t usecs) {
	while (usecs--) {
		// 0x80 port is used for POST codes,
		// and as such it generates delay without
		// having bad side effects
		outb(0x80, 0);
	}
}

// Inspired by musl atoi implementation: http://git.musl-libc.org/cgit/musl/tree/src/stdlib/atoi.c
unsigned int strtoui(uint8_t** s, size_t size) {
	unsigned int n = 0;

	while (**s >= '0' && **s <= '9' && size > 0) {
		n = 10 * n + (**s - '0');
		++*s; --size;
	}

	return n;
}

bool isspace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

uint32_t rand(void) {
	if (!prng_seeded) {
		// Get pseudorandom bits by using data left over by the EBDA,
		// which should be pretty unique
		prng_seed = 12229904691154926667UL ^ (*(uint64_t*) 0x046C) ^ (*(uint64_t*) 0x0497) ^ (*(uint64_t*) 0x0410);

		prng_seeded = true;
	}

	// From https://git.musl-libc.org/cgit/musl/tree/src/prng/rand.c
	prng_seed = 6364136223846793005ULL * prng_seed + 1;
	return prng_seed >> 33;
}

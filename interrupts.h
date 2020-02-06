#pragma once

#include <stdint.h>

struct interrupt_descriptor {
	uint16_t offset_low;		// 0-15
	uint16_t segment_selector;	// Code descriptor in GDT
	uint8_t zero;				// Always 0
	uint8_t type_attributes;
	uint16_t offset_high;		// 16-31
} __attribute__((packed));

_Static_assert(sizeof(struct interrupt_descriptor) == 8);

struct interrupt_descriptor_table {
	uint16_t length;	// Length of all interrupt descriptors (256 bytes minimum)
	uint32_t base_addr;	// Address of first interrupt descriptor
} __attribute__((packed));

_Static_assert(sizeof(struct interrupt_descriptor_table) == 6, "The IDT must be 6 bytes long");

struct interrupt_frame {
    uint16_t ip;
    uint16_t cs;
    uint16_t flags;
    uint16_t sp;
    uint16_t ss;
} __attribute__((packed));

/**
 * Configures the Interrupt Descriptor Table and the Programmable Interrupt Controller
 * in order for the CPU to handle interrupts properly. Afterwards, it enables interrupts.
 * The specified tick function will be executed whenever 0.5 ms pass (actually, 499.943258 us).
 */
void setup_interrupts(void (*tickFunction) (void));

/**
 * Enables hardware and software interrupts. The IDT and PIC should be configured
 * previously.
 */
void sti(void);

/**
 * Disables hardware and software interupts.
 */
void cli(void);

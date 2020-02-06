#include <stdbool.h>
#include <stddef.h>

#include "interrupts.h"
#include "drawing.h"
#include "vbe.h"
#include "baselib.h"

#define MASTER_PIC_COMMAND 0x20
#define MASTER_PIC_DATA 0x21
#define SLAVE_PIC_COMMAND 0xA0
#define SLAVE_PIC_DATA 0xA1
#define PIT_COMMAND 0x43
#define PIT_DATA 0x40

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x03 // 8086/88 (MCS-80/85) mode, auto EOI

// The following two variables provide a much more expressive way to calculate IDT start,
// but are not strictly standards conforming.
//static const uint32_t idt_size = sizeof(struct interrupt_descriptor) * idt_entries + sizeof(struct interrupt_descriptor_table);
//static const uint32_t idt_start = 0x7FFFF - idt_size + 1;

static const uint16_t idt_entries = 33; // 33 IRQ. 32 mandatory by Intel in protected mode for processor exceptions
static const uint32_t idt_start = 0x7FEF0;

static bool interruptsConfigured = false;

static void (*tickFunc)(void) = NULL;

__attribute__((interrupt)) static void cpu_exception_isr(struct interrupt_frame*, unsigned int);
__attribute__((interrupt)) static void pit_isr(struct interrupt_frame*);

void setup_interrupts(void (*tickFunction) (void)) {
	tickFunc = tickFunction;

	if (!interruptsConfigured) {
		struct interrupt_descriptor_table* idt = (struct interrupt_descriptor_table*) idt_start;

		// Get the address to the first interrupt descriptor
		struct interrupt_descriptor* first_desc = (struct interrupt_descriptor*) (idt + 1);

		// Populate the IDT
		idt->length = sizeof(struct interrupt_descriptor) * idt_entries - 1;
		idt->base_addr = (uint32_t) first_desc;

		struct interrupt_descriptor* current_desc = first_desc;

		// Populate the entries in the IDT.
		// The first 32 descriptors handle CPU exceptions
		for (unsigned int i = 0; i < 0x20; ++i) {
			current_desc->offset_low = ((uint32_t) &cpu_exception_isr) & 0xFFFF;
			current_desc->offset_high = (((uint32_t) &cpu_exception_isr) & 0xFFFF0000) >> 16;
			current_desc->segment_selector = 0x08; // The bootloader places us here
			current_desc->zero = 0;
			// 32-bit interrupt gate, present, privilege level 0, storage segment 0
			current_desc->type_attributes = 0x8E;
			++current_desc;
		}

		// The 33rd descriptor is somewhat special, because we want
		// to handle PIT interrupts which go to it
		current_desc->offset_low = ((uint32_t) &pit_isr) & 0xFFFF;
		current_desc->offset_high = (((uint32_t) &pit_isr) & 0xFFFF0000) >> 16;
		current_desc->segment_selector = 0x08;
		current_desc->zero = 0;
		current_desc->type_attributes = 0x8E;

		// Load the IDT
		__asm__ volatile("LIDT %0" : : "m"(*idt));

		// Now program the cascaded PICs so interrupt
		// offsets do not overlap with CPU exceptions.
		// This is what the Linux kernel does
		outb(MASTER_PIC_COMMAND, ICW1_INIT | ICW1_ICW4);
		approximate_udelay(2);
		outb(SLAVE_PIC_COMMAND, ICW1_INIT | ICW1_ICW4);
		approximate_udelay(2);
		outb(MASTER_PIC_DATA, 0x20); // Start hardware interrupts at 0x20
		approximate_udelay(2);
		outb(SLAVE_PIC_DATA, 0x28); // Start hardware interrupts at 0x28
		approximate_udelay(2);
		outb(MASTER_PIC_DATA, 4); // Slave PIC at IRQ 2 (100)
		approximate_udelay(2);
		outb(SLAVE_PIC_DATA, 2); // Tell slave PIC it is a slave
		approximate_udelay(2);
		outb(MASTER_PIC_DATA, ICW4_8086);
		approximate_udelay(2);
		outb(SLAVE_PIC_DATA, ICW4_8086);
		approximate_udelay(2);

		// Configure the master PIC with a mask of the interrupts we actually handle
		// (only PIT for now, ignore slave)
		outb(MASTER_PIC_DATA, 0xFE);
		approximate_udelay(2);

		outb(PIT_COMMAND, 0x34); // LO/HI access mode, rate generator, channel 0
		// We want to use a reload value of 419, so frequency is divided by 419.
		// The resulting period is very close to 0.5 ms (499.943258 us), so for
		// practical purposes we can tell a tick occurs each half of millisecond.
		// We send the first low byte, 0xA3, and then the high byte, 0x01.
		// After the high byte is sent, the PIT starts ticking.
		// After one second, the clock would be off by 0.11 ms, which means that,
		// after one hour, it would be off by 408.5 ms. Not great, but enough
		// for our purposes
		outb(PIT_DATA, 0xA3);
		outb(PIT_DATA, 0x01);

		interruptsConfigured = true;

		// Enable interrupts
		sti();
	}
}

__attribute__((interrupt)) void cpu_exception_isr(
	__attribute__ ((unused)) struct interrupt_frame* frame, unsigned int error_code
) {
	fill(0, 0, modeInfoBlockPtr->XResolution, modeInfoBlockPtr->YResolution, 255, 0, error_code);
	halt(true);
}

__attribute__((interrupt)) void pit_isr(__attribute__ ((unused)) struct interrupt_frame* frame) {
	// Call the time tick function
	if (tickFunc != NULL) {
		(*tickFunc)();
	}
}

inline void sti(void) {
	__asm__ volatile("STI");
}

inline void cli(void) {
	__asm__ volatile("CLI");
}

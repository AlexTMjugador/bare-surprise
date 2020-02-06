#pragma once

#include <stdint.h>

struct ModeInfoBlock {
	uint16_t ModeAttributes;		// + 0

	uint8_t WinAAttributes;			// + 2
	uint8_t WinBAttributes;			// + 3
	uint16_t WinGranularity;		// + 4
	uint16_t WinSize;				// + 6
	uint16_t WinASegment;			// + 8
	uint16_t WinBSegment;			// + 10
	void* WinFuncPtr;				// + 12. Pointer to window function

	uint16_t BytesPerScanLine;		// + 16

	// VBE 1.2 onwards
	uint16_t XResolution;			// + 18. Horizontal resolution in pixels or characters
	uint16_t YResolution;			// + 20. Vertical resolution in pixels or characters
	uint8_t XCharSize;				// + 22. Character cell width in pixels
	uint8_t YCharSize;				// + 23. Character cell height in pixels
	uint8_t NumberOfPlanes;			// + 24. Number of memory planes
	uint8_t BitsPerPixel;			// + 25
	uint8_t NumberOfBanks;			// + 26
	uint8_t MemoryModel;			// + 27
	uint8_t BankSize;				// + 28. In KiB
	uint8_t NumberOfImagePages;		// + 29
	uint8_t Reserved1;				// + 30

	// Direct color fields
	uint8_t RedMaskSize;			// + 31. In bits
	uint8_t RedFieldPosition;		// + 32. Bit position of LSB of rmask
	uint8_t GreenMaskSize;			// + 33. In bits
	uint8_t GreenFieldPosition;		// + 34. Bit position of LSB of mask
	uint8_t BlueMaskSize;			// + 35. In bits
	uint8_t BlueFieldPosition;		// + 36. Bit position of LSB of mask
	uint8_t RsvdMaskSize;			// + 37. In bits
	uint8_t RsvdFieldPosition;		// + 38. Bit position of LSB of mask
	uint8_t DirectColorModeInfo; 	// + 39.

	// VBE 2.0 onwards
	uint8_t* PhysBasePtr;			// Physical address for flat memory buffer
	uint8_t* OffScreenMemOffset;	// Pointer to start of ofscreen memory
	uint16_t OffScreenMemSize;		// Amount of offscreen memory in KiB

	uint8_t Reserved2[206];
};

_Static_assert(sizeof(struct ModeInfoBlock) == 256, "ModeInfoBlock size must equal 256 bytes");
_Static_assert(sizeof(uint32_t) == sizeof(uint8_t*), "A uint8_t pointer must be 4 bytes long");
_Static_assert(sizeof(uint32_t) == sizeof(void*), "A void pointer must be 4 bytes long");

// A pointer to the VBE 2.0 ModeInfoBlock structure made available by the bootloader.
static const struct ModeInfoBlock* modeInfoBlockPtr = (struct ModeInfoBlock*) 0x0700;

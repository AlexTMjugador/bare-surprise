# bare-surprise
A toy bootloader, operating system and graphical application made from scratch for a birthday surprise, whose total size is less than 12 KiB. That is smaller than a single JPEG image, and 10 times less than the amount of RAM found in a SNES.

## Overview
This project aims to build from scratch the minimum things necessary to get almost any x86 PC up and running without an OS, showing a small birthday congratulation (referred to in the code as _payload_), in the less disk space possible. The congratulation itself, however, is easily replaceable, so this project can serve as a basis for other similar, simple payloads.

To achieve the project goals, a through understanding of the x86 CPU architecture and the IBM PC inner workings is needed. The following paragraphs are just meant to allow the reader to grasp the flow of events that happens in the end result.

The first stage bootloader must be coded in x86 assembly, because it needs direct access to the CPU registers and INT instruction, and memory access registers are not yet configured (languages like C, even while they compile to machine code, can't run because the stack pointer register is not initialized). So, unsurprisingly, the first stage bootloader of this project, contained in the MBR (so at most is 512 - 2 = 510 bytes long) and loaded by the BIOS, sets up the stack and memory segment registers. Apart from that, it also checks whether VESA Bios Extensions 2.0 are supported, because they are needed for the payload, reads the second stage bootloader and payload from the next sectors of the disk, and jumps to the second stage bootloader.

The second stage bootloader, which is 512 bytes long, chooses the first suitable video mode for the payload, using VBE 2.0 calls. If successful, it loads a Global Descriptor Table, which contains information for the CPU on which regions of memory have which permissions, and is needed for switching to 32-bit protected mode (for bbackward compatibility, all x86 CPUs start execution in 16-bit real mode, identical to the Intel 8086 used in the first IBM PC design). This mode is used to relieve constraints on memory segmentation, offering instead a flat memory model that is easier to work with. But, most importantly, it is supported by most C compilers. Once the protected mode switch is completed, interrupts are disabled (they are enabled later in protected mode, when ISRs are set up), the A20 line is enabled in a best effort, so all memory lines can be used, and the 11 KiB C11 payload takes the control.

The current payload configures the Interrupt Descriptor Table, the standard IBM PC interrupt controller and Programmable Interval Timer (PIT), so that its interrupt service routine executes a tick function each 500 µs, used for updating the screen. To achieve this result, there are implementations for an incredibly tiny subset of the standard C library functions, differing with it in some documented aspects. There are also functions for:

- _Decoding Portable Bit Map (PBM) images_. Designed primarily as an intermediary format, PBM encodes monochrome images in a extremely simple to parse way. Free and open source tools like FFmpeg and GIMP can read and generate images in this format. Note that this implementation does not support comments, so they should be stripped from the file beforehand.
- _Run length encoding (RLE) decompression_. RLE techniques are extremely fast and simple to implement, while providing a > 2:1 compression ratio for the PBM images used in this project. The code for more sophisticated compression schemes would take so much instructions that the space efficiency advantage they give would be neutralized.

## Building

For building the project, it is important to know that it is composed by several subprojects, each with its own Makefile:

- Main project: the root folder project contains the payload and bootloader.
- assets: the data that is intended to be included in the payload raw is put here. For now, they are PBM images. All these files are combined in an automatically generated C header file, `assets.h`, that is part of the resulting payload. That header allows accesing files at runtime like arrays.
- util: this auxiliary project contains the RLE compressor that will generate data suitable for decompressing with the provided decompressor (RLE is not a single standarized algorithm, so interoperability is a concern).

Dividing the project in subprojects eases creation and maintenance of Makefile scripts: the main project uses the other ones. Therefore, for generating the raw disk image suitable for writing on a hard disk, `build/disk.img`, it suffices with running Make in the root directory.

```console
you@computer:~/bare-surprise$ make
```

The following tools are required:
- GNU Make 4.2.1 or later (older or non-GNU variants will probably work, but were not tested).
- GCC 9 or later for x86 (older versions will probably work, but were not tested). **This project makes use of GCC specific extensions and it won't probably compile as-is with other compilers**.
- binutils 2.33.90 or later. This normally comes with GCC, and includes the GNU linker, for which there are specific linker scripts in this project.
- bbe 0.2.2 (for stripping the comment generated by GIMP from PBM files).
- Netwide Assembler (NASM) 2.14.02 or later.

For testing, there are targets that launch `qemu-system-x86` with the resulting disk image, but otherwise QEMU is not necessary. Also, an Unix environment is assumed, with a POSIX shell at `/bin/sh`, and `sed` and `xxd` available.

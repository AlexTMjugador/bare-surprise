; Intel 80386 compliance
CPU 386
; IBM PC BIOS loads this code at memory address
; 0x7C00. No guarantee about on which segment it is
ORG 0x7C00

; When this bootloader hands over execution to C code,
; the following variables are guaranteed to be available
; in the specified memory addresses:
; 0x0500 - 0x06FF: VbeInfoBlock (512 bytes)
; 0x0700 - 0x07FF: ModeInfoBlock (256 bytes)
; 0x0800 - 0x080B: temporary itoa buffers (12 bytes)
; ??? - 0x7BFF: stack (grows backwards)
; 0x7C00 - 0x7FFF: bootloader code (1 KiB)
; 0x8000 - 0x7FFFF: C code (480 KiB maximum)
; When C code executes, interrupts are disabled. It is responsible
; for setting up a IDT and enabling them, if needed.

; ------------------
; Master Boot Record
;   (first stage)
; ------------------

mbr:
	; Initialize memory segment and stack configuration
	XOR ax, ax
	MOV ds, ax
	MOV es, ax
	MOV ss, ax
	MOV bp, ax
	MOV sp, 0x7BFF

	; Make sure direction flag is cleared
	CLD

	; Set up screen
	CALL clear_screen
	CALL hide_cursor

	; Print welcome message
	MOV si, welcome
	CALL puts

	; Check if VESA BIOS extensions 2.0 are supported.
	; First, set VBE 2.0 signature of VbeInfoBlock
	MOV si, vbe_signature
	MOV di, 0x0500
	CALL strcpy

	; Now call "Function 00h - Return VBE Controller Information"
	MOV ax, 0x4F00
	MOV di, 0x0500
	INT 0x10
	CMP ax, 0x004F
	JNZ print_vesa_error

	; VESA is supported. Check for VBE 2.0 support
	; by reading VbeVersion
	CMP byte [0x0505], 2
	JL print_vesa_error

	; Print version information and OEM string
	MOV si, supported_str_start
	CALL puts
	MOV al, byte [0x0505]
	ADD al, 48 ; 48 is the ASCII code for 0
	CALL putchar
	MOV al, '.'
	CALL putchar
	MOV al, byte [0x0504]
	ADD al, 48
	CALL putchar
	MOV si, supported_str_mid
	CALL puts
	MOV si, word [0x0506]
	MOV ds, word [0x0508]
	CALL puts
	MOV al, `\r`
	CALL putchar
	MOV al, `\n`
	CALL putchar

	; Load the second stage bootloader
	; from the second sector at 0x7E00, and the
	; C code payload.
	; It is assumed that a track has at least
	; two sectors in it
	MOV ah, 0x02
	MOV al, 23 ; One sector for second stage + 22 sectors for C payload (11 KiB)
	MOV ch, 0 ; First cylinder (track)
	MOV dh, 0 ; First head
	MOV cl, 2 ; Second sector
	; DL has the boot disk identifier provided by the BIOS
	MOV bx, 0x7E00
	INT 0x13
	JC print_io_error

	; Jump straight to the second stage bootloader!
	JMP second_stage

; ---------------
; Basic functions
; ---------------

; Prints the current VBE mode information structure resolution information.
; It is assumed that the current VBE mode information structure is
; loaded at 0x0700.
print_mode:
	PUSH si

	MOV dx, word [0x0712]
	CALL itoa
	MOV si, 0x0806
	CALL puts
	MOV al, 'x'
	CALL putchar
	MOV dx, word [0x0714]
	CALL itoa
	MOV si, 0x0806
	CALL puts
	MOV al, 'x'
	CALL putchar
	MOV dl, byte [0x0719]
	MOV dh, 0
	CALL itoa
	MOV si, 0x0806
	CALL puts

	POP si
	RET

; Prints a VBE "not supported" error message. This
; procedure never returns.
print_vesa_error:
	MOV si, not_supported_str
	CALL puts
	JMP freeze

print_io_error:
	MOV si, io_error_str
	CALL puts
	JMP freeze

; Prints the null-terminated string pointed by SI
; using IBM PC BIOS video services.
puts:
	.loop:
		LODSB
		TEST al, al
		JZ .return
		CALL putchar
		JMP .loop

	.return:
		RET

; Prints a single character stored in AL, using IBM PC
; BIOS video services.
putchar:
	MOV ah, 0x0E
	INT 0x10
	RET

; Clears the screen.
clear_screen:
	; Get current video mode
	MOV ah, 0x0F
	INT 0x10
	; Set the video mode again, clearing video memory
	AND ax, 0x00FF
	INT 0x10
	RET

; Hides the text mode cursor.
hide_cursor:
	MOV ah, 0x01
	MOV cx, 0x2020
	INT 0x10
	RET

; Copies a null-terminated string from the segment offset
; indexes pointed to by SI and DI.
strcpy:
	.copy_char:
		MOVSB
		MOV ax, [DS:SI]
		TEST ax, ax
		JNZ .copy_char

	RET

; Disables interrupts and hints the CPU to enter a low
; power idle mode forever.
freeze:
	CLI
	HLT
	JMP freeze

; Converts an unsigned 16 bit integer in DX to a null-terminated
; string that is at most 6 bytes long and starts at 0x0806.
; Integers bigger than 2550 will cause problems.
itoa:
	PUSH di
	PUSH ax
	PUSH dx
	PUSH cx

	; Set up registers
	MOV di, 0x0800

	; Zero out used memory
	XOR ax, ax
	MOV cx, 12
	.cleanup:
		STOSB
		LOOP .cleanup

	; Continue setting up registers
	MOV di, 0x0800
	MOV ax, dx
	MOV dx, 10

	.store_digits:
		; The 16 bit integer to divide is in AX
		DIV dl ; Result in AL, modulus in AH
		XCHG ah, al ; Result in AH, modulus in AL
		STOSB ; Store modulus in DI
		CMP ah, 10
		JNB .shift_and_continue ; If result >= 10, there are more digits
		CMP ah, 0
		JE .convert_to_string ; If result == 0, we divided a single digit number, so result is not important
		; Result > 0 and < 9 here
		XCHG ah, al ; Result in AL, modulus in AH
		STOSB ; Store result in DI
		JMP .convert_to_string

	.shift_and_continue:
		; Store result in AH to AL, clear AH and continue
		MOV al, ah
		MOV ah, 0
		JMP .store_digits

	.convert_to_string:
		MOV byte [di], 0 ; Null terminator
		DEC di ; Move to the last digit
		MOV cx, 0x0806

		.loop:
			MOV al, byte [di] ; Get digit at DI
			ADD al, 48 ; Convert to ASCII
			XCHG di, cx ; Only DI may be used for indirect access
			MOV byte [di], al ; Store at 0x0806 + offset
			XCHG di, cx
			INC cx ; Increment offset of next character
			DEC di ; Move pointer to the previous digit
			CMP di, 0x07FF
			JNE .loop ; End loop when end of digits is reached

	POP cx
	POP dx
	POP ax
	POP di
	RET

; ---------
; Constants
; ---------
welcome: DB `-----------\r\nTBSBL 1.0\r\n-----------\r\n`, 0

supported_str_start: DB 'VBE ', 0
supported_str_mid: DB ' found. OEM: ', 0
not_supported_str: DB '! Error calling VBE, bailing', 0
vbe_signature: DB 'VBE2', 0

io_error_str: DB '! I/O error while reading from boot disk', 0

; MBR signature on the last 2 bytes
TIMES 510 - ($ - $$) DB 0
DB 0x55
DB 0xAA

; -----------------------
; Second stage bootloader
; -----------------------

; Selects the best VBE mode, assuming that VBE controller
; information was loaded on 0x0500, switches to protected
; mode and loads C code payload.
second_stage:
	; Print header string
	XOR ax, ax ; Clear accumulator
	MOV ds, ax
	MOV si, selected_mode
	CALL puts
	MOV al, ' '
	CALL putchar

	MOV si, word [0x050E]
	MOV ds, word [0x050C]

	.loop:
		LODSW
		MOV cx, ax
		CMP ax, 0xFFFF ; End of list?
		JE .bail

		; Call "Return VBE Mode Information"
		MOV ax, 0x4F01
		MOV di, 0x0700
		INT 0x10
		CMP ax, 0x004F
		JNZ print_vesa_error

		; Check for appropriate mode attributes
		MOV ax, word [0x0700]
		AND ax, 0000_0000_1001_1001b
		CMP ax, 0000_0000_1001_1001b ; D0 = mode supported in hardware, D3 = color mode, D4 = graphics mode, D7 = LFB available
		JNE .loop ; Skip if mode attributes do not meet requirements

		; Get X and Y resolution
		MOV ax, word [0x0712]
		MOV dx, word [0x0714]

		; Ignore if resolution is too big
		CMP ax, 2550
		JNBE .loop
		CMP dx, 2550
		JNBE .loop

		; Ignore if resolution is too small
		CMP ax, 640
		JB .loop
		CMP dx, 480
		JB .loop

		; Check for direct color memory model
		CMP byte [0x071B], 6
		JNE .loop ; Skip

		; Make sure it has a single plane
		CMP byte [0x0718], 1
		JNE .loop ; Skip

		; Check for 24 bit color
		CMP byte [0x0719], 24
		JNE .loop ; Skip

		; Check for 8:8:8 color mask
		CMP byte [0x071F], 8
		JNE .loop ; Skip
		CMP byte [0x0721], 8
		JNE .loop
		CMP byte [0x0723], 8
		JNE .loop

		; This is the mode we want
		PUSH cx ; Put the mode in the stack
		CALL print_mode
		JMP .set_mode

	.bail:
		MOV si, no_mode
		CALL puts
		JMP freeze

	.set_mode:
		MOV si, video_mode_found
		CALL puts

		; Wait for keystroke
		XOR ah, ah
		INT 0x16

		MOV al, `\r`
		CALL putchar
		MOV al, `\n`
		CALL putchar
		MOV al, `\r`
		CALL putchar
		MOV al, `\n`
		CALL putchar

		; Call "Set VBE Mode"
		MOV ax, 0x4F02
		POP bx ; Get the mode from the stack
		AND bx, 00000001_11111111b ; Discard 7 higher bits
		OR bh, 0100_0000b ; Clear display memory, use linear frame buffer model
		INT 0x10
		CMP ax, 0x004F
		JNZ print_vesa_error

		; Disable interrupts
		CLI

		; Enable A20 line for full access to memory
		CALL enable_a20

		; Load the Global Descriptor Table we need to switch to
		; protected mode. The GDT also allows for a flat memory
		; model of 4 GiB
		XOR ax, ax
		MOV ds, ax
		LGDT [gdt_descriptor]

		; Switch to protected mode
		MOV eax, cr0
		OR eax, 1
		MOV cr0, eax

		; Far jump to 32-bit code (cleans up the pipeline too)
		JMP 0x08:complete_protected_mode_entry

; ---------
; Functions
; ---------

; Function adapted from https://wiki.osdev.org/A20_Line#Testing_the_A20_line
enable_a20:
	MOV ax, 0xFFFF
	MOV ds, ax

	MOV di, 0x0500
	MOV si, 0x0510

	MOV al, byte [es:di]
	PUSH ax

	MOV al, byte [ds:si]
	PUSH ax

	MOV byte [es:di], 0x00
	MOV byte [ds:si], 0xFF

	CMP byte [es:di], 0xFF

	POP ax
	MOV byte [ds:si], al

	POP ax
	MOV byte [es:di], al

	; Do not enable A20 if already enabled (QEMU does this)
	JNE .return

	; BIOS call to enable A20. Not universally supported,
	; but all methods to enable A20 are messy
	MOV ax, 0x2401
	INT 0x15

	.return:
		RET

; ---------
; Constants
; ---------

video_mode_found: DB `\r\n\r\nPress any key to start.`, 0

gdt:
	; Null GDT (unused)
	DQ 0

	; Code GDT
	DW 0xFFFF ; Segment limit (0-15)
	DW 0 ; Base address (0-15)
	DB 0 ; Base address (16-23)
	DB 1001_1010b ; Readable segment, non-conforming, code segment, ring 0, present flag
	DB 1100_1111b ; 32-bit, 4 KiB granularity (4 GiB flat memory)
	DB 0 ; Base address (24-31)

	; Data GDT
	DW 0xFFFF ; Segment limit (0-15)
	DW 0 ; Base address (0-15)
	DB 0 ; Base address (16-23)
	DB 1001_0010b ; R/W segment, downward expand direction, data segment, ring 0
	DB 1100_1111b ; 32-bit, 4 KiB granularity (4 GiB flat memory)
	DB 0 ; Base address (24-31)

gdt_size: EQU $ - gdt

gdt_descriptor:
	DW gdt_size - 1
	DD gdt

; ---------
; Constants
; ---------

selected_mode: DB '  Chosen video mode:', 0
no_mode: DB '! No suitable video mode found, bailing', 0

; ------------------------
; "Third stage" bootloader
; ------------------------

; We execute 32-bit code from now on
BITS 32

; Sets up the current segment descriptor, disables the A20 line
; and jumps straight to C code, previously loaded at 0x8000.
complete_protected_mode_entry:
	; Data segment 0x10 (16)
	MOV ax, 0x10
	MOV ds, ax
	MOV es, ax
	MOV fs, ax
	MOV gs, ax
	MOV ss, ax

	; Jump to C!
	JMP 0x8000

; Pad second sector with zeroes
TIMES 1024 - ($ - $$) DB 0

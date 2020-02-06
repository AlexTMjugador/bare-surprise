BUILD_DIR = build
SUBFOLDERS = assets util

PAYLOAD_FILES = $(shell find -maxdepth 1 -type f -iname '*.c' -o -iname '*.h')
PAYLOAD_CODE_FILES = $(shell find -maxdepth 1 -type f -iname '*.c')
ASSETS_HEADER = assets/build/assets.h

SUBFOLDERS_CLEAN=$(addsuffix clean,$(SUBFOLDERS))

.PHONY: default
default: $(BUILD_DIR)/disk.img

.PHONY: clean
clean: $(SUBFOLDERS_CLEAN) clean_this

.PHONY: %clean
%clean: %
	@$(MAKE) -C '$<' clean

.PHONY: clean_this
clean_this:
	@echo 'RM $(BUILD_DIR)'
	@rm -rf '$(BUILD_DIR)'

.PHONY: test
test: $(BUILD_DIR)/disk.img
	@echo 'QEMU $(BUILD_DIR)/disk.img'
	@qemu-system-i386 -drive file="$(BUILD_DIR)/disk.img",format=raw

$(BUILD_DIR)/bootloader.bin: bootloader.asm $(BUILD_DIR)
	@echo 'NASM $<'
	@nasm -o '$@' '$<'

$(BUILD_DIR)/payload.bin: linker.ld $(PAYLOAD_FILES) $(ASSETS_HEADER) $(BUILD_DIR)
	@echo 'CC $(PAYLOAD_CODE_FILES)'
	@$(CC) -s -std=c11 -march=i386 -mtune=generic -m32 -masm=intel -fno-pie -mgeneral-regs-only \
		-Os -ffreestanding -nostdlib -Wl,--build-id=none,--hash-style=sysv,--gc-sections,--print-map \
		-ffunction-sections -fdata-sections -Tlinker.ld -Wall -Wextra \
		-o '$@' $(PAYLOAD_CODE_FILES)

$(BUILD_DIR)/disk.img: $(BUILD_DIR)/bootloader.bin $(BUILD_DIR)/payload.bin $(BUILD_DIR)
	@echo 'Generating 12 KiB (24 sectors) disk image: $@'
	@cat $(BUILD_DIR)/bootloader.bin $(BUILD_DIR)/payload.bin > '$@' 2>/dev/null
	@dd if=/dev/null of="$(BUILD_DIR)/disk.img" bs=1 count=1 seek=12K 2>/dev/null

.PHONY: $(ASSETS_HEADER)
$(ASSETS_HEADER):
	@echo 'MAKE $@'
	@$(MAKE) --no-print-directory -C assets '$(subst assets/,,$@)'

$(BUILD_DIR):
	@echo 'MKDIR $(BUILD_DIR)'
	@mkdir -p "$(BUILD_DIR)"

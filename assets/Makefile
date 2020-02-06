RLE_COMPRESSOR_DIR = ../util

BUILD_DIR = build

PBM_IMAGES = $(shell find -type f -iname '*.pbm')

.PHONY: all
all: $(BUILD_DIR)/assets.h

.PHONY: clean
clean:
	@echo 'RM $(BUILD_DIR)'
	@rm -rf '$(BUILD_DIR)'
	@echo 'RM *.stripped'
	@rm -f *.stripped

$(BUILD_DIR)/assets.h: $(addsuffix .stripped,$(PBM_IMAGES)) | $(addprefix $(BUILD_DIR)/,$(addsuffix .stripped.rle,$(PBM_IMAGES))) $(BUILD_DIR) generate_assets_header.sh
	@echo 'GENERATE_ASSETS_HEADER $@'
	@$(shell ./generate_assets_header.sh '$(BUILD_DIR)')

$(BUILD_DIR)/./%.rle: $(subst .rle,,$(subst $(BUILD_DIR)/./,,$@)) $(RLE_COMPRESSOR_DIR)/build/rle_compressor $(BUILD_DIR)
	@echo 'RLE $(subst .rle,,$(subst $(BUILD_DIR)/./,,$@))'
	@cat '$(subst .rle,,$(subst $(BUILD_DIR)/./,,$@))' | $< > '$@'

%.stripped: % $(BUILD_DIR)
	@echo 'BBE $<'
	@cat '$<' | bbe -b '/# Created by/:/\n/' -e 'D 1' -o '$@'

.PHONY: $(RLE_COMPRESSOR_DIR)/build/rle_compressor
$(RLE_COMPRESSOR_DIR)/build/rle_compressor:
	@echo 'MAKE $(RLE_COMPRESSOR_DIR)/build/rle_compressor'
	@$(MAKE) --no-print-directory -C '$(RLE_COMPRESSOR_DIR)' build/rle_compressor

$(BUILD_DIR):
	@echo 'MKDIR $(BUILD_DIR)'
	@mkdir -p "$(BUILD_DIR)"

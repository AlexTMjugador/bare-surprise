BUILD_DIR = build

.PHONY: default
default: $(BUILD_DIR)/rle_compressor

.PHONY: clean
clean:
	@echo 'RM $(BUILD_DIR)'
	@rm -rf '$(BUILD_DIR)'

$(BUILD_DIR)/rle_compressor: rle_compressor.c $(BUILD_DIR)
	@echo 'CC $<'
	@$(CC) -o '$@' $<

$(BUILD_DIR):
	@echo 'MKDIR $(BUILD_DIR)'
	@mkdir -p '$(BUILD_DIR)'

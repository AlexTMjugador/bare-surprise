#!/bin/sh

if [ $# -lt 1 ] || [ ! -d "$1" ]; then
    printf 'Syntax: %s [final asset directory]\n' "$(basename "$0")"
    exit 1
fi

# Delete output file if already present
rm -rf "$1"/assets.h 2>/dev/null || true

readonly TMP_ASSET_FILE=$(mktemp --tmpdir assets.h.XXX)

printf '//Automatically generated header file. Do not edit!\n\n#pragma once\n\n#include <stddef.h>\n\n' > "$TMP_ASSET_FILE"

for file in "$1"/*; do
    xxd -i -c32 "$file" | sed 's/unsigned char/uint8_t/;s/unsigned int/size_t/;s/build_//;s/_comment_stripped//' >> "$TMP_ASSET_FILE"
    printf '\n' >> "$TMP_ASSET_FILE"
done

mv "$TMP_ASSET_FILE" "$1"/assets.h
rm -f "$TMP_ASSET_FILE" >/dev/null 2>&1 || true

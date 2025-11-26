#!/bin/bash
set -e

FONT_DIR="assets/fonts"

if ! mkdir -p "$FONT_DIR"; then
    echo "✗ Error: Failed to create font directory: $FONT_DIR"
    exit 1
fi

copy_font() {
    local source="$1"
    local dest="$FONT_DIR/arial.ttf"

    if ! cp "$source" "$dest"; then
        echo "✗ Error: Failed to copy font from $source to $dest"
        return 1
    fi
    if [ ! -f "$dest" ]; then
        echo "✗ Error: Font file was not created at $dest"
        return 1
    fi
    echo "✓ Successfully copied font: $source"
    return 0
}

if [ -f "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf" ]; then
    if copy_font "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf"; then
        echo "✓ Using DejaVu Sans font"
    else
        exit 1
    fi
elif [ -f "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" ]; then
    if copy_font "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"; then
        echo "✓ Using DejaVu Sans font"
    else
        exit 1
    fi
else
    FONT=$(find /usr/share/fonts -name "*.ttf" 2>/dev/null | head -1)
    if [ -n "$FONT" ]; then
        if copy_font "$FONT"; then
            echo "✓ Using system font: $FONT"
        else
            exit 1
        fi
    else
        echo "✗ Error: No TTF font found in /usr/share/fonts"
        echo "Please manually copy a .ttf font to: $FONT_DIR/arial.ttf"
        exit 1
    fi
fi

echo ""
echo "✓ Font setup complete!"
echo "Font location: $FONT_DIR/arial.ttf"

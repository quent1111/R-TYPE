#!/bin/bash
# Setup fonts for R-TYPE menu

FONT_DIR="assets/fonts"
mkdir -p "$FONT_DIR"

# Try to find and copy a suitable font
if [ -f "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf" ]; then
    cp "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf" "$FONT_DIR/arial.ttf"
    echo "✓ Copied DejaVu Sans font"
elif [ -f "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" ]; then
    cp "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" "$FONT_DIR/arial.ttf"
    echo "✓ Copied DejaVu Sans font"
else
    # Try to find any TTF font
    FONT=$(find /usr/share/fonts -name "*.ttf" 2>/dev/null | head -1)
    if [ -n "$FONT" ]; then
        cp "$FONT" "$FONT_DIR/arial.ttf"
        echo "✓ Copied system font: $FONT"
    else
        echo "⚠ Warning: No font found. Please place a TTF font at $FONT_DIR/arial.ttf"
    fi
fi

echo ""
echo "Font setup complete!"
echo "If you have issues, manually copy a .ttf font to: $FONT_DIR/arial.ttf"

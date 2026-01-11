#!/bin/bash
set -e
echo "Compiling the application..."
gcc main_gui.c logic.c -o assignment_gui $(pkg-config --cflags --libs gtk4)
echo "Build complete!"
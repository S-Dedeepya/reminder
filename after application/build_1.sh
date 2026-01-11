#!/bin/bash

echo "Compiling the application..."

# This is the same compile command you've been using
gcc main_gui.c logic.c -o assignment_gui $(pkg-config --cflags --libs gtk4)

echo "Build complete! If there were no errors, the 'assignment_gui' executable is ready."

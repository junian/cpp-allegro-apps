#!/bin/sh
# Build script for macOS and Linux
# Requires: pkg-config, allegro 5

g++ main.cpp -o snake.exe \
    $(pkg-config allegro-5 allegro_main-5 allegro_image-5 allegro_primitives-5 --libs --cflags)

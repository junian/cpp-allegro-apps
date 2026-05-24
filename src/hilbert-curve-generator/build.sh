#!/bin/sh
# Build script for macOS and Linux
# Requires: pkg-config, allegro 5

g++ hilbert.cpp -o hilbert.exe \
    $(pkg-config allegro-5 allegro_main-5 allegro_primitives-5 --libs --cflags)

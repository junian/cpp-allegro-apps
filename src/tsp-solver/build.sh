#!/bin/sh
# Build script for macOS and Linux
# Requires: pkg-config, allegro 5

g++ "Travelling Salesman Problem Solver.cpp" -o tsp-solver \
    $(pkg-config allegro-5 allegro_main-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 --libs --cflags)

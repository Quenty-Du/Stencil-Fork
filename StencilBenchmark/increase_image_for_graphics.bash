#!/bin/bash

((max_pixel_count=20 * 1000000))

width=1
height=1

while (( $width * $height * 100000 < $max_pixel_count )); do
    ./build/samples/Debug/sample_program.exe "$width" "$height"
    if [[ $width -gt $height ]]; then
        ((height++))
    else
        ((width++))
    fi
done
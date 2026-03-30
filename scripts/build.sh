#!/bin/bash

echo "Building project..."

mkdir -p build
cd build

cmake ..
make -j4

echo "Done."
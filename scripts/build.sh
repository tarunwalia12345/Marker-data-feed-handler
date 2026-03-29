#!/bin/bash
set -e

echo "Building project."

rm -rf build
mkdir build
cd build

cmake ..
make -j

echo "Build complete"
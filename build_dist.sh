#!/bin/sh
set -e

echo "Building MarkIt Distribution..."

# Create a fresh release build directory
rm -rf build_release
cmake -B build_release -DCMAKE_BUILD_TYPE=Release
cmake --build build_release --config Release

echo "Distribution packages created in build_release directory!"

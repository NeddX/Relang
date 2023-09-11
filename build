#!/bin/bash

echo "[!]: Build started"

export CC=$(which clang)
export CXX=$(which clang++)

kernel=$(uname -s | tr '[:upper:]' '[:lower:]')
arch=$(uname -m | tr '[:upper:]' '[:lower:]')
platform="$kernel-$arch"
build_conf="Debug"
cpwd=$pwd

if [ -z "$1" ]; then
    build_conf="Debug"
else
    build_conf="$1"
fi

if [ ! -d "builds" ]; then
    echo "[!]: Configuring CMake..."
        mkdir -p "builds"
        mkdir -p "builds/$platform"
        cd "builds/$platform"
        cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="$build_conf" ../../
elif [ ! -d "builds/$platform" ]; then
        echo "[!]: Configuring CMake.."
        mkdir -p "builds/$platform"
        cd "builds/$platform"
        cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="$build_conf" ../../
else
        cd "builds/$platform"
fi

echo "[!]: Building..."
cmake --build ./

echo "[!]: Build finished."

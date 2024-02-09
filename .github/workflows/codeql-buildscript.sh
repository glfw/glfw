#!/usr/bin/env bash

sudo apt-get update
sudo apt-get install -y libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev libwayland-dev libxkbcommon-dev

cmake -B build-full-static -D GLFW_BUILD_WAYLAND=ON -D GLFW_BUILD_X11=ON
cmake --build build-full-static --parallel

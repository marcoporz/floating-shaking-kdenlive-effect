#!/usr/bin/env bash
# Build & install the "floating" MLT module + Kdenlive effect.
set -euo pipefail
cd "$(dirname "$0")"
echo "MLT version: $(pkg-config --modversion mlt-framework-7)"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
sudo cmake --install build          # copies libmltfloating.so into MLT's module dir
mkdir -p "$HOME/.local/share/kdenlive/effects"
cp kdenlive/floating.xml "$HOME/.local/share/kdenlive/effects/"
echo "Done. Restart Kdenlive and search for 'Floating' in the effects list."

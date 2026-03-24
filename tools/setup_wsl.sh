#!/bin/bash
# Run this inside WSL after installing Ubuntu:
#   wsl --install -d Ubuntu-24.04
#   wsl
#   cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"
#   bash tools/setup_wsl.sh

set -euo pipefail

echo "=== Installing system packages ==="
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    binutils-mips-linux-gnu \
    cpp-mips-linux-gnu \
    python3 \
    python3-pip \
    python3-venv \
    git \
    wget \
    cmake

echo ""
echo "=== Setting up Python virtual environment ==="
python3 -m venv .venv
source .venv/bin/activate

echo ""
echo "=== Installing Python tools ==="
pip install splat64 spimdisasm

echo ""
echo "=== Cloning maspsx ==="
if [ ! -d "tools/maspsx" ]; then
    git clone https://github.com/mkst/maspsx.git tools/maspsx
else
    echo "maspsx already cloned"
fi

echo ""
echo "=== Building GCC 2.7.2 (mips-cc1) ==="
if [ ! -d "tools/gcc-2.7.2" ]; then
    git clone https://github.com/decompals/mips-gcc-2.7.2.git tools/gcc-2.7.2
    cd tools/gcc-2.7.2
    make
    cd ../..
else
    echo "gcc-2.7.2 already built"
fi

echo ""
echo "=== Verifying toolchain ==="
echo -n "mips-linux-gnu-as: " && mips-linux-gnu-as --version | head -1
echo -n "mips-linux-gnu-ld: " && mips-linux-gnu-ld --version | head -1
echo -n "cc1 (GCC 2.7.2):  " && ls -la tools/gcc-2.7.2/build/cc1 2>/dev/null || echo "NOT FOUND"
echo -n "maspsx:            " && ls tools/maspsx/maspsx.py 2>/dev/null || echo "NOT FOUND"
echo -n "splat:             " && python3 -m splat --version 2>/dev/null || echo "NOT FOUND"

echo ""
echo "=== Setup complete! ==="
echo ""
echo "Next steps:"
echo "  1. Activate venv: source .venv/bin/activate"
echo "  2. Run splat:     python3 -m splat split splat.yaml"
echo "  3. Build:         make"

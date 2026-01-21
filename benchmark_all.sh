#!/bin/bash

EXE="./build/gpgpu_mm"

echo "========================================="
echo " BENCHMARKING: INTEL INTEGRATED GRAPHICS"
echo "========================================="
# Run default (usually Intel on hybrid laptops)
$EXE

echo ""
echo "========================================="
echo " BENCHMARKING: NVIDIA DISCRETE GPU"
echo "========================================="
# Force NVIDIA using Prime Offload environment variables
# We add --skip-cpu because we already benchmarked the CPU above
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia $EXE --skip-cpu

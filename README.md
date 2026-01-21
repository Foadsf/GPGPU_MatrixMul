# OpenGL GPGPU Matrix Multiplication

A benchmark comparing **CPU** vs **GPU** performance using $1024 \times 1024$ Matrix Multiplication ($O(N^3)$ complexity).

This project demonstrates the honest benefit of GPGPU. Unlike synthetic tests with artificial loops, Matrix Multiplication is a foundational operation in AI and Physics that naturally benefits from massive parallelism.

## The Benchmark
* **Task:** Calculate $C = A \times B$ for $1024 \times 1024$ matrices.
* **Complexity:** Requires approx. 2 Billion floating-point operations.
* **CPU Implementation:** Standard naive triple-loop ($O(N^3)$).
* **GPU Implementation:** OpenGL Compute Shader (4.6) dispatching 1 thread per element.

## Performance Results (Sony Vaio SVF15N2C5E)
*Hardware: Intel Core i7-4500U (Haswell) | Integrated Intel HD 4400 | Discrete NVIDIA GeForce GT 735M*

| Device | Execution Time | Speedup vs CPU | Notes |
| :--- | :--- | :--- | :--- |
| **CPU (Single Thread)** | ~26,174 ms | 1x | Baseline |
| **Intel HD 4400** | ~245 ms | **106x** | Integrated Graphics |
| **NVIDIA GT 735M** | ~153 ms | **171x** | Discrete Legacy GPU |

## Prerequisites
* **CMake** (3.15+)
* **C++ Compiler** (GCC/Clang/MSVC) with C++17 support
* **vcpkg** (Package Manager)
* **OpenGL 4.3+ capable GPU**

## Build Instructions (Linux)

This project uses `vcpkg` for dependency management.

1.  **Install Dependencies**
    ```bash
    # Ensure vcpkg is installed and bootstrapped
    cd /path/to/vcpkg
    ./vcpkg install glfw3 glad --triplet=x64-linux
    ```

2.  **Configure & Build**
    ```bash
    # From the project root
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build build --config Release
    ```

## Usage

### 1. Basic Run (Default GPU)
Runs the benchmark on the default GPU (usually Integrated graphics on laptops) and the CPU.
```bash
./build/gpgpu_mm

```

### 2. Skip CPU Benchmark

If you are iterating quickly and want to save ~20 seconds:

```bash
./build/gpgpu_mm --skip-cpu

```

### 3. Automated Hybrid GPU Benchmark

We have included a script `benchmark_all.sh` that detects your hardware configuration and runs the benchmark on **Integrated** and **Discrete** GPUs sequentially using environment variables.

```bash
chmod +x benchmark_all.sh
./benchmark_all.sh

```

---

# Lessons Learned & Troubleshooting Guide

This section documents the specific challenges encountered while setting up this project on a **legacy hybrid laptop (Sony Vaio)** running **Linux Mint 22 (Ubuntu 24.04 base)**.

### 1. Hybrid Graphics (NVIDIA Optimus) on Linux

Unlike Windows, OpenGL on Linux does not automatically switch GPUs based on power plans. To force the application to run on the discrete NVIDIA GPU without rebooting, we used "Prime Offload" environment variables:

```bash
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./build/gpgpu_mm

```

### 2. Legacy Hardware vs. Modern Kernels

**The Issue:** The NVIDIA GeForce GT 735M (Kepler architecture) is a legacy card supported only up to the **470.xx** driver series. However, the **Linux Kernel 6.14** introduced changes that broke the DKMS build process for the 470 driver.

**Symptoms:**

* `nvidia-smi` fails with "couldn't communicate with the NVIDIA driver".
* `apt install nvidia-driver-470` completes but reports DKMS build errors for the active kernel.

**The Solution:**
We downgraded/pinned the system to **Kernel 6.11**, which maintains compatibility with the legacy 470 drivers.

1. Boot into "Advanced Options" in GRUB.
2. Select Kernel 6.11.
3. Modify `/etc/default/grub` to make this persistent: `GRUB_DEFAULT="1>4"` (Submenu 1, Entry 4).

### 3. Missing Modules in Older Kernels

**The Issue:** Switching to an older kernel (6.11) on a fresh install resulted in a loss of WiFi (Intel 7260) and HDMI output. This was because the generic kernel image does not include all proprietary firmware/modules by default.

**The Solution:**
We manually installed the extra modules package for the specific kernel version:

```bash
sudo apt install linux-modules-extra-6.11.0-29-generic

```

*(Note: Since WiFi was down, this required downloading the `.deb` file on a different machine/kernel and transferring it via USB).*

### 4. Driver Persistence

To ensure the NVIDIA driver loads correctly and doesn't conflict with the open-source `nouveau` driver:

1. **Blacklist Nouveau:** Created `/etc/modprobe.d/blacklist-nouveau.conf`.
2. **Load NVIDIA Modules:** Added `nvidia`, `nvidia-modeset`, etc., to `/etc/modules-load.d/`.
3. **Update Initramfs:** `sudo update-initramfs -u`.

### 5. Git Submodule Workflow

This project is set up as a submodule. To ensure clean state:

* Use `git submodule add <URL> <path>` to mount it.
* Commit changes inside the submodule first, push them, and *then* commit the submodule reference update in the parent repository.

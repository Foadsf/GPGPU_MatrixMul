# OpenGL GPGPU Matrix Multiplication

A benchmark comparing **CPU** vs **GPU** performance using $1024 \times 1024$ Matrix Multiplication ($O(N^3)$ complexity). 

This project demonstrates the honest benefit of GPGPU. Unlike synthetic tests with artificial loops, Matrix Multiplication is a foundational operation in AI and Physics that naturally benefits from massive parallelism.

## The Benchmark
* **Task:** Calculate $C = A \times B$ for $1024 \times 1024$ matrices.
* **Complexity:** Requires approx. 2 Billion floating-point operations.
* **CPU Implementation:** Standard naive triple-loop ($O(N^3)$).
* **GPU Implementation:** OpenGL Compute Shader (4.6) dispatching 1 thread per element.

## Results (Typical)
* **CPU Time:** ~1850ms
* **GPU Time:** ~100ms (includes memory upload/download overhead)
* **Speedup:** ~18x

## Prerequisites
* [Scoop](https://scoop.sh/) (Windows Package Manager)
* Visual Studio (C++ Compiler)
* OpenGL 4.3+ capable GPU

## Build Instructions

1.  **Install Dependencies**
    ```powershell
    scoop install cmake vcpkg
    vcpkg integrate install
    vcpkg install glfw3 glad --triplet=x64-windows
    ```

2.  **Configure & Build**
    ```powershell
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$env:USERPROFILE/scoop/apps/vcpkg/current/scripts/buildsystems/vcpkg.cmake"
    cmake --build build --config Release
    ```

3.  **Run**
    ```powershell
    .\build\Release\gpgpu_mm.exe
    ```

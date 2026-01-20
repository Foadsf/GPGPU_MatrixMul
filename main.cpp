#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>

// --- Configuration ---
const int WIDTH = 1024; // 1024x1024 Matrix
const int SIZE = WIDTH * WIDTH;

// --- Utils ---
std::string loadShader(const char* filename) {
    std::ifstream file(filename);
    if (!file) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Standard CPU Matrix Multiplication (O(N^3))
void cpu_matrix_mult(const std::vector<float>& A, const std::vector<float>& B, std::vector<float>& C) {
    for (int row = 0; row < WIDTH; row++) {
        for (int col = 0; col < WIDTH; col++) {
            float sum = 0.0f;
            for (int k = 0; k < WIDTH; k++) {
                sum += A[row * WIDTH + k] * B[k * WIDTH + col];
            }
            C[row * WIDTH + col] = sum;
        }
    }
}

int main() {
    // 1. Generate Data
    std::cout << "Matrix Size: " << WIDTH << "x" << WIDTH << " (" << SIZE << " elements)" << std::endl;
    std::vector<float> A(SIZE);
    std::vector<float> B(SIZE);
    std::vector<float> C_CPU(SIZE); // Result from CPU
    std::vector<float> C_GPU(SIZE); // Result from GPU

    for (int i = 0; i < SIZE; i++) {
        A[i] = static_cast<float>(rand()) / RAND_MAX;
        B[i] = static_cast<float>(rand()) / RAND_MAX;
    }

    // 2. CPU Benchmark
    std::cout << "Starting CPU Matrix Multiplication..." << std::endl;
    auto startCPU = std::chrono::high_resolution_clock::now();
    
    cpu_matrix_mult(A, B, C_CPU);

    auto endCPU = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> msCPU = endCPU - startCPU;
    std::cout << "CPU Time: " << msCPU.count() << " ms" << std::endl;


    // 3. GPU Setup
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hidden", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // Compile
    std::string sourceStr = loadShader("compute.glsl");
    const char* src = sourceStr.c_str();
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    glUseProgram(program);

    // 4. GPU Benchmark
    std::cout << "Starting GPU Matrix Multiplication..." << std::endl;
    auto startGPU = std::chrono::high_resolution_clock::now();

    // Create Buffers (A, B, C)
    GLuint ssboA, ssboB, ssboC;
    glGenBuffers(1, &ssboA);
    glGenBuffers(1, &ssboB);
    glGenBuffers(1, &ssboC);

    // Upload A
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboA);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * sizeof(float), A.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboA);

    // Upload B
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboB);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * sizeof(float), B.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboB);

    // Allocate C (GPU needs memory to write to)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboC);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboC);

    // Set Uniform Size
    glUniform1i(glGetUniformLocation(program, "WIDTH"), WIDTH);

    // Dispatch
    // We break the 1024x1024 grid into groups of 32x32 threads.
    // 1024 / 32 = 32 groups.
    glDispatchCompute(WIDTH / 32, WIDTH / 32, 1);
    
    // Barrier
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Readback
    float* ptr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(C_GPU.data(), ptr, SIZE * sizeof(float));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    auto endGPU = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> msGPU = endGPU - startGPU;
    std::cout << "GPU Time: " << msGPU.count() << " ms" << std::endl;

    // 5. Verification
    bool correct = true;
    for (int i = 0; i < SIZE; i++) {
        // Use a slightly larger epsilon due to floating point accumulation diffs
        if (abs(C_CPU[i] - C_GPU[i]) > 0.1f) { 
            correct = false;
            std::cout << "Mismatch at " << i << " CPU:" << C_CPU[i] << " GPU:" << C_GPU[i] << std::endl;
            break;
        }
    }

    std::cout << "Results Match: " << (correct ? "YES" : "NO") << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Speedup Factor: " << msCPU.count() / msGPU.count() << "x" << std::endl;

    glfwTerminate();
    return 0;
}
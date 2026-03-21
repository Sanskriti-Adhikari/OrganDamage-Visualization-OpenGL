#include "lungs.h"
#include "renderer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <cstdio>

static LungSimulation* g_sim = nullptr;

static void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    if (height <= 0) height = 1;
    glViewport(0, 0, width, height);
    R_buildOrtho(width, height);
}

static void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (!g_sim || (action != GLFW_PRESS && action != GLFW_REPEAT)) return;

    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_RIGHT) g_sim->onKeyRight();
    if (key == GLFW_KEY_LEFT) g_sim->onKeyLeft();
    if (key == GLFW_KEY_SPACE) g_sim->toggleAuto();
    if (key == GLFW_KEY_R) g_sim->reset();
}

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 760, "Lungs Deterioration - Smoking Damage Over Time", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    int fbw = 1200, fbh = 760;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    framebuffer_size_callback(window, fbw, fbh);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    R_init();

    LungSimulation sim;
    sim.reset();
    g_sim = &sim;

    glfwSetKeyCallback(window, key_callback);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        lastTime = now;
        if (dt > 0.05f) dt = 0.05f;

        sim.update(dt);

        glClearColor(0.07f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        sim.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    g_sim = nullptr;
    R_cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

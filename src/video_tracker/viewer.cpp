#include "viewer.hpp"

#include <GLFW/glfw3.h>
#include <format>
#include <stdexcept>
#include <utility>

video_tracker::Viewer::Viewer(int width, int height) {
    if (!glfwInit()) {
        throw std::runtime_error(std::format("unable to init GLFW"));
    }

    m_window = glfwCreateWindow(width, height, "VideoTracker", nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error(std::format("unable to create window"));
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int, int action, int) {
        auto* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
        viewer->m_on_key(key, action);
    });
}

video_tracker::Viewer::~Viewer() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool video_tracker::Viewer::should_close() const {
    return glfwWindowShouldClose(m_window);
}

video_tracker::Viewer::Events video_tracker::Viewer::poll_events() {
    glfwPollEvents();
    return std::exchange(m_pending_events, Events{});
}

void video_tracker::Viewer::set_frame(const AVFrame* frame) {
    if (frame) return;
}

void video_tracker::Viewer::draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(m_window);
}

void video_tracker::Viewer::m_on_key(int key, int action) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        m_pending_events.toggle_playback =
            !m_pending_events.toggle_playback;
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        m_pending_events.step_requested = true;
    }
}

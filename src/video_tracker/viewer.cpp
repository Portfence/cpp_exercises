#include "viewer.hpp"


#include <GL/gl.h>
#include <format>
extern "C" {
    #include <libavutil/hwcontext.h>
    #include <libavutil/hwcontext_cuda.h>
}
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

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

video_tracker::Viewer::~Viewer() {
    cuGraphicsUnregisterResource(m_resource);
    glDeleteTextures(1, &m_texture);
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
    const auto hw_frames_ctx = reinterpret_cast<const AVHWFramesContext*>(frame->hw_frames_ctx->data);
    auto* device_ctx = hw_frames_ctx->device_ctx;

    if (!device_ctx || device_ctx->type != AV_HWDEVICE_TYPE_CUDA) {
        throw std::runtime_error("expected a CUDA device");
    }

    auto cuda = static_cast<AVCUDADeviceContext*>(device_ctx->hwctx);
    auto context = cuda->cuda_ctx;
    const auto res = cuCtxPushCurrent(context); // TODO: check if scu
    if (res != CUDA_SUCCESS) {
        const char* name = nullptr;
            cuGetErrorName(res, &name);

            throw std::runtime_error(std::format(
                "CUDA push current context failed: {} ({})",
                name ? name : "unknown",
                static_cast<int>(res)));
    }

    if (!m_resource) { // consider moving this into constructor
        glTexImage2D(
        GL_TEXTURE_2D, 0, GL_R8,
        frame->width, frame->height, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);

        CUresult res = cuGraphicsGLRegisterImage(
            &m_resource,
            m_texture,
            GL_TEXTURE_2D,
            CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD
        );


        if (res != CUDA_SUCCESS) {
            const char* name = nullptr;
            cuGetErrorName(res, &name);

            throw std::runtime_error(std::format(
                "CUDA image registration failed: {} ({})",
                name ? name : "unknown",
                static_cast<int>(res)));
        }
    }

    auto stream = cuda->stream;
    cuGraphicsMapResources(1, &m_resource, stream);

    CUarray destination = nullptr;
    cuGraphicsSubResourceGetMappedArray(&destination, m_resource, 0, 0);

    // copy or convert pixels

    cuGraphicsUnmapResources(1, &m_resource, stream);

    cuStreamSynchronize(stream); // TODO check error
    CUcontext popped = nullptr;
    cuCtxPopCurrent(&popped);
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

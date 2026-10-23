#include "viewer.hpp"

#include <format>
#include <fstream>
#include <iterator>
extern "C" {
    #include <libavutil/hwcontext.h>
    #include <libavutil/hwcontext_cuda.h>
}
#include <stdexcept>
#include <utility>

namespace {
    std::string read_file_to_string(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error(std::format("could not open file {}", filepath));
        }

        return std::string(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
    }
}

video_tracker::Viewer::Viewer(int width, int height) {
    if (!glfwInit()) {
        throw std::runtime_error(std::format("unable to init GLFW"));
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(width, height, "VideoTracker", nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error(std::format("unable to create window"));
    }

    glfwMakeContextCurrent(m_window);
    // need to be immediately after make context current
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        throw std::runtime_error("Failed to load OpenGL functions");
    }
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

    const std::string base = "src/video_tracker/shaders";
    const std::string vertex_string = read_file_to_string(std::format("{}/vertex.glsl", base));
    const std::string fragment_string = read_file_to_string(std::format("{}/fragment.glsl", base));

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_code = vertex_string.c_str();
    glShaderSource(vertex_shader, 1, &vertex_code, nullptr);
    glCompileShader(vertex_shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[1024]{};
        glGetShaderInfoLog(vertex_shader, sizeof(log), nullptr, log);
        throw std::runtime_error(std::format("Vertex shader: {}", log));
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_code = fragment_string.c_str();
    glShaderSource(fragment_shader, 1, &fragment_code, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[1024]{};
        glGetShaderInfoLog(fragment_shader, sizeof(log), nullptr, log);
        throw std::runtime_error(std::format("Fragment shader: {}", log));
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex_shader);
    glAttachShader(m_program, fragment_shader);
    glLinkProgram(m_program);
    GLint linked = GL_FALSE;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024]{};
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        throw std::runtime_error(std::format("Shader linking: {}", log));
    }
    glGenVertexArrays(1, &m_vao);

    glUseProgram(m_program);
    glUniform1i(
        glGetUniformLocation(m_program, "frame_texture"),
        0
    );

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

video_tracker::Viewer::~Viewer() {
    cuGraphicsUnregisterResource(m_resource);
    glDeleteTextures(1, &m_texture);
    glDeleteProgram(m_program);
    glDeleteVertexArrays(1, &m_vao);
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

    // resource is allocated after this
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
    CUDA_MEMCPY2D_st copy{};
    copy.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    copy.srcDevice = reinterpret_cast<CUdeviceptr>(frame->data[0]);
    copy.srcPitch = frame->linesize[0];
    copy.dstMemoryType = CU_MEMORYTYPE_ARRAY;
    copy.dstArray = destination;
    copy.WidthInBytes = frame->width;
    copy.Height = frame->height;

    if (cuMemcpy2DAsync(&copy, stream) != CUDA_SUCCESS) {
        throw std::runtime_error("failed to cuMemcpy");
    }

    cuGraphicsUnmapResources(1, &m_resource, stream);

    cuStreamSynchronize(stream); // TODO check error
    m_has_frame = true;
    CUcontext popped = nullptr;
    cuCtxPopCurrent(&popped);
}

void video_tracker::Viewer::draw() {
    int width;
    int height;

    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);

    if (m_has_frame) {
        glUseProgram(m_program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

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

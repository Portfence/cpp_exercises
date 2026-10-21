#pragma once

#include <cuda.h>
#include <cudaGL.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <libavutil/frame.h>

namespace video_tracker {
class Viewer{
public:
    struct Events {
        bool toggle_playback = false;
        bool step_requested = false;
    };

    Viewer(int width, int height);
    ~Viewer();

    Viewer(const Viewer&) = delete;
    Viewer& operator=(const Viewer&) = delete;

    bool should_close() const;

    Events poll_events();
    void set_frame(const AVFrame* frame);
    void draw();
private:
    GLFWwindow* m_window = nullptr;
    Events m_pending_events;
    GLuint m_texture = 0;
    CUgraphicsResource m_resource = nullptr;

    void m_on_key(int key, int action);
};
}

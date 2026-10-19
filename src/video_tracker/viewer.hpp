#pragma once

#include <GLFW/glfw3.h>

namespace video_tracker {
class Viewer{
public:
    struct Events {
        bool toggle_playback = false;
    };

    Viewer(int width, int height);
    ~Viewer();

    Viewer(const Viewer&) = delete;
    Viewer& operator=(const Viewer&) = delete;

    bool should_close() const;

    Events poll_events();
    void draw();
private:
    GLFWwindow* m_window = nullptr;
    Events m_pending_events;
};
}

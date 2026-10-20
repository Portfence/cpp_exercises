#include "app.hpp"

#include "playback.hpp"
#include "viewer.hpp"

#include <exception>
#include <format>
#include <print>

extern "C" {
#include <libavutil/avutil.h>
}

using std::println;

video_tracker::App::App() {}

int video_tracker::App::run_viewer() {

    println("FFmpeg {}", av_version_info());

    try {
        auto viewer = Viewer(1920, 1080);
        auto playback = Playback("resources/people_uhd60fps.mp4");
        println("  <SPACE>      : play/pause");
        println("  <RIGHT_ARROW>: step frames (only when paused)");

        while (true) {
            const auto viewer_events = viewer.poll_events();
            if (viewer.should_close()) {
                break;
            }

            if (viewer_events.toggle_playback) {
                playback.toggle_pause();
            }

            // need to pause stream to step framess
            if (viewer_events.step_requested && playback.paused()) {
                playback.step_forward();
            }

            playback.update();

            viewer.draw();
        }
    } catch (const std::exception& e) {
        println("video player error: {}", e.what());
        return 1;
    }

    return 0;
}

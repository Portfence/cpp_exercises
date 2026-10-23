#include "app.hpp"

#include "playback.hpp"
#include "viewer.hpp"

#include <cstdlib>
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
        auto playback = Playback("resources/people_uhd60fps.mp4");
        auto viewer = Viewer(1920, 1080);
        println("  <SPACE>      : play/pause");
        println("  <RIGHT_ARROW>: step frames (only when paused)");

        while (true) {
            const auto viewer_events = viewer.poll_events();
            if (viewer.should_close()) {
                break;
            }

            if (viewer_events.toggle_playback) {
                playback.toggle_pause();
                if (playback.paused()) {
                     println("paused at frame {}", playback.frame_counter());
                } else {
                    println("resuming");
                }
            }

            // need to pause stream to step framess
            if (viewer_events.step_requested && playback.paused()) {
                playback.step_forward();
            }

            if (playback.update()) {
                viewer.set_frame(playback.current_frame());
            }

            viewer.draw();
        }
    } catch (const std::exception& e) {
        println("app error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

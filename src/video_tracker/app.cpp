#include "app.hpp"

#include "common.hpp"
#include "decoder.hpp"
#include "playback.hpp"
#include "viewer.hpp"

#include <exception>
#include <format>
#include <memory>
#include <print>
#include <utility>

extern "C" {
#include <libavutil/avutil.h>
}

using std::println;
using video_tracker::FramePtr;

video_tracker::App::App() {}

int video_tracker::App::run_viewer() {

    println("FFmpeg {}", av_version_info());

    try {
        auto viewer = Viewer(1920, 1080);
        auto playback = Playback("resources/people_uhd60fps.mp4");
        FramePtr current_frame;
        println("Space: play/pause");
        while (1) {
            const auto events = viewer.poll_events();
            if (viewer.should_close()) {
                break;
            }

            if (events.toggle_playback) {
                playback.toggle_pause();
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

#include "app.hpp"

#include "common.hpp"
#include "decoder.hpp"

#include <exception>
#include <format>
#include <print>

extern "C" {
#include <libavutil/avutil.h>
}

using std::println;
using video_tracker::VideoDecoder;
using video_tracker::FramePtr;

video_tracker::App::App() {}

int video_tracker::App::run() {
    println("FFmpeg {}", av_version_info());

    try {
        auto video_decoder = VideoDecoder("resources/people_uhd60fps.mp4");
        int i = 0;
        while (FramePtr frame = video_decoder.next_frame()) {
            if (frame) {
                i++;
            }
        }
        println("received {} frames", i);
    } catch (std::exception& e) {
        println("decoder error: {}", e.what());
    }

    return 0;
}

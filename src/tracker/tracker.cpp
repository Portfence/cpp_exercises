#include "tracker.hpp"
#include "decoder.hpp"

#include <cerrno>
#include <exception>
#include <format>
#include <print>

extern "C" {
#include <libavcodec/packet.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavcodec/codec.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext.h>
}

using std::println;

tracker::Tracker::Tracker() {}

int tracker::Tracker::run() {
    println("FFmpeg {}", av_version_info());

    try {
        auto video_decoder = VideoDecoder("resources/people_uhd60fps.mp4");
        int i = 0;
        while (auto frame = video_decoder.next_frame()) {
            if (frame) {
                // print("decoded frame number: {} ", m_codec_context->frame_num);
                if (frame->format == AV_PIX_FMT_CUDA) {
                    println("CUDA frame received");
                } else {
                    println("some other frame received");
                }
                i++;
            }
            av_frame_unref(frame);
        }
        println("received {} frames", i);
    } catch (std::exception& e) {
        println("decoder error: {}", e.what());
    }

    return 0;
}

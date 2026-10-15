#pragma once

extern "C"{
    #include <libavutil/frame.h>
}
#include <memory>

namespace video_tracker {
struct FrameDeleter {
    void operator()(AVFrame* frame) const noexcept {
        av_frame_free(&frame);
    }
};

using FramePtr = std::unique_ptr<AVFrame, FrameDeleter>;
}

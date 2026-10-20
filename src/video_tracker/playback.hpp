#pragma once

#include "common.hpp"
#include "decoder.hpp"
#include <libavutil/frame.h>
#include <string>

namespace video_tracker {
class Playback{
    public:
        explicit Playback(const std::string& source);
        ~Playback();

        void toggle_pause();
        void step_forward();

        void update();

        const AVFrame* current_frame() const noexcept;
        bool paused() const noexcept;
        bool finished() const noexcept;
    private:
        VideoDecoder m_video_decoder;
        FramePtr m_current_frame;
        int m_counter = 0;

        bool m_paused = false;
        bool m_finished = false;
        bool m_step_requested = false;
};
}

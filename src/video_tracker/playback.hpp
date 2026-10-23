#pragma once

#include "common.hpp"
#include "decoder.hpp"

#include <string>

extern "C" {
    #include <libavutil/frame.h>
}

namespace video_tracker {
class Playback{
    public:
        explicit Playback(const std::string& source);
        ~Playback();

        void toggle_pause() noexcept;
        void step_forward() noexcept;

        /// Updates the internal state of Playback.
        /// Returns true when new frame is available, false if not
        bool update();

        const AVFrame* current_frame() const noexcept;
        int frame_counter() const noexcept;
        bool paused() const noexcept;
        bool finished() const noexcept;
    private:
        VideoDecoder m_video_decoder;
        FramePtr m_current_frame;
        int m_frame_counter = 0;

        bool m_paused = false;
        bool m_finished = false;
        bool m_step_requested = false;
};
}

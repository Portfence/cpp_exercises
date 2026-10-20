#include "playback.hpp"

#include <print>

extern "C" {
    #include <libavutil/frame.h>
}

video_tracker::Playback::Playback(const std::string& source) : m_video_decoder(source) {}

video_tracker::Playback::~Playback() {}

void video_tracker::Playback::toggle_pause() {
    m_paused = !m_paused;
    m_paused ? std::println("paused at frame {}", m_counter) : std::println("resuming");
}

void video_tracker::Playback::step_forward() {
    m_step_requested = true;
}

void video_tracker::Playback::update() {
    if (!m_paused || m_step_requested) {
        if (auto frame = m_video_decoder.next_frame()) {
            m_current_frame = std::move(frame);
            m_counter++;
        } else {
            m_paused = true;
            m_finished = true;
            std::println("Stream EOF: {} frames", m_counter);
        }

        if (m_step_requested) {
            std::println("stepping to frame {}", m_counter);
            m_step_requested = false;
        }
    }
}

const AVFrame* video_tracker::Playback::current_frame() const noexcept {
    return m_current_frame.get();
}

bool video_tracker::Playback::paused() const noexcept {
    return m_paused;
}

bool video_tracker::Playback::finished() const noexcept {
    return m_finished;
}


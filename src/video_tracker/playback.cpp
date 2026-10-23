#include "playback.hpp"

#include <print>

extern "C" {
    #include <libavutil/frame.h>
}

video_tracker::Playback::Playback(const std::string& source) : m_video_decoder(source) {}

video_tracker::Playback::~Playback() {}

void video_tracker::Playback::toggle_pause() noexcept {
    m_paused = !m_paused;
}

void video_tracker::Playback::step_forward() noexcept {
    m_step_requested = true;
}

bool video_tracker::Playback::update() {
    bool new_frame = false;
    if (!m_paused || m_step_requested) {
        if (auto frame = m_video_decoder.next_frame()) {
            m_current_frame = std::move(frame);
            m_frame_counter++;
            new_frame = true;
        } else {
            m_paused = true;
            m_finished = true;
            std::println("Stream EOF: {} frames", m_frame_counter);
        }

        if (m_step_requested) {
            std::println("stepping to frame {}", m_frame_counter);
            m_step_requested = false;
        }
    }
    return new_frame;
}

const AVFrame* video_tracker::Playback::current_frame() const noexcept {
    return m_current_frame.get();
}

int video_tracker::Playback::frame_counter() const noexcept {
    return m_frame_counter;
}

bool video_tracker::Playback::paused() const noexcept {
    return m_paused;
}

bool video_tracker::Playback::finished() const noexcept {
    return m_finished;
}


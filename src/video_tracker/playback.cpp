#include "playback.hpp"
#include <libavutil/frame.h>
#include <print>

video_tracker::Playback::Playback(const std::string& source) : m_video_decoder(source) {}
video_tracker::Playback::~Playback() {}

void video_tracker::Playback::toggle_pause() {
    m_paused = !m_paused;
    m_paused ? std::println("paused") : std::println("resumed");
}
void video_tracker::Playback::step_forward() {}

void video_tracker::Playback::update() {
    if (!m_paused) {
        if (auto frame = m_video_decoder.next_frame()) {
            m_current_frame = std::move(frame);
            m_counter++;
        } else {
            m_paused = true;
            m_finished = true;
            std::println("Stream EOF: {} frames", m_counter);
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


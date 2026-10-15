#pragma once

#include "common.hpp"

#include <string>
extern "C" {
    #include <libavcodec/packet.h>
    #include <libavcodec/codec.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

namespace video_tracker {
class VideoDecoder{
public:
    explicit VideoDecoder(const std::string& filepath);
    ~VideoDecoder();

    VideoDecoder(const VideoDecoder&) = delete;
    VideoDecoder& operator=(const VideoDecoder&) = delete;

    /// Returns an owned frame, or nullptr at EOF. Throws on failure.
    [[nodiscard]] FramePtr next_frame();

private:
    AVFormatContext* m_format_context = nullptr;
    const AVCodec* m_decoder = nullptr;
    AVStream* m_video_stream = nullptr;
    AVCodecContext* m_codec_context = nullptr;
    AVPacket* m_packet = nullptr;
    int m_video_stream_index = -1;

};
} // namespace video_tracker

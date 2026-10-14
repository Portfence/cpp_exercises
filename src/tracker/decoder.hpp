#pragma once

#include <string_view>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/frame.h>
}

namespace tracker {
class VideoDecoder{
public:
    explicit VideoDecoder(std::string_view filepath);
    ~VideoDecoder();
    [[nodiscard]] AVFrame* next_frame();
private:
    AVFormatContext* m_format_context = nullptr;
    const AVCodec* m_decoder = nullptr;
    AVStream* m_video_stream = nullptr;
    AVCodecContext* m_codec_context = nullptr;
    AVPacket* m_packet = nullptr;
    int m_video_stream_index;

};
} // namespace tracker

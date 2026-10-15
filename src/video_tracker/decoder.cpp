
#include "decoder.hpp"
#include "common.hpp"

#include <cerrno>
#include <format>
#include <print>
#include <stdexcept>
#include <utility>
extern "C" {
#include <libavutil/pixfmt.h>
#include <libavcodec/packet.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/hwcontext.h>
}

using std::println;

namespace {
    AVPixelFormat choose_cuda(
        AVCodecContext*,
        const AVPixelFormat* formats)
        {
            for (; *formats != AV_PIX_FMT_NONE; ++formats) {
                if (*formats == AV_PIX_FMT_CUDA)
                return *formats;
        }
        return AV_PIX_FMT_NONE;
    }
}

// https://ffmpeg.org/doxygen/trunk/hw_decode_8c-example.html
video_tracker::VideoDecoder::VideoDecoder(const std::string& filepath) : m_packet(av_packet_alloc()){
    // allocate format context and open file
    if (avformat_open_input(&m_format_context, filepath.c_str(), nullptr, nullptr) < 0) {
        throw std::runtime_error(std::format("error: could not open source file {}", filepath));
    }

    // retrieve stream info (headers/metadata)
    if (avformat_find_stream_info(m_format_context, nullptr) < 0) {
        avformat_close_input(&m_format_context);
        throw std::runtime_error(std::format("error: could not find stream info"));
    }

    // find the best video stream inside the container
    m_video_stream_index = av_find_best_stream(m_format_context, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &m_decoder, 0);
    if (m_video_stream_index < 0) {
        avformat_close_input(&m_format_context);
        throw std::runtime_error(std::format("error: could not find best stream info"));
    }

    m_video_stream = m_format_context->streams[m_video_stream_index];
    // TODO: consider if this decoder should print this (maybe log/provide info-getter instead)
    println("Successfully opened {}", filepath);
    println("Codec name {}", m_decoder->long_name);
    println("Resolution {}x{}", m_video_stream->codecpar->width, m_video_stream->codecpar->height);

    // allocate a codec context for the decoder
    m_codec_context = avcodec_alloc_context3(m_decoder);
    if (!m_codec_context) {
        avformat_close_input(&m_format_context);
        throw std::runtime_error(std::format("error: could not allocate codec context"));
    }

    // copy parameters from the stream to the codec context
    if (avcodec_parameters_to_context(m_codec_context, m_video_stream->codecpar) < 0) {
        avcodec_free_context(&m_codec_context);
        avformat_close_input(&m_format_context);
        throw std::runtime_error(std::format("error: could not copy codec parameters"));
    }

    m_codec_context->get_format = choose_cuda;
    int result = av_hwdevice_ctx_create(&m_codec_context->hw_device_ctx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0);
    if (result < 0) {
        avcodec_free_context(&m_codec_context);
        avformat_close_input(&m_format_context);
        throw std::runtime_error(std::format("error: could not create hwdevice context"));
    }

    // open the decoder
    if(avcodec_open2(m_codec_context, m_decoder, nullptr) < 0) {
        avcodec_free_context(&m_codec_context);
        avformat_close_input(&m_format_context);
        throw std::runtime_error(std::format("error: could not open codec"));
    }
}

video_tracker::VideoDecoder::~VideoDecoder() {
    avcodec_free_context(&m_codec_context);
    avformat_free_context(m_format_context);
}

video_tracker::FramePtr video_tracker::VideoDecoder::next_frame() {
    auto frame = FramePtr(av_frame_alloc());

    while(1) {
        if (const int response = avcodec_receive_frame(m_codec_context, frame.get()); response >= 0) return frame;
        else if (response == AVERROR_EOF) return nullptr;
        else if (response == AVERROR(EAGAIN)) {} // decoder needs input - fall through
        else throw std::runtime_error(std::format("failed to receive frame from decoder"));

        if (const int response = av_read_frame(m_format_context, m_packet); response >= 0 && m_packet->stream_index == m_video_stream_index) {
            if (avcodec_send_packet(m_codec_context, m_packet) < 0) {
                throw std::runtime_error(std::format("failed to sent packet to decoder"));
            }
            // fall through and try to return a frame on next iteration
        }
        else if (response == AVERROR_EOF) {
            // stream finished - tell decoder
            if (avcodec_send_packet(m_codec_context, nullptr) != 0) {
                throw std::runtime_error(std::format("failed to send NULL packet to decoder"));
            }
        }
        else if (response < 0) {
            throw std::runtime_error("failed to read packet");
        }
        av_packet_unref(m_packet);
    }
    std::unreachable();
}



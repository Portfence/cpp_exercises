#include "tracker.hpp"
#include <cerrno>
#include <format>
#include <print>
#include <string>

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
using std::print;

static AVPixelFormat choose_cuda(
    AVCodecContext*,
    const AVPixelFormat* formats)
{
    for (; *formats != AV_PIX_FMT_NONE; ++formats) {
        if (*formats == AV_PIX_FMT_CUDA)
            return *formats;
    }
    return AV_PIX_FMT_NONE;
}

tracker::Tracker::Tracker() {}

int tracker::Tracker::run() {
    println("FFmpeg {}", av_version_info());
    // Next: demux with libavformat, then decode via libavcodec using an
    // AVHWDeviceContext and keep the hardware AVFrames on the GPU.
    // https://ffmpeg.org/doxygen/trunk/hw_decode_8c-example.html
    const auto filepath = std::string("resources/people_uhd60fps.mp4");

    // allocate format context and open file
    AVFormatContext* format_context = nullptr;
    if (avformat_open_input(&format_context, filepath.c_str(), nullptr, nullptr) < 0) {
        println("error: could not open source file {}", filepath);
        return -1;
    }

    // retrieve stream info (headers/metadata)
    if (avformat_find_stream_info(format_context, nullptr) < 0) {
        println("error: could not find stream info");
        avformat_close_input(&format_context);
        return -1;
    }

    // find the best video stream inside the container
    const AVCodec* decoder = nullptr;
    const int video_stream_index = av_find_best_stream(format_context, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (video_stream_index < 0) {
        println("error: could not find best stream info");
        avformat_close_input(&format_context);
        return -1;
    }

    AVStream* video_stream = format_context->streams[video_stream_index];
    println("Successfully opened {}", filepath);
    println("Codec name {}", decoder->long_name);
    println("Resolution {}x{}", video_stream->codecpar->width, video_stream->codecpar->height);

    // allocate a codec context for the decoder
    AVCodecContext* codec_context = avcodec_alloc_context3(decoder);
    if (!codec_context) {
        println("error: could not allocate codec context");
        avformat_close_input(&format_context);
        return -1;
    }
    // copy parameters from the stream to the codec context
    if (avcodec_parameters_to_context(codec_context, video_stream->codecpar) < 0) {
        println("error: could not copy codec parameters");
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    codec_context->get_format = choose_cuda;
    int result = av_hwdevice_ctx_create(&codec_context->hw_device_ctx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0);
    if (result < 0) {
        println("error: could not create hwdevice context");
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    // open the decoder
    if(avcodec_open2(codec_context, decoder, nullptr) < 0) {
        println("error: could not open codec");
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }
    // preparation loop for the reading data packet by packet

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    while(av_read_frame(format_context, packet) >= 0) {
         // Filter out non-video packets (e.g., audio streams)
        if (packet->stream_index == video_stream_index) {
            int response = avcodec_send_packet(codec_context, packet);
            if (response < 0) break;
            while(response >= 0) {
                response = avcodec_receive_frame(codec_context, frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    println("error during decoding");
                    goto cleanup;
                }
                // process frame here
                print("decoded frame number: {} ", codec_context->frame_num);
                if (frame->format == AV_PIX_FMT_CUDA) {println("with CUDA");} else {println("");}
            }
        }
        av_packet_unref(packet);
    }

cleanup:
    av_packet_unref( packet);
    av_frame_unref(frame);
    avformat_close_input(&format_context);
    avcodec_free_context(&codec_context);

    return 0;
}

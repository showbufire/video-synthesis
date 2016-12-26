#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace video_syn {

  class VideoDecoder {

  public:
    VideoDecoder(const char *mediaFile);

    virtual ~VideoDecoder();

    VideoDecoder(const VideoDecoder&) = delete;

    VideoDecoder &operator=(const VideoDecoder&) = delete;

    bool nextFrame(AVFrame *pFrame);

    int getWidth();

    int getHeight();

    AVPixelFormat getPixelFormat();

  private:
    AVCodec *pCodec = nullptr;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx = nullptr;
    AVPacket packet;
  };

}

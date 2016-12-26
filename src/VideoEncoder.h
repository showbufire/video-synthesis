#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace video_syn {

  class VideoEncoder {
  public:
    struct Config;
    VideoEncoder(const char *filename, const Config &config);

    virtual ~VideoEncoder();

    VideoEncoder(const VideoEncoder&) = delete;

    VideoEncoder &operator=(const VideoEncoder&) = delete;

    void encodeFrame(AVFrame *frame);

    void finish();

  private:
    bool finished = false;
    AVPacket packet;
    AVCodec *pCodec = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    FILE *pFile;

  public:
    struct Config {
      int width;
      int height;
      AVPixelFormat pix_fmt;
      int bit_rate;
      AVRational time_base;
      int gop_size;
      int max_b_frames;
      AVCodecID codec_id;
    };
  };

}

#include "VideoDecoder.h"

#include <stdexcept>
#include <iostream>

namespace video_syn {

  VideoDecoder::VideoDecoder(const char *mediaFilename) {
    if (avformat_open_input(&pFormatCtx, mediaFilename, nullptr, nullptr)) {
      throw std::runtime_error("decoder could not open media file");
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr)<0) {
      throw std::runtime_error("could not find stream information");
    }
    av_dump_format(pFormatCtx, 0, mediaFilename, 0);

    pCodecCtx = pFormatCtx->streams[0]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == nullptr) {
      throw std::runtime_error("Unsuported codec!");
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
      throw std::runtime_error("Could not open input codec");
    }
  }

  bool VideoDecoder::nextFrame(AVFrame *pFrame) {
    int frameFinished, len;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
      len = avcodec_decode_video2(pCodecCtx,
                                  pFrame,
                                  &frameFinished,
                                  &packet);
      if (len < 0) {
        throw std::runtime_error("Error while decoding frame");
      }
      if (frameFinished) {
        return true;
      }
    }
    return false;
  }

  int VideoDecoder::getWidth() {
    return pCodecCtx->width;
  }

  int VideoDecoder::getHeight() {
    return pCodecCtx->height;
  }

  AVPixelFormat VideoDecoder::getPixelFormat() {
    return pCodecCtx->pix_fmt;
  }

  VideoDecoder::~VideoDecoder() {
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
  }
}

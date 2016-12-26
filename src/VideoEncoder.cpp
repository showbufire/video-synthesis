#include "VideoEncoder.h"

#include <stdexcept>

static uint8_t endcode[] = { 0, 0, 1, 0xb7 };

namespace video_syn {

  VideoEncoder::VideoEncoder(const char *filename, const Config &config) {
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
      throw std::runtime_error("Could not allocate video codec context");
    }
    pCodecCtx->width = config.width;
    pCodecCtx->height = config.height;
    pCodecCtx->bit_rate = config.bit_rate;
    pCodecCtx->time_base = config.time_base;
    pCodecCtx->gop_size = config.gop_size;
    pCodecCtx->max_b_frames = config.max_b_frames;
    pCodecCtx->pix_fmt = config.pix_fmt;

    pCodec = avcodec_find_encoder(config.codec_id);
    if (!pCodec) {
      throw std::runtime_error("Codec not found");
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
      throw std::runtime_error("Could not open output codec");
    }

    pFile = fopen(filename, "wb");
    if (!pFile) {
      throw std::runtime_error("Could not open file ");
    }
  }

  VideoEncoder::~VideoEncoder() {
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
  }

  void VideoEncoder::encodeFrame(AVFrame *pFrame) {
    if (finished) {
      throw std::runtime_error("encoder already finished");
    }
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    int gotOutput;
    int ret = avcodec_encode_video2(pCodecCtx,
                                    &packet,
                                    pFrame,
                                    &gotOutput);
    if (ret < 0) {
      throw std::runtime_error("Error encoding frame");
    }
    if (gotOutput) {
      fwrite(packet.data, 1, packet.size, pFile);
      av_packet_unref(&packet);
    }
  }

  void VideoEncoder::finish() {
    for (int gotOutput = 1; gotOutput; ) {
      av_init_packet(&packet);
      packet.data = NULL;
      packet.size = 0;
      int ret = avcodec_encode_video2(pCodecCtx,
                                      &packet,
                                      nullptr,
                                      &gotOutput);
      if (ret < 0) {
        throw std::runtime_error("Error encoding frame");
      }
      if (gotOutput) {
        fwrite(packet.data, 1, packet.size, pFile);
        av_packet_unref(&packet);
      }
    }
    fwrite(endcode, 1, sizeof(endcode), pFile);
    fclose(pFile);
    finished = true;
  }
}


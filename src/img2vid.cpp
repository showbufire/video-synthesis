#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstdint>

#include "VideoDecoder.h"
#include "VideoEncoder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/common.h>
#include <libswscale/swscale.h>
}

const int FPS = 25;
const int TIME_LENGTH = 4; // seconds;
const int TOTAL_FRAMES = FPS * TIME_LENGTH;
const char *OUTPUT_FILENAME = "img2vid.mpg";

const AVCodecID CODEC_ID = AV_CODEC_ID_MPEG1VIDEO;

using namespace video_syn;

AVFrame *initFrame(int width, int height) {
  AVFrame *pFrame = av_frame_alloc();
  pFrame->format = AV_PIX_FMT_YUV420P;
  pFrame->width = width;
  pFrame->height = height;
  int ret = av_image_alloc(pFrame->data,
                           pFrame->linesize,
                           pFrame->width,
                           pFrame->height,
                           AV_PIX_FMT_YUV420P,
                           32);
  if (ret < 0) {
    throw std::runtime_error("could not allocate raw picture buffer");
  }
  return pFrame;
}

void video_encode(const char *imgFile) {
  VideoDecoder decoder(imgFile);

  int width = decoder.getWidth();
  int height = decoder.getHeight();

  VideoEncoder::Config encoderConfig = {
    .width = width,
    .height = height,
    .pix_fmt = AV_PIX_FMT_YUV420P,
    .bit_rate = 200000,
    .time_base = (AVRational){1, FPS},
    .gop_size = 25,
    .max_b_frames = 1,
    .codec_id = CODEC_ID,
  };
  VideoEncoder encoder(OUTPUT_FILENAME, encoderConfig);

  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(decoder.getWidth(),
                          decoder.getHeight(),
                          decoder.getPixelFormat(),
                          width,
                          height,
                          AV_PIX_FMT_YUV420P,
                          SWS_BILINEAR,
                          nullptr,
                          nullptr,
                          nullptr);

  AVFrame *pOutputFrame = initFrame(width, height);
  AVFrame *pInputFrame = av_frame_alloc();
  if (!decoder.nextFrame(pInputFrame)) {
    throw std::runtime_error("Expect a frame from input, but got nothing");
  }

  for (int i = 0; i < TOTAL_FRAMES; i++ ) {
    sws_scale(swsCtx,
              (uint8_t const * const *)pInputFrame->data,
              pInputFrame->linesize,
              0,
              height,
              pOutputFrame->data,
              pOutputFrame->linesize);
    pOutputFrame->pts = i;
    encoder.encodeFrame(pOutputFrame);
  }
  encoder.finish();

  av_free(pInputFrame);
  av_free(pOutputFrame);
}

int main(int argc, char **argv) {

  av_register_all();
  if (argc < 2) {
    printf("usage: %s imagefile\n"
           "Example program to encode a video stream from image using libavcoded.\n", argv[0]);
    return -1;
  }
  video_encode(argv[1]);

  return 0;
}

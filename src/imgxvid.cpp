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
const int SECONDS_PER_PIC = 2;
const int WIDTH = 640;
const int HEIGHT = 480;
const char *OUTPUT_FILENAME = "imgxvid.mpg";
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

int encode_image(const char *imgFile,
                 VideoEncoder &encoder,
                 int pts) {
  VideoDecoder decoder(imgFile);
  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(decoder.getWidth(),
                          decoder.getHeight(),
                          decoder.getPixelFormat(),
                          WIDTH,
                          HEIGHT,
                          AV_PIX_FMT_YUV420P,
                          SWS_BILINEAR,
                          nullptr,
                          nullptr,
                          nullptr);

  AVFrame *pOutputFrame = initFrame(WIDTH, HEIGHT);
  AVFrame *pInputFrame = av_frame_alloc();
  if (!decoder.nextFrame(pInputFrame)) {
    throw std::runtime_error("Expect a frame from input, but got nothing");
  }
  for ( int i = 0; i < SECONDS_PER_PIC * FPS; i++ ) {
    sws_scale(swsCtx,
              (uint8_t const * const *)pInputFrame->data,
              pInputFrame->linesize,
              0,
              decoder.getHeight(),
              pOutputFrame->data,
              pOutputFrame->linesize);
    pOutputFrame->pts = pts++;
    encoder.encodeFrame(pOutputFrame);
  }
  av_free(pInputFrame);
  av_free(pOutputFrame);
  return pts;
}

int encode_video(const char *vidFile,
                 VideoEncoder &encoder,
                 int pts) {
  VideoDecoder decoder(vidFile);
  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(decoder.getWidth(),
                          decoder.getHeight(),
                          decoder.getPixelFormat(),
                          WIDTH,
                          HEIGHT,
                          AV_PIX_FMT_YUV420P,
                          SWS_BILINEAR,
                          nullptr,
                          nullptr,
                          nullptr);

  AVFrame *pOutputFrame = initFrame(WIDTH, HEIGHT);
  AVFrame *pInputFrame = av_frame_alloc();
  while (decoder.nextFrame(pInputFrame)) {
    sws_scale(swsCtx,
              (uint8_t const * const *)pInputFrame->data,
              pInputFrame->linesize,
              0,
              decoder.getHeight(),
              pOutputFrame->data,
              pOutputFrame->linesize);
    // TODO what if pts is not increamental from the video source
    pOutputFrame->pts = pts++;
    encoder.encodeFrame(pOutputFrame);
  }
  av_free(pInputFrame);
  av_free(pOutputFrame);
  return pts;
}

void encode(const char *imgFilename, const char *vidFilename) {
  VideoEncoder::Config encoderConfig = {
    .width = WIDTH,
    .height = HEIGHT,
    .pix_fmt = AV_PIX_FMT_YUV420P,
    .bit_rate = 200000,
    .time_base = (AVRational){1, FPS},
    .gop_size = 25,
    .max_b_frames = 1,
    .codec_id = CODEC_ID,
  };
  VideoEncoder encoder(OUTPUT_FILENAME, encoderConfig);
  int pts = encode_image(imgFilename, encoder, 0);
  encode_video(vidFilename, encoder, pts);
  encoder.finish();
}

int main(int argc, char **argv) {

  av_register_all();
  if (argc < 3) {
    printf("usage: %s img vid\n"
           "Example program to encode a video stream from image and video using libavcoded.\n", argv[0]);
    return -1;
  }
  encode(argv[1], argv[2]);

  return 0;
}

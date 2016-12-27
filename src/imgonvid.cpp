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

using namespace video_syn;

const char *OUTPUT_FILENAME = "imgonvid.mpg";
const AVCodecID CODEC_ID = AV_CODEC_ID_MPEG1VIDEO;
const int FPS = 25;

AVFrame *initFrame(int width, int height, AVPixelFormat pixelFormat) {
  AVFrame *pFrame = av_frame_alloc();
  pFrame->format = pixelFormat;
  pFrame->width = width;
  pFrame->height = height;
  int ret = av_image_alloc(pFrame->data,
                           pFrame->linesize,
                           width,
                           height,
                           pixelFormat,
                           32);
  if (ret < 0) {
    throw std::runtime_error("could not allocate raw picture buffer");
  }
  return pFrame;
}

AVFrame *getImageFrame(const char *filename, int width, int height) {
  VideoDecoder decoder(filename);
  AVFrame *srcFrame = av_frame_alloc();
  if (!decoder.nextFrame(srcFrame)) {
    throw std::runtime_error("no frame from the image");
  }

  AVFrame *dstFrame = initFrame(width, height, AV_PIX_FMT_RGB24);
  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(decoder.getWidth(),
                          decoder.getHeight(),
                          decoder.getPixelFormat(),
                          width,
                          height,
                          AV_PIX_FMT_RGB24,
                          SWS_BILINEAR,
                          nullptr,
                          nullptr,
                          nullptr);
  sws_scale(swsCtx,
            (uint8_t const * const *)srcFrame->data,
            srcFrame->linesize,
            0,
            decoder.getHeight(),
            dstFrame->data,
            dstFrame->linesize);
  av_free(srcFrame);
  return dstFrame;
}

void encode(const char *imgFilename, const char *vidFilename) {
  VideoDecoder vidDecoder(vidFilename);
  int width = vidDecoder.getWidth();
  int height = vidDecoder.getHeight();
  int halfWidth = width / 2;
  int halfHeight = height / 2;
  VideoEncoder::Config encoderConfig = {
    .width = width,
    .height = height,
    .pix_fmt = AV_PIX_FMT_YUV420P,
    .bit_rate = 200000,
    .time_base = AVRational{1, FPS},
    .gop_size = 10,
    .max_b_frames = 1,
    .codec_id = CODEC_ID,
  };
  VideoEncoder encoder(OUTPUT_FILENAME, encoderConfig);
  AVFrame *pVidFrame = av_frame_alloc();
  AVFrame *pImgFrame = getImageFrame(imgFilename, halfWidth, halfHeight);
  AVFrame *pRGBFrame = initFrame(width, height, AV_PIX_FMT_RGB24);
  AVFrame *pYUVFrame = initFrame(width, height, AV_PIX_FMT_YUV420P);

  struct SwsContext *swsRGBCtx;
  swsRGBCtx = sws_getContext(vidDecoder.getWidth(),
                             vidDecoder.getHeight(),
                             vidDecoder.getPixelFormat(),
                             width,
                             height,
                             AV_PIX_FMT_RGB24,
                             SWS_BILINEAR,
                             nullptr,
                             nullptr,
                             nullptr);
  struct SwsContext *swsYUVCtx;
  swsYUVCtx = sws_getContext(width,
                             height,
                             AV_PIX_FMT_RGB24,
                             width,
                             height,
                             AV_PIX_FMT_YUV420P,
                             SWS_BILINEAR,
                             nullptr,
                             nullptr,
                             nullptr);

  int pts = 0;
  while (vidDecoder.nextFrame(pVidFrame)) {
    sws_scale(swsRGBCtx,
              (uint8_t const * const *)pVidFrame->data,
              pVidFrame->linesize,
              0,
              height,
              pRGBFrame->data,
              pRGBFrame->linesize);
    int ls1 = pRGBFrame->linesize[0];
    int ls2 = pImgFrame->linesize[0];
    for (int y = 0; y < halfHeight; y++) {
      for (int x = 0; x < halfWidth; x++) {
        for (int rgb = 0; rgb < 3; ++rgb) {
          pRGBFrame->data[0][y * ls1 + x * 3 + rgb] =
            pImgFrame->data[0][y * ls2 + x * 3 + rgb];
        }
      }
    }
    sws_scale(swsYUVCtx,
              (uint8_t const * const *)pRGBFrame->data,
              pRGBFrame->linesize,
              0,
              height,
              pYUVFrame->data,
              pYUVFrame->linesize);
    pYUVFrame->pts = pts++;
    encoder.encodeFrame(pYUVFrame);
  }
  encoder.finish();
  av_free(pVidFrame);
  av_free(pImgFrame);
  av_free(pRGBFrame);
  av_free(pYUVFrame);
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

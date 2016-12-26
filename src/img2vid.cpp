#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstdint>

#include "VideoDecoder.h"

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

static uint8_t endcode[] = { 0, 0, 1, 0xb7 };

using namespace video_syn;

void video_encode(const char *imgFile) {
  VideoDecoder decoder(imgFile);

  AVCodecContext *pOutputCodecCtx = nullptr;
  AVCodec *pOutputCodec = nullptr;
  AVFrame *pOutputFrame = av_frame_alloc();

  pOutputCodecCtx = avcodec_alloc_context3(pOutputCodec);
  if (!pOutputCodecCtx) {
    std::cerr << "Could not allocate video codec context" << std::endl;
    exit(-1);
  }
  pOutputCodecCtx->width = decoder.getWidth();
  pOutputCodecCtx->height = decoder.getHeight();

  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(decoder.getWidth(),
                          decoder.getHeight(),
                          decoder.getPixelFormat(),
                          pOutputCodecCtx->width,
                          pOutputCodecCtx->height,
                          AV_PIX_FMT_YUV420P,
                          SWS_BILINEAR,
                          nullptr,
                          nullptr,
                          nullptr);
  int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P,
                                    pOutputCodecCtx->width,
                                    pOutputCodecCtx->height);
  uint8_t *buffer = (uint8_t *)av_malloc( numBytes * sizeof(uint8_t) );
  avpicture_fill((AVPicture *)pOutputFrame,
                 buffer,
                 AV_PIX_FMT_YUV420P,
		 pOutputCodecCtx->width,
                 pOutputCodecCtx->height);

  pOutputCodec = avcodec_find_encoder(CODEC_ID);
  if (!pOutputCodec) {
    std::cerr << "Codec not found" << std::endl;
    exit(-1);
  }

  pOutputCodecCtx->bit_rate = 200000;

  pOutputCodecCtx->time_base = (AVRational){1, FPS};
  pOutputCodecCtx->gop_size = 25;
  pOutputCodecCtx->max_b_frames = 1;
  pOutputCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

  if (avcodec_open2(pOutputCodecCtx, pOutputCodec, NULL) < 0) {
    std::cerr << "Could not open output codec" << std::endl;
    exit(-1);
  }

  FILE *f = fopen(OUTPUT_FILENAME, "wb");
  if (!f) {
    std::cerr << "Could not open file " << OUTPUT_FILENAME << std::endl;
    exit(-1);
  }

  pOutputFrame->width = pOutputCodecCtx->width;
  pOutputFrame->height = pOutputCodecCtx->height;
  pOutputFrame->format = pOutputCodecCtx->pix_fmt;

  int got_output;
  int i;

  AVPacket pkt;

  AVFrame *pInputFrame = av_frame_alloc();
  if (!decoder.nextFrame(pInputFrame)) {
    std::cerr << "Expect a frame from input, but got nothing" << std::endl;
    exit(-1);
  }

  for (i = 0; i < TOTAL_FRAMES; i++ ) {
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    fflush(stdout);

    sws_scale(swsCtx,
              (uint8_t const * const *)pInputFrame->data,
              pInputFrame->linesize,
              0,
              pOutputCodecCtx->height,
              pOutputFrame->data,
              pOutputFrame->linesize);
    pOutputFrame->pts = i;
    int ret = avcodec_encode_video2(pOutputCodecCtx,
                                    &pkt,
                                    pOutputFrame,
                                    &got_output);
    if (ret < 0) {
      std::cerr<< "Error encoding frame" << std::endl;
      exit(-1);
    }
    if (got_output) {
      //      printf("Write frame %3d (size=%5d)\n", i, pkt.size);
      fwrite(pkt.data, 1, pkt.size, f);
      av_packet_unref(&pkt);
    }
  }

  for (got_output = 1; got_output; i++) {
    fflush(stdout);

    int ret = avcodec_encode_video2(pOutputCodecCtx,
                                    &pkt,
                                    NULL,
                                    &got_output);
    if (ret < 0) {
      std::cerr << "Error encoding frame" << std::endl;
      exit(-1);
    }

    if (got_output) {
      //      printf("Write frame %3d (size=%5d)\n", i, pkt.size);
      fwrite(pkt.data, 1, pkt.size, f);
      av_packet_unref(&pkt);
    }
  }

  fwrite(endcode, 1, sizeof(endcode), f);
  fclose(f);

  av_free(buffer);
  avcodec_close(pOutputCodecCtx);
  av_free(pOutputCodecCtx);
  av_free(pInputFrame);
  av_free(pOutputFrame);
}

int main(int argc, char **argv) {

  av_register_all();
  if (argc < 2) {
    printf("usage: %s imagefile\n"
           "Example program to encode a video stream from image using libavcoded.\n", argv[0]);
    return 1;
  }
  video_encode(argv[1]);

  return 0;
}

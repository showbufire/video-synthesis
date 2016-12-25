#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstdint>

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

void video_encode(const char *imgFile) {
  AVFormatContext *pFormatCtx = nullptr;
  if (avformat_open_input(&pFormatCtx, imgFile, nullptr, nullptr) != 0) {
    std::cerr << "Could not open the image file: " << imgFile << std::endl;
    exit(-1);
  }
  if (avformat_find_stream_info(pFormatCtx, nullptr)<0) {
    std::cerr << "Could not find stream information " << std::endl;
    exit(-1);
  }
  av_dump_format(pFormatCtx, 0, imgFile, 0);

  AVCodecContext *pInputCodecCtx = nullptr;
  AVCodecContext *pOutputCodecCtx = nullptr;
  AVCodec *pInputCodec = nullptr;
  AVCodec *pOutputCodec = nullptr;
  AVFrame *pInputFrame = av_frame_alloc();
  AVFrame *pOutputFrame = av_frame_alloc();
  AVPacket packet;

  // image has one stream
  pInputCodecCtx = pFormatCtx->streams[0]->codec;
  pInputCodec = avcodec_find_decoder(pInputCodecCtx->codec_id);
  if (pInputCodec == nullptr) {
    std::cerr << "Unsuported codec!" << std::endl;
    exit(-1);
  }
  if (avcodec_open2(pInputCodecCtx, pInputCodec, NULL) < 0) {
    std::cerr << "Could not open input codec" << std::endl;
    exit(-1);
  }

  pOutputCodecCtx = avcodec_alloc_context3(pOutputCodec);
  if (!pOutputCodecCtx) {
    std::cerr << "Could not allocate video codec context" << std::endl;
    exit(-1);
  }
  pOutputCodecCtx->width = pInputCodecCtx->width;
  pOutputCodecCtx->height = pInputCodecCtx->height;

  struct SwsContext *swsCtx;
  swsCtx = sws_getContext(pInputCodecCtx->width,
                          pInputCodecCtx->height,
                          pInputCodecCtx->pix_fmt,
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

  int inputNumBytes = avpicture_get_size(pInputCodecCtx->pix_fmt,
                                         pInputCodecCtx->width,
                                         pInputCodecCtx->height);
  uint8_t *inputBuffer = (uint8_t *)av_malloc(inputNumBytes * sizeof(uint8_t) );
  avpicture_fill((AVPicture *)pInputFrame,
                 inputBuffer,
                 pInputCodecCtx->pix_fmt,
		 pOutputCodecCtx->width,
                 pOutputCodecCtx->height);

  while (av_read_frame(pFormatCtx, &packet) >= 0) {
    int frameFinished;
    int len = avcodec_decode_video2(pInputCodecCtx,
                                    pInputFrame,
                                    &frameFinished,
                                    &packet);
    if (len < 0) {
      std::cerr << "Error while decoding frame: " << av_err2str(len) << std::endl;
      exit(-1);
    }
    if (frameFinished) {
      break;
    }
  }
  //  av_packet_unref(&packet);

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

  av_free(inputBuffer);
  av_free(buffer);
  avcodec_close(pInputCodecCtx);
  av_free(pInputCodecCtx);
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

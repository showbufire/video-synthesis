#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstdint>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/common.h>
}

const int FPS = 25;
const int TIME_LENGTH = 4; // seconds;
const int TOTAL_FRAMES = FPS * TIME_LENGTH;
const int WIDTH = 640;
const int HEIGHT = 480;
const char *OUTPUT_FILENAME = "img.mpg";

const AVCodecID CODEC_ID = AV_CODEC_ID_MPEG1VIDEO;

static uint8_t endcode[] = { 0, 0, 1, 0xb7 };

void video_encode(const char *imgFile) {
  AVCodec *codec;
  AVCodecContext *c = NULL;

  codec = avcodec_find_encoder(CODEC_ID);
  if (!codec) {
    std::cerr << "Codec not found" << std::endl;
    exit(1);
  }

  c = avcodec_alloc_context3(codec);
  if (!c) {
    std::cerr << "Could not allocate video codec context" << std::endl;
    exit(1);
  }

  c->bit_rate = 200000;
  c->width = WIDTH;
  c->height = HEIGHT;

  c->time_base = (AVRational){1, FPS};
  c->gop_size = 25;
  c->max_b_frames = 1;
  c->pix_fmt = AV_PIX_FMT_YUV420P;

  if (avcodec_open2(c, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }
  AVFrame *frame;
  frame = av_frame_alloc();
  if (!frame) {
    std::cerr << "Could not allocate video frame" << std::endl;
    exit(1);
  }
  frame->format = c->pix_fmt;
  frame->width = c->width;
  frame->height = c->height;

  int ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
  if (ret < 0) {
    std::cerr << "Could not allocate raw picture buffer\n" << std::endl;
    exit(1);
  }

  FILE *f = fopen(OUTPUT_FILENAME, "wb");
  if (!f) {
    std::cerr << "Could not open file " << OUTPUT_FILENAME << std::endl;
    exit(1);
  }

  AVPacket pkt;
  int got_output;
  int i;
  for (i = 0; i < TOTAL_FRAMES; i++ ) {
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    fflush(stdout);

    for (int y = 0; y < c->height; y++) {
      for (int x = 0; x < c->width; x++) {
        frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
      }
    }

    /* Cb and Cr */
    for (int y = 0; y < c->height/2; y++) {
      for (int x = 0; x < c->width/2; x++) {
        frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
        frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
      }
    }

    frame->pts = i;
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
      std::cerr<< "Error encoding frame" << std::endl;
      exit(1);
    }
    if (got_output) {
      printf("Write frame %3d (size=%5d)\n", i, pkt.size);
      fwrite(pkt.data, 1, pkt.size, f);
      av_packet_unref(&pkt);
    }
  }

  for (got_output = 1; got_output; i++) {
    fflush(stdout);

    ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
    if (ret < 0) {
      std::cerr << "Error encoding frame" << std::endl;
      exit(1);
    }

    if (got_output) {
      printf("Write frame %3d (size=%5d)\n", i, pkt.size);
      fwrite(pkt.data, 1, pkt.size, f);
      av_packet_unref(&pkt);
    }
  }

  fwrite(endcode, 1, sizeof(endcode), f);
  fclose(f);

  avcodec_close(c);
  av_free(c);
  av_freep(&frame->data[0]);
  av_frame_free(&frame);
  printf("\n");
}

int main(int argc, char **argv) {

  avcodec_register_all();
  if (argc < 2) {
    printf("usage: %s imagefile\n"
           "Example program to encode a video stream from image using libavcoded.\n", argv[0]);
    return 1;
  }
  video_encode(argv[1]);

  return 0;
}

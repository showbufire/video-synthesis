#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstdint>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/common.h>
}

void read_image(const char *imgFile) {
  AVFormatContext *pFormatCtx;
  if (avformat_open_input(&pFormatCtx, imgFile, nullptr, nullptr) != 0) {
    std::cerr << "Could not open the image file: " << imgFile << std::endl;
    exit(1);
  }
  if (avformat_find_stream_info(pFormatCtx, nullptr)<0) {
    std::cerr << "Could not find stream information " << std::endl;
    exit(1);
  }
  av_dump_format(pFormatCtx, 0, imgFile, 0);

  avformat_close_input(&pFormatCtx);
}

int main(int argc, char **argv) {
  av_register_all();
  if (argc < 2) {
    printf("usage: %s imagefile\n"
           "Example program to read an image and dump its information");
    return 1;
  }
  read_image(argv[1]);
  return 0;
}

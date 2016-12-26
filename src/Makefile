FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \

CXXFLAGS += -Wall -g -std=c++11
CXXFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CXXFLAGS)
LDLIBS := $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LDLIBS)
CC = g++

BINS =	img2vid		\
	imgimg		\
	imgxvid		\

OBJS =  $(addsuffix .o,$(BINS)) \
	VideoEncoder.o  \
	VideoDecoder.o  \

OUTPUTS=$(addsuffix .mpg,$(BINS))

.phony: all clean clean-output

all: $(BINS)

img2vid: img2vid.o VideoDecoder.o VideoEncoder.o

img2vid.o: img2vid.cpp

imgimg: imgimg.o VideoDecoder.o VideoEncoder.o

imgimg.o: imgimg.cpp

imgxvid: imgxvid.o VideoDecoder.o VideoEncoder.o
imgxvid.o: imgxvid.cpp

VideoEncoder.o: VideoEncoder.h VideoEncoder.cpp

VideoDecoder.o: VideoDecoder.h VideoDecoder.cpp

clean-output:
	$(RM) $(OUTPUTS)

clean: clean-output
	$(RM) $(BINS) $(OBJS)

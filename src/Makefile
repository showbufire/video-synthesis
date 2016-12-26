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

LIBS =  VideoDecoder VideoEncoder

BINS =	img2vid		\

OBJS=$(addsuffix .o,$(LIBS)) $(addsuffix .o,$(BINS))
OUTPUTS=$(addsuffix .mpg,$(BINS))

.phony: all clean clean-output

all: $(BINS)

$(BINS): $(OBJS)

clean-output:
	$(RM) $(OUTPUTS)

clean: clean-output
	$(RM) $(BINS) $(OBJS)

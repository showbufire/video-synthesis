FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \

CFLAGS += -Wall -g
CFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
LDLIBS := $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LDLIBS)
CC = g++

.phony: all clean

EXAMPLES =	img2vid		\
		readimg		\

OBJS=$(addsuffix .o,$(EXAMPLES))
OUTPUTS=$(addsuffix .mpg,$(EXAMPLES))

all: $(OBJS) $(EXAMPLES)

clean-output:
	$(RM) $(OUTPUTS)

clean: clean-output
	$(RM) $(EXAMPLES) $(OBJS)

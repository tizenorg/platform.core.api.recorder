CC ?= gcc

C_FILES = $(shell ls *.c)

PKGS = capi-media-camera capi-media-recorder dlog glib-2.0 gthread-2.0

LDFLAGS = `pkg-config --libs $(PKGS)`
LDFLAGS += $(TET_ROOT)/lib/tet3/tcm_s.o
LDFLAGS += -L$(TET_ROOT)/lib/tet3 -ltcm_s
LDFLAGS += -L$(TET_ROOT)/lib/tet3 -lapi_s

CFLAGS = -I. `pkg-config --cflags $(PKGS)`
CFLAGS += -I$(TET_ROOT)/inc/tet3
CFLAGS += -Wall -Werror

#TARGETS = $(C_FILES:%.c=tc-%)
TCS := $(shell ls -1 *.c | cut -d. -f1)

all: $(TCS)

%: %.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TCS)

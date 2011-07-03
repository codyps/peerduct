CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
RM = rm -f

MAKEFLAGS += -Rr --no-print-directory

ifndef V
	QUIET_CC = @ echo '    ' CC $@;
	QUIET_LD = @ echo '    ' LD $@;
endif

.PHONY: all
all: build


YAJL_DIR        = yajl
YAJL_BUILDDIR   = $(YAJL_DIR)/build/yajl-2.0.3
YAJL_LIBDIR     = $(YAJL_BUILDDIR)/lib
YAJL_INCDIR     = $(YAJL_BUILDDIR)/include
YAJL_LIB        = $(YAJL_LIBDIR)/libyajl_s.a

YAJL_LDFLAGS    = $(YAJL_LIB)
YAJL_CFLAGS     = -I$(YAJL_INCDIR)

TARGETS = pd nd_reader test_list nnode
pd: main.c.o
nd_reader: nd_reader.c.o nodes_dat.c.o peer.c.o
test_list: list_test.c.o

cfg_json.c.o : $(YAJL_LIB)

nnode: nnode.c.o cfg_json.c.o
nnode: CFLAGS  += $(YAJL_CFLAGS)
nnode: LDFLAGS += $(YAJL_LDFLAGS) -lev

$(YAJL_LIB) :
	cd $(YAJL_DIR) && ./configure && $(MAKE)

srcdir = .
VPATH = $(srcdir)


CFLAGS          += -ggdb
override CFLAGS += -Wall -pipe
LDFLAGS         += -Wl,--as-needed -O2

.PHONY: rebuild
rebuild: | clean build 

.PHONY: build
build: $(TARGETS)

%.c.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(TARGETS) : 
	$(QUIET_LD)$(LD) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGETS) *.d *.o
	$(MAKE) -C $(YAJL_DIR) clean



-include $(wildcard *.d)

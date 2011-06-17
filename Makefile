CC = gcc
RM = rm -f

.PHONY: all
all: build


TARGETS = pd nd_reader test_list
pd: main.c.o
nd_reader: nd_reader.c.o nodes_dat.c.o peer.c.o
test_list: list_test.c.o

srcdir = .
VPATH = $(srcdir)


CFLAGS = -ggdb
override CFLAGS += -Wall -pipe
LDFLAGS = -Wl,--as-needed -O2

.PHONY: rebuild
rebuild: | clean build 

.PHONY: build
build: $(TARGETS)

%.c.o : %.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(TARGETS) : 
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(TARGETS) *.d *.o

-include $(wildcard *.d)

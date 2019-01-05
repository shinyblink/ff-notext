CC ?= cc
CFLAGS ?= -Os
CPPFLAGS += -pedantic -Wall -Wextra #-fsanitize=address,undefined

DESTDIR ?= /usr/local

BINS=ff-notext
all: $(BINS)

ff-notext: ff-notext.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags --libs tesseract) $(LDFLAGS) -o ff-notext $^

install: $(BINS)
	install $(BINS) $(DESTDIR)/bin

clean:
	rm -f $(BINS) *.o

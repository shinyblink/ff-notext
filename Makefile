CC ?= cc
CFLAGS ?= -Os
CPPFLAGS += -pedantic -Wall -Wextra #-fsanitize=address,undefined

PREFIX ?= /usr/local
DESTDIR ?= /

BINS=ff-notext
all: $(BINS)

ff-notext: ff-notext.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags --libs tesseract) $(LDFLAGS) -o ff-notext $^

install: $(BINS)
	install -d $(DESTDIR)/$(PREFIX)/bin
	install $(BINS) $(DESTDIR)/$(PREFIX)/bin

uninstall:
	cd $(DESTDIR)/$(PREFIX)/bin && rm $(BINS)

clean:
	rm -f $(BINS) *.o

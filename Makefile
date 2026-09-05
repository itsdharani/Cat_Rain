PREFIX ?= /usr
CC ?= cc
CFLAGS += -O2 -Wall

catrain: catrain.c
	$(CC) $(CFLAGS) -o catrain catrain.c -lncursesw

install: catrain
	install -Dm755 catrain $(DESTDIR)$(PREFIX)/bin/catrain

clean:
	rm -f catrain

.PHONY: install clean


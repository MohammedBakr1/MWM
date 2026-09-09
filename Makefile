CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -O2 -I/usr/include/X11
LDFLAGS = -lX11

SRC = src/mwm.c src/config.c src/client.c src/layout.c src/actions.c
OBJ = $(SRC:.c=.o)
BIN = mwm

PREFIX ?= /usr/local

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(BIN)
	install -Dm755 $(BIN) $(PREFIX)/bin/$(BIN)
	install -Dm644 mwmrc.default $(DESTDIR)/etc/xdg/mwm/mwmrc.default
	@mkdir -p $(DESTDIR)$(HOME)/.config/mwm
	@[ -f $(DESTDIR)$(HOME)/.config/mwm/mwmrc ] || cp mwmrc.default $(DESTDIR)$(HOME)/.config/mwm/mwmrc

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all install clean

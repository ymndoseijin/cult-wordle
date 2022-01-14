.POSIX:

include config.mk

SRC = wordle.c
OBJ = $(SRC:.c=.o)

all: options cult-wordle

options:
	@echo cult-wordle build options:
	@echo "CFLAGS  = $(STCFLAGS)"
	@echo "LDFLAGS = $(STLDFLAGS)"
	@echo "CC      = $(CC)"

.c.o:
	$(CC) $(STCFLAGS) -c $<


cult-wordle: $(OBJ)
	$(CC) -o $@ $(OBJ) $(STLDFLAGS)

clean:
	rm -f cult-wordle $(OBJ)

install: cult-wordle
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f cult-wordle $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/wordle-clone

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/wordle-clone

.PHONY: all options clean dist install uninstall

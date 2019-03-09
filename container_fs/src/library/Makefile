CFLAGS := -m64 -O2 -g -D_GNU_SOURCE -D_REENTRANT -W -I/usr/local/include
LDFLAGS := -m64 -lm

all: fcontainer.c
	$(CC) $(CFLAGS) -Wall -fPIC -c fcontainer.c
	$(CC) $(CFLAGS) -shared -Wl,-soname,libfcontainer.so.1 -o libfcontainer.so.1.0 fcontainer.o

install: libfcontainer.so.1.0
	cp libfcontainer.so.1.0 /usr/lib/libfcontainer.so.1
	ln -fs /usr/lib/libfcontainer.so.1 /usr/lib/libfcontainer.so
	cp fcontainer.h  /usr/local/include


clean:
	rm -f *.so *.o *.1.0


.PHONY: all clean

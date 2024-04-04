
LDFLAGS = -lunwind -lunwind-$(shell uname -m)

# SRC = $(wildcard *.c)
# OBJ = $(SRC:.c=.o)

all:libmaze.so libsolver.so
%.o: %.c
	gcc -c -Wall -fPIC $< -o $@

libmaze.so: libmaze_dummy.o
	gcc -shared -Wall -o $@ $^ $(LDFLAGS)

libsolver.so: libsolver.o
	gcc -shared -Wall -o $@ $^ $(LDFLAGS)

clean:
	rm *.so *.o
# //maze

test:
	LD_LIBRARY_PATH=$(shell pwd) LD_PRELOAD=libsolver.so ./maze
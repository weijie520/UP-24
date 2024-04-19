LDFLAGS = -ldl

all: logger logger.so

%.o: %.c
	gcc -c -Wall -fPIC $< -o $@

logger: main.o
	gcc -Wall -o $@ $^ $(LDFLAGS)

logger.so: logger.o
	gcc -shared -Wall -o $@ $^ $(LDFLAGS)

clean:
	rm -f logger logger.so *.o *.log
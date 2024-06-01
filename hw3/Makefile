all: sdb

sdb: sdb.cpp
	g++ -Wall -o $@ $^ -lcapstone

test: sdb
	cp sdb test/
	${MAKE} -C test/

test-clean:
	${MAKE} clean -C test/

clean: test-clean
	rm -f sdb *.o
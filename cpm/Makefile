CC = ../neo-c
INSTALL ?= install
DESTDIR ?= /usr/local
CFLAGS ?= -O2
CPPFLAGS ?= -DCPM_DEFAULT_STDLIB_DIR=\"$(abspath ..)\"
CPM_BIN := $(CURDIR)/cpm
NEOC_BIN := $(abspath ../neo-c)

.PHONY: all install clean test

all: cpm

cpm: cpm.nc
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<

install: cpm
	mkdir -p "$(DESTDIR)/bin"
	$(INSTALL) -m 755 ./cpm "$(DESTDIR)/bin/cpm"

test: cpm
	rm -rf /tmp/neo-cpm-test
	mkdir -p /tmp/neo-cpm-test
	cd /tmp/neo-cpm-test && "$(CPM_BIN)" new hello
	test -f /tmp/neo-cpm-test/hello/lib/neo-c.h
	test -f /tmp/neo-cpm-test/hello/lib/neo-c-str.nc
	test -f /tmp/neo-cpm-test/hello/src/common.h
	printf '#ifndef CPM_COMMON_H\n#define CPM_COMMON_H\n\n#include <neo-c.h>\n\nstring fun();\nint add(int a, int b);\n\n#endif\n' > /tmp/neo-cpm-test/hello/src/common.h
	printf '#include <neo-c.h>\n\nstring fun()\n{\n    return s"Hello from neo-c cpm";\n}\n\nint add(int a, int b)\n{\n    return a + b;\n}\n' > /tmp/neo-cpm-test/hello/src/add.nc
	printf '#include <neo-c.h>\n\nint main(int argc, char** argv)\n{\n    if(add(20, 22) != 42) {\n        return 1;\n    }\n    puts(fun());\n    return 0;\n}\n' > /tmp/neo-cpm-test/hello/src/main.nc
	cd /tmp/neo-cpm-test/hello && CPM_NEOC="$(NEOC_BIN)" "$(CPM_BIN)" build
	test -f /tmp/neo-cpm-test/hello/target/debug/hello.c
	test -f /tmp/neo-cpm-test/hello/target/debug/src_add.c
	test -f /tmp/neo-cpm-test/hello/target/debug/neo-c-str.c
	test -f /tmp/neo-cpm-test/hello/target/debug/neo-c-str.o
	test ! -f /tmp/neo-cpm-test/hello/main.c
	test ! -f /tmp/neo-cpm-test/hello/neo-c-str.c
	cd /tmp/neo-cpm-test/hello && ./target/debug/hello > test.out
	grep -q '^Hello from neo-c cpm$$' /tmp/neo-cpm-test/hello/test.out

clean:
	rm -f cpm cpm.c cpm.nc.i *.o *.out

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
	printf '#include <neo-c.h>\n\ntypedef int MyInt;\n\ntypedef struct Pair {\n    int a;\n    int b;\n} Pair;\n\nunion Cell {\n    int i;\n    long l;\n};\n\nenum Kind {\n    KIND_OK = 1\n};\n\nstring fun()\n{\n    return s"Hello from neo-c cpm";\n}\n\nMyInt add(MyInt a, MyInt b)\n{\n    return a + b;\n}\n' > /tmp/neo-cpm-test/hello/src/add.nc
	printf '#include <neo-c.h>\n\nint main(int argc, char** argv)\n{\n    Pair p;\n    union Cell cell;\n    enum Kind kind;\n    p.a = 20;\n    p.b = 22;\n    cell.i = 1;\n    kind = KIND_OK;\n    if(kind != KIND_OK || cell.i != 1 || add(p.a, p.b) != 42) {\n        return 1;\n    }\n    puts(fun());\n    return 0;\n}\n' > /tmp/neo-cpm-test/hello/src/main.nc
	cd /tmp/neo-cpm-test/hello && CPM_NEOC="$(NEOC_BIN)" "$(CPM_BIN)" build
	grep -q '^typedef int MyInt;$$' /tmp/neo-cpm-test/hello/src/common.h
	grep -q '^typedef struct Pair' /tmp/neo-cpm-test/hello/src/common.h
	grep -q '^union Cell' /tmp/neo-cpm-test/hello/src/common.h
	grep -q '^enum Kind' /tmp/neo-cpm-test/hello/src/common.h
	grep -q '^string fun();$$' /tmp/neo-cpm-test/hello/src/common.h
	grep -q '^MyInt add(MyInt a, MyInt b);$$' /tmp/neo-cpm-test/hello/src/common.h
	test -f /tmp/neo-cpm-test/hello/target/debug/hello.c
	test -f /tmp/neo-cpm-test/hello/target/debug/src_add.c
	test -f /tmp/neo-cpm-test/hello/target/debug/neo-c-str.c
	test -f /tmp/neo-cpm-test/hello/target/debug/neo-c-str.o
	test ! -f /tmp/neo-cpm-test/hello/main.c
	test ! -f /tmp/neo-cpm-test/hello/neo-c-str.c
	test ! -f /tmp/neo-cpm-test/hello/hello.o
	test ! -f /tmp/neo-cpm-test/hello/src_add.o
	test ! -f /tmp/neo-cpm-test/hello/neo-c-str.o
	cd /tmp/neo-cpm-test/hello && ./target/debug/hello > test.out
	grep -q '^Hello from neo-c cpm$$' /tmp/neo-cpm-test/hello/test.out

clean:
	rm -f cpm cpm.c cpm.nc.i *.o *.out

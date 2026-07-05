# cpm

`cpm` is a small Cargo-like package manager for neo-c projects.

```sh
cpm new hello
cd hello
cpm build
cpm run
cpm test
cpm val
cpm leak
cpm clean
```

`cpm new hello` creates:

```text
hello/
  Neo.toml
  lib/neo-c.h
  lib/neo-c-str.nc
  lib/neo-c-str.h
  lib/neo-c-libc.h
  lib/neo-c-net.h
  lib/neo-c-pthread.h
  src/common.h
  src/main.nc
  .gitignore
```

The manifest format is intentionally small:

```toml
[package]
name = "hello"
version = "1.0.0"

[build]
src = "src/main.nc"
out = "target/debug/hello"
neoc = "neo-c"
neoc_flags = "-I. -Ilib"
ldflags = ""
jobs = 0
lowmem = false
strip = false
```

`cpm build` compiles every `.nc` file under `src` to generated C and object
files under `target/debug`, then links them into `target/debug/<package-name>`.
`cpm build` regenerates `src/common.h` from top-level `typedef`, `struct`,
`union`, `enum`, and function definitions in `src/*.nc`, then automatically
includes it at the `.nc` source stage before each `src/*.nc` file. This lets
project sources use types and functions defined in other source files without
hand-written prototypes. Put manual shared declarations in a separate header
and include that from source files.
Set `[build] common_header = false` for projects that already manage their own
cross-file declarations or cannot safely include a generated common header.
Transpile/compile jobs run in parallel by default, using the number of online
CPUs. The final link step stays serial. Set `[build] jobs = 1` or
`CPM_JOBS=1` to force serial builds, or set a larger value to choose the
parallelism explicitly:

```sh
CPM_JOBS=4 cpm build
```

If two source files would emit the same generated C basename, cpm falls back to
the serial build path to avoid output-file collisions.
`strip` defaults to `false`; set `strip = true` only when you want cpm to strip
the final binary after linking.

For low-memory systems such as 512MB machines, use lowmem mode. It disables
parallel jobs and passes `-lowmem` to neo-c:

```sh
CPM_LOWMEM=1 cpm build
```

You can also make it project-local:

```toml
[build]
lowmem = true
```

The top-level neo-c compiler also has `Neo.toml`. From the repository root,
build it through cpm with:

```sh
make cpm-build-ncc
```

The equivalent direct command is:

```sh
make -C cpm
CPM_NEOC=./neo-c cpm/cpm build
```

This writes generated C, objects, and `target/debug/ncc` under `target/debug`
without replacing the checked-in self-host C sources in the repository root.

For projects that keep sources outside `src/` or need an exact link set, use
`sources`:

```toml
[build]
src = "src/main.nc"
sources = "src/main.nc src/file1.nc src/file2.nc"
out = "target/debug/app"
```

`sources` is a space- or comma-separated list. When it is present, `cpm build`
uses only those files and preserves their order.
If an existing generated C file with the same basename is present in the project
root, cpm temporarily saves it, moves the new generated C into `target/debug`,
and restores the original file. This lets legacy self-host trees keep checked-in
generated C while still using cpm.

The neo-c standard library source files are copied into each project under
`lib/`. `cpm build` compiles `lib/neo-c-str.nc` into
`target/debug/neo-c-str.c` and `target/debug/neo-c-str.o`, then links that
project-local object. If `neo-c-str.nc` is listed explicitly in `sources`, cpm
compiles that source with `-uniq` and does not add another runtime object.

`cpm install` builds the package and installs the output to `$DESTDIR/bin`.
`DESTDIR` defaults to `/usr/local`.

Set `CPM_NEOC=/path/to/neo-c` to override the compiler at runtime. Set
`CPM_STDLIB_DIR=/path/to/neo-c` if `cpm new` cannot find the standard library
sources. `cpm val` uses Valgrind. `cpm leak` builds with sanitizer flags and
falls back to Valgrind if the sanitizer path fails.

For libc-free tiny Linux binaries, `Neo.toml` can select a bare build path:

```toml
[build]
src = "src/main.nc"
out = "target/debug/small"
neoc = "neo-c"
neoc_flags = ""
bare = true
cc = "clang"
cflags = "-target x86_64-linux-gnu -Oz -ffreestanding -fno-asynchronous-unwind-tables -fno-ident -fno-stack-protector -fno-unwind-tables -nostdlib"
linker = "ld"
linker_flags = "-nostdlib -static -n --build-id=none"
linker_script = "small.ld"
ldflags = ""
strip = true
strip_sections = true
```

With `bare = true`, `cpm build` runs `neo-c -bare -S`, compiles the generated C
with `cc`/`cflags`, links with `linker`, and uses `strip --strip-section-headers`
when `strip_sections = true`.

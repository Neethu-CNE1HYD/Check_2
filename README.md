# Check_2 - nav_service crash demo

Source side of a test for the Runtime Issues Analyzer. `nav_service` crashes on purpose in ten different
ways, selected by the first command-line argument. `libweather.so` is a small shared library used by mode `lib`.
Every crashing line carries a `CRASH <mode>` comment. This README describes tag `v1.1.0` (release build).

## Build

Build from the repository root (DWARF paths stay repository-relative, e.g. `src/nav_service.c`).
Binaries are never stripped and carry `--build-id=sha1`.

```sh
./build.sh debug   /abs/out     # -O0 -g                  -> /abs/out/bin/nav_service, /abs/out/lib/libweather.so
./build.sh release /abs/out2    # -O2 -g -fomit-frame-pointer -fasynchronous-unwind-tables
```

`CC` and `CXX` select the compiler (defaults `cc`/`c++`), for example
`CC="python -m ziglang cc -target x86_64-linux-gnu.2.17" CXX="python -m ziglang c++ -target x86_64-linux-gnu.2.17"`.
Tag `v1.0.0` is meant for the debug build, `v1.1.0` for the release build.

## Crash

```sh
ulimit -c unlimited
./out/bin/nav_service <mode>      # null stack abort assert div0 doublefree smash throw rodata lib
```

`libweather.so` is found through the `$ORIGIN/../lib` rpath.

## Expected crash location (v1.1.0)

| mode | file | line | function | signal | notes |
|---|---|---|---|---|---|
| `null` | `src/nav_service.c` | 21 | `crash_null` | SIGSEGV |  |
| `stack` | `src/nav_service.c` | 28 | `crash_stack (recursive)` | SIGSEGV | thousands of identical crash_stack frames |
| `abort` | `src/nav_service.c` | 33 | `crash_abort` | SIGABRT | user frame is the caller of abort() |
| `assert` | `src/nav_service.c` | 38 | `crash_assert` | SIGABRT | user frame is the caller of __assert_fail |
| `div0` | `src/nav_service.c` | 44 | `crash_div0` | SIGFPE |  |
| `doublefree` | `src/nav_service.c` | 51 | `crash_doublefree` | SIGABRT | user frame is the caller of free() |
| `smash` | `src/nav_service.c` | 57 | `crash_smash` | SIGABRT | user frame is the caller of __stack_chk_fail |
| `throw` | `src/nav_throw.cpp` | 5 | `crash_throw` | SIGABRT | user frame is the caller of __cxa_throw / terminate |
| `rodata` | `src/nav_service.c` | 64 | `crash_rodata` | SIGSEGV |  |
| `lib` | `src/libweather.c` | 5 | `weather_store` | SIGSEGV | chain: weather_store <- weather_update <- crash_lib <- main |

For SIGABRT modes a real core's frame #0 is inside libc (`raise`/`abort`); the first frame in this
repository is the one listed above. Frame `main` follows at `src/nav_service.c` in `main()`.

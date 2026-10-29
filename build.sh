#!/bin/sh
# Usage: ./build.sh debug|release <out-dir>
# Run from the repository root so DWARF paths stay repository-relative (src/...).
# Cross-compile with zig:
#   CC="python -m ziglang cc -target x86_64-linux-gnu.2.17" \
#   CXX="python -m ziglang c++ -target x86_64-linux-gnu.2.17" ./build.sh debug /abs/out
set -e
MODE=${1:?debug|release}
OUT=${2:?out dir}
CC=${CC:-cc}
CXX=${CXX:-c++}

case "$MODE" in
  debug)   OPT="-O0" ;;
  release) OPT="-O2 -fomit-frame-pointer -fasynchronous-unwind-tables" ;;
  *) echo "mode must be debug or release" >&2; exit 2 ;;
esac
COMMON="-g -Iinclude -Wall -fno-sanitize=undefined -fdebug-prefix-map=$(pwd)=. $OPT"
LD="-Wl,--build-id=sha1"

mkdir -p "$OUT/bin" "$OUT/lib"
python tools/gen_route.py build/gen
COMMON="$COMMON -Ibuild/gen"
$CC $COMMON -fPIC -shared -Wl,-soname,libweather.so $LD src/libweather.c -o "$OUT/lib/libweather.so"
$CXX $COMMON -c src/nav_throw.cpp -o "$OUT/nav_throw.o"
$CC $COMMON -fstack-protector-all -c src/nav_service.c -o "$OUT/nav_service.o"
$CC $COMMON -c build/gen/route_table.c -o "$OUT/route_table.o"
$CXX "$OUT/nav_service.o" "$OUT/nav_throw.o" "$OUT/route_table.o" $LD -L"$OUT/lib" -lweather \
  -Wl,-rpath,'$ORIGIN/../lib' -o "$OUT/bin/nav_service"
rm -f "$OUT"/*.o

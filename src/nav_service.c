#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libweather.h"
#include "route_table.h" /* generated into build/gen by tools/gen_route.py */

void crash_throw(void); /* src/nav_throw.cpp */

static void crash_null(void)
{
    volatile int *p = NULL;
    *p = 1; /* CRASH null: NULL pointer write, SIGSEGV */
}

static int crash_stack(int depth)
{
    volatile char pad[256];
    pad[0] = (char)depth;
    return crash_stack(depth + 1) + pad[0]; /* CRASH stack: unbounded recursion, SIGSEGV */
}

static void crash_abort(void)
{
    abort(); /* CRASH abort: explicit abort(), SIGABRT */
}

static void crash_assert(int value)
{
    assert(value > 0); /* CRASH assert: assertion failure, SIGABRT */
}

static int crash_div0(int divisor)
{
    volatile int numerator = 100;
    return numerator / divisor; /* CRASH div0: integer divide by zero, SIGFPE */
}

static void crash_doublefree(void)
{
    char *buf = malloc(64);
    free(buf);
    free(buf); /* CRASH doublefree: second free(), SIGABRT */
}

static void crash_smash(const char *src)
{
    char buf[16];
    strcpy(buf, src); /* CRASH smash: stack buffer overflow, SIGABRT via stack protector */
    puts(buf);
}

static void crash_rodata(void)
{
    char *s = (char *)"read-only literal";
    s[0] = 'X'; /* CRASH rodata: write to string literal, SIGSEGV */
}

static void crash_lib(void)
{
    weather_update(42); /* calls into libweather.so, crash is inside the library */
}

static void crash_gen(void)
{
    route_lookup(NULL, 3); /* crash is inside the generated route_table.c */
}

int main(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "";

    if (!strcmp(mode, "null"))            crash_null();
    else if (!strcmp(mode, "stack"))      crash_stack(0);
    else if (!strcmp(mode, "abort"))      crash_abort();
    else if (!strcmp(mode, "assert"))     crash_assert(argc - 2);
    else if (!strcmp(mode, "div0"))       crash_div0(argc - 2);
    else if (!strcmp(mode, "doublefree")) crash_doublefree();
    else if (!strcmp(mode, "smash"))      crash_smash("this string is far longer than sixteen bytes");
    else if (!strcmp(mode, "throw"))      crash_throw();
    else if (!strcmp(mode, "rodata"))     crash_rodata();
    else if (!strcmp(mode, "lib"))        crash_lib();
    else if (!strcmp(mode, "gen"))        crash_gen();
    else {
        fprintf(stderr, "usage: %s null|stack|abort|assert|div0|doublefree|smash|throw|rodata|lib\n", argv[0]);
        return 2;
    }
    return 0;
}

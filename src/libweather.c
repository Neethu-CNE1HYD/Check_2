#include "libweather.h"

static void weather_store(int *slot, int value)
{
    *slot = value; /* CRASH lib: NULL pointer write, SIGSEGV */
}

void weather_update(int station_id)
{
    int *slot = 0;
    weather_store(slot, station_id);
}

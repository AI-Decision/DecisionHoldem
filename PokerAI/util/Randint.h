#pragma once
#include <sys/time.h>
typedef unsigned int uint32;
class Randint {
public:
    uint32 prngState = 0;
    Randint() {
        //struct timeval ti;
        //gettimeofday(&ti, NULL);
        //prngState = (uint32)(ti.tv_sec + ti.tv_usec);
        prngState = rand();
    }
    Randint(uint32 seed)
    {
        //Seed the pseudo-random number generator
        prngState = seed;
    }
    void reset(uint32 seed) {
        prngState = seed;
    }
    void reset() {
        struct timeval ti;
        gettimeofday(&ti, NULL);
        prngState = (uint32)(ti.tv_sec + ti.tv_usec);
    }
    uint32 _rand()
    {
        uint32 value;
        prngState *= 1103515245;
        prngState += 12345;
        value = (prngState >> 16) & 0x07FF;
        prngState *= 1103515245;
        prngState = prngState + 12345;
        value <<= 10;
        value |= (prngState >> 16) & 0x03FF;
        prngState *= 1103515245;
        prngState += 12345;
        value <<= 10;
        value |= (prngState >> 16) & 0x03FF;
        return value;
    }
};
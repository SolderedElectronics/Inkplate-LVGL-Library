#ifndef __WAVEFROMS_INKPLATE_6_H__
#define __WAVEFROMS_INKPLATE_6_H__

#ifdef ARDUINO_INKPLATE6V2

#define WAVEFORM3BIT                                                                                                   \
    {{0, 0, 0, 0, 1, 1, 1, 1, 0}, {0, 0, 0, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 0, 2, 1, 0, 0},                            \
     {1, 1, 1, 2, 2, 1, 1, 0, 0}, {1, 1, 1, 1, 2, 2, 1, 0, 0}, {0, 1, 1, 1, 2, 2, 1, 0, 0},                            \
     {0, 0, 0, 0, 1, 1, 2, 0, 0}, {0, 0, 0, 0, 0, 0, 2, 0, 0}};


#define E_INK_WIDTH  800
#define E_INK_HEIGHT 600
#endif
#endif

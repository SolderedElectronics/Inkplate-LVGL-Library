#ifdef ARDUINO_INKPLATE5V2
#ifndef __WAVEFROMS_INKPLATE_5V2_H__
#define __WAVEFROMS_INKPLATE_5V2_H__

#define WAVEFORM3BIT                                                                                                   \
    {{0, 0, 1, 1, 2, 1, 1, 1, 0}, {1, 1, 2, 2, 1, 2, 1, 1, 0}, {0, 1, 2, 2, 1, 1, 2, 1, 0},                            \
     {0, 0, 1, 1, 1, 1, 1, 2, 0}, {1, 2, 1, 2, 1, 1, 1, 2, 0}, {0, 1, 1, 1, 2, 0, 1, 2, 0},                            \
     {1, 1, 1, 2, 2, 2, 1, 2, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}};

#ifndef E_INK_WIDTH
#define E_INK_WIDTH 1280
#endif
#ifndef E_INK_HEIGHT
#define E_INK_HEIGHT 720
#endif

#endif
#endif
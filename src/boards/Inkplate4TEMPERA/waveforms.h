#ifdef ARDUINO_INKPLATE4TEMPERA
#ifndef __WAVEFROMS_INKPLATE_4TEMPERA_H__
#define __WAVEFROMS_INKPLATE_4TEMPERA_H__

#define WAVEFORM3BIT                                                                                                   \
    {{0, 0, 1, 1, 1, 1, 1, 0}, {1, 1, 1, 2, 1, 1, 0, 0}, {2, 1, 1, 0, 2, 1, 1, 0}, {0, 0, 0, 1, 1, 1, 2, 0},           \
     {2, 1, 1, 2, 1, 1, 2, 0}, {1, 2, 1, 1, 2, 1, 2, 0}, {1, 1, 1, 2, 1, 2, 2, 0}, {0, 0, 0, 0, 0, 2, 2, 0}};

#ifndef E_INK_WIDTH
#define E_INK_WIDTH 600
#endif
#ifndef E_INK_HEIGHT
#define E_INK_HEIGHT 600
#endif

#endif
#endif
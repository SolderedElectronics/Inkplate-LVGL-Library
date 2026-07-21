#ifndef POKEMON_SPRITE_H
#define POKEMON_SPRITE_H

#include <Inkplate-LVGL.h>

// Declaration of the statically embedded Pokemon sprite.
// The definition lives in 149.c (or whichever .c file you generated with the
// LVGL image converter).  When you change POKEMON_ID in config.h, replace the
// .c file with your newly converted image and keep the variable named
// `pokemon_sprite` so that the rest of the code compiles without changes.
extern const lv_image_dsc_t pokemon_sprite;

#endif

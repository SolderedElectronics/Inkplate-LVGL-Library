/**
 **************************************************
 *
 * @file        FS_driver_implementation.h
 * @brief       LVGL filesystem driver registration for the SD card
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#pragma once

class Inkplate;

/**
 * @brief       Registers the LVGL 'S:' filesystem driver backed by SdFat.
 *
 *              Called automatically from Inkplate::begin(). Subsequent calls
 *              with a null pointer (e.g. from user sketches) are silently
 *              ignored. Double-registration is prevented by an internal guard
 *              so it is safe to call multiple times.
 *
 * @param       Inkplate *inkplate
 *              Pointer to the Inkplate instance whose SdFat volume will be
 *              used for all SD file operations. Pass nullptr to skip.
 */
void lv_fs_init_sd(Inkplate *inkplate = nullptr);

/**
 **************************************************
 *
 * @file        Inkplate-LVGL.h
 * @brief       Main header for the Inkplate LVGL library
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Josip Šimun Kuči @ Soldered
 ***************************************************/


#ifndef __INKPLATE_H__
#define __INKPLATE_H__

#include "Arduino.h"
#include "SPI.h"
#include "Wire.h"
#include "lvgl/custom_allocation_algorithm.h"
#ifndef ARDUINO_INKPLATE2
#include "lvgl/FS_driver_implementation.h"
#endif
#include "boardSelect.h"
#include "graphics/GraphicsDefs.h"
#include "system/InkplateBoards.h"
#include "system/NetworkController/NetworkController.h"
#include "system/defines.h"
#include "system/inkplateSemaphore.h"


void display_flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

class Inkplate : public InkplateBoardClass, public NetworkController
{
  public:
#ifndef USE_COLOR_IMAGE
    Inkplate(uint8_t mode);
#else
    Inkplate();
#endif
    void begin(lv_display_render_mode_t renderMode = LV_DISP_RENDER_MODE_FULL);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void setRotation(uint8_t r);
    void enableDithering(bool state);
    uint8_t getRotation();
    lv_display_t *disp;
    bool ditherEnabled = false;
    lv_display_render_mode_t _renderMode;


  protected:
  private:
    uint8_t _rotation = 0;
    uint8_t _width = 0;
    uint8_t _height = 0;
    uint8_t _beginDone = 0;
    uint8_t _mode;
    void initLVGL(lv_display_render_mode_t renderMode);
};
#endif
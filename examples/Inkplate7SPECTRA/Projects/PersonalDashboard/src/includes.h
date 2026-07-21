#ifndef INCLUDES_H
#define INCLUDES_H

#include <Inkplate-LVGL.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include "config.h"
#include "Network.h"
#include "Gui.h"

// Global instances (defined in PersonalDashboard.ino)
extern Inkplate inkplate;
extern DataFetcher  network;
extern Gui      gui;

#endif

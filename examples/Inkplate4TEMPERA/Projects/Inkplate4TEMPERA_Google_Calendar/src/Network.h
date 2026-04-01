#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "calendarData.h"

class NetworkFunctions {
public:
    NetworkFunctions(const String& calendarID, const String& apiKey);
    
    void connectWiFi();
    bool fetchCalendar(calendarData* data);

private:
    String calendarID;
    String apiKey;
};

#endif
#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <HTTPClient.h>
#include "WeatherData.h"
#include <Arduino.h>

class NetworkFunctions {
public:
    struct UserInfo {
        String lastUpdated;
        int currentHour;
        String city;
        String username;
        String lastUpdatedDate;
        String lastUpdatedTime;
        bool apiError;
        bool useMetric;
        String temperatureLabel;
        String speedLabel;
    };

    void fetchWeatherData(WeatherData* weatherData, UserInfo* userInfo, const float* latitude, const float* longitude);

private:
    String getWeatherDescription(int code);
    String extractDate(String dateTime);
    String extractTime(String dateTime);
    String extractSun(String dateTime);
    String getFormattedTime();
    int getCurrentHour();
    String getDayName(int dayIndex);
};

#endif
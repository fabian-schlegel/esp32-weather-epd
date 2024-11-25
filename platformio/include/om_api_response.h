//
// Created by fabian on 22.01.24.
//

#ifndef PLATFORMIO_OM_API_RESPONSE_H
#define PLATFORMIO_OM_API_RESPONSE_H

#include <cstdint>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "config.h"

#define OM_NUM_HOURLY        24
#define OM_NUM_DAILY          7

#ifdef USE_HTTP
#define OM_PORT              80
#else
#define OM_PORT              443
#endif

/*
 * Current weather data API response
 * https://api.open-meteo.com/v1/dwd-icon?latitude=52.52&longitude=13.41&current=is_day,temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m&hourly=temperature_2m
 */
typedef struct om_current
{
    time_t  dt;                   // epoch seconds
    float   temperature_2m;       // °C
    int     relative_humidity_2m; // %
    float   apparent_temperature; // °C
    float   precipitation;        // mm
    int     weather_code;         // wmo code
    int     cloud_cover;          // %
    float   surface_pressure;     // hPa
    float   wind_speed_10m;       // km/h
    int     wind_direction_10m;   // °
    bool    is_day;               // true, false
} om_current_t;

/*
 * Hourly forecast weather data API response
 */
typedef struct om_hourly
{
    time_t  dt;                   // epoch seconds
    float   temperature_2m;       // °C
    float   precipitation;        // mm
    int     weather_code;         // wmo code
} om_hourly_t;

/*
 * Daily forecast weather data API response
 * https://api.open-meteo.com/v1/dwd-icon?latitude=52.52&longitude=13.41&daily=weather_code,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,daylight_duration,sunshine_duration,precipitation_sum,precipitation_probability_max,wind_speed_10m_max,wind_direction_10m_dominant
 */
typedef struct om_daily
{
    int      weather_code;                // wmo code
    float    temperature_2m_max;          // °C
    float    temperature_2m_min;          // °C
    time_t   sunrise;                     // epoch seconds
    time_t   sunset;                      // epoch seconds
    float    daylight_duration;           // s
    float    sunshine_duration;           // s
    float    precipitation_sum;           // mm
    float    precipitation_hours;         // h
    float    wind_speed_10m_max;          // km/h
    int      wind_direction_10m_dominant; // °
} om_daily_t;

/*
 * National weather alerts data from major national weather warning systems
 */
typedef struct om_alerts
{
    String  sender_name;      // Name of the alert source.
    String  event;            // Alert event name
    int64_t start;            // Date and time of the start of the alert, Unix, UTC
    int64_t end;              // Date and time of the end of the alert, Unix, UTC
    String  description;      // Description of the alert
    String  tags;             // Type of severe weather
} om_alerts_t;

/*
 * Response from Open Meto DWD API
 * https://api.open-meteo.com/v1/dwd-icon?latitude=52.52&longitude=13.41&current=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m&hourly=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,weather_code,surface_pressure,cloud_cover,wind_speed_10m,wind_direction_10m&daily=weather_code,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,daylight_duration,sunshine_duration,precipitation_sum,precipitation_probability_max,wind_speed_10m_max,wind_direction_10m_dominant
 */
typedef struct om_resp
{
    om_current_t   current;
    om_hourly_t    hourly[OM_NUM_HOURLY];
    om_daily_t     daily[OM_NUM_DAILY];
} om_resp_t;

DeserializationError deserializeOpenMeteo(String json,
                                        om_resp_t &r);

#ifdef USE_HTTP
int getOM(WiFiClient &client, om_resp_t &r);
#else
int getOM(WiFiClientSecure &client, om_resp_t &r);
#endif

#endif //PLATFORMIO_OM_API_RESPONSE_H

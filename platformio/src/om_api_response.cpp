#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "om_api_response.h"

#include <config.h>
#include <HTTPClient.h>

const String OM_ENDPOINT = "api.open-meteo.com";

#ifdef USE_HTTP
int getOM(WiFiClient &client, om_resp_t &r)
#else
int getOM(WiFiClientSecure &client, om_resp_t &r)
#endif
{
    int attempts = 0;
    bool rxSuccess = false;
    DeserializationError jsonErr = {};
    const String uri = "/v1/dwd-icon?latitude=" + LAT + "&longitude=" + LON
                       + "&current=is_day,temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m&hourly=temperature_2m,precipitation,weather_code,&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,daylight_duration,sunshine_duration,precipitation_sum,precipitation_hours,wind_speed_10m_max,wind_direction_10m_dominant&forecast_days=5&forecast_hours=24&timezone=Europe%2FBerlin";

    String sanitizedUri = OM_ENDPOINT + uri;

    Serial.print("Attempting HTTP request");
    Serial.println(": " + sanitizedUri);

    int httpResponse = 0;
    while (!rxSuccess && attempts < 3)
    {
        wl_status_t connection_status = WiFi.status();
        if (connection_status != WL_CONNECTED)
        {
            // -512 offset distinguishes these errors from httpClient errors
            return -512 - static_cast<int>(connection_status);
        }

        HTTPClient http;
        http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 5000ms
        http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 5000ms
        http.begin(client, OM_ENDPOINT, OM_PORT, uri);
        httpResponse = http.GET();
        if (httpResponse == HTTP_CODE_OK)
        {
            jsonErr = deserializeOpenMeteo(http.getString(), r);
            if (jsonErr)
            {
                // -256 offset distinguishes these errors from httpClient errors
                httpResponse = -256 - static_cast<int>(jsonErr.code());
            }
            rxSuccess = !jsonErr;
        }
        client.stop();
        http.end();
        Serial.println("  " + String(httpResponse, DEC) + " ");
        ++attempts;
    }

    return httpResponse;
}

DeserializationError deserializeOpenMeteo(String json,
                                        om_resp_t &r)
{
    int i;

    Serial.println(json);
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, json);

#if DEBUG_LEVEL >= 1
    Serial.println("[debug] doc.overflowed() : " + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
    serializeJsonPretty(doc, Serial);
#endif

    if (error) {
        return error;
    }

    const JsonObject current = doc["current"];

    r.current.temperature_2m       = current["temperature_2m"]      .as<float>();
    r.current.apparent_temperature = current["apparent_temperature"].as<float>();
    r.current.surface_pressure     = current["surface_pressure"]    .as<float>();
    r.current.relative_humidity_2m = current["relative_humidity_2m"].as<int>();
    r.current.cloud_cover          = current["cloud_cover"]         .as<int>();
    r.current.wind_speed_10m       = current["wind_speed_10m"]      .as<float>();
    r.current.wind_direction_10m   = current["wind_direction_10m"]  .as<int>();
    r.current.precipitation        = current["precipitation"]       .as<float>();
    r.current.weather_code         = current["weather_code"]        .as<int>();
    r.current.is_day               = current["is_day"]              .as<int>();

    for (i = 0; i < OM_NUM_HOURLY; i++)
    {
        struct tm tm = {0};
        if (strptime(doc["hourly"]["time"][i].as<const char *>(), "%Y-%m-%dT%H:%M", &tm) != nullptr) {
            r.hourly[i].dt = mktime(&tm);
        }
        r.hourly[i].temperature_2m       = doc["hourly"]["temperature_2m"][i]      .as<float>();
        r.hourly[i].precipitation        = doc["hourly"]["precipitation"][i]       .as<float>();
        r.hourly[i].weather_code         = doc["hourly"]["weather_code"][i]        .as<int>();
    }

    for (i = 0; i < OM_NUM_DAILY; i++)
    {
        r.daily[i].temperature_2m_min          = doc["daily"]["temperature_2m_min"][i]         .as<float>();
        r.daily[i].temperature_2m_max          = doc["daily"]["temperature_2m_max"][i]         .as<float>();
        r.daily[i].sunshine_duration           = doc["daily"]["sunshine_duration"][i]          .as<float>();
        r.daily[i].daylight_duration           = doc["daily"]["daylight_duration"][i]          .as<float>();
        r.daily[i].precipitation_sum           = doc["daily"]["precipitation_sum"][i]          .as<float>();
        r.daily[i].precipitation_hours         = doc["daily"]["precipitation_hours"][i]        .as<float>();
        r.daily[i].wind_speed_10m_max          = doc["daily"]["wind_speed_10m_max"][i]         .as<float>();
        r.daily[i].wind_direction_10m_dominant = doc["daily"]["wind_direction_10m_dominant"][i].as<int>();
        r.daily[i].weather_code                = doc["daily"]["weather_code"][i]               .as<int>();
        printf("[%d] SD: %f, DD: %f, PH: %f\n", i, r.daily[i].sunshine_duration, r.daily[i].daylight_duration, r.daily[i].precipitation_hours);
    }
    return error;
} // end deserializeOpenMeteo

#ifndef UNLEASH_h

#define UNLEASH_h

#define DEBUGPRINTFV Serial.printf
#define DEBUGPRINTFE Serial.printf
#define UNLEASHJSON_SIZE 2048

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "time.h"

class Unleash {
   public:
    // connects to the Unleash endpoint, registering the application with given
    // application name and instance id
    // server address must be a http(s)://example.com, without trailing "/"
    // if updateLocalTime is true, device local time will be updated based on
    // the time got from the NTP server with timezone set to UTC
    void begin(const char* server, const char* appName, const char* instanceId, bool updateLocalTime = true);

    // checks if the provided feature flag is enabled or returns default value
    // if there is no possibility to say (e.g. strategy is unsupported)
    bool isEnabled(const char* featureName, bool value = false);

    // periodically updates the feature list from the server
    // must be called in your sketch in the loop()
    void loop();

    // sets the polling interval (in milliseconds) for getting features from the server
    // the default is 15 seconds
    void setPollingInterval(long interval);

   private:
    String server;
    StaticJsonDocument<UNLEASHJSON_SIZE> serverFeatures;
    HTTPClient http;
    void fetchFeatures();
    long lastFeatureFetch;
    long pollingInterval = 15000;
};

void Unleash::begin(const char* server, const char* appName, const char* instanceId, bool updateLocalTime) {
    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 0;
    const int daylightOffset_sec = 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        DEBUGPRINTFE("Failed to obtain time!\n");
        return;
    }

    this->server = String(server);

    char startTime[20];
    strftime(startTime, 20, "%Y-%m-%dT%H:%M:%S.%f%z", &timeinfo);

    StaticJsonDocument<256> doc;

    doc["appName"] = appName;
    doc["instanceId"] = instanceId;
    doc["sdkVersion"] = "arduino-unleash:0.1.0";

    JsonArray strategies = doc.createNestedArray("strategies");
    strategies.add("default");

    doc["started"] = String(startTime);
    doc["interval"] = 10000;

    String output;
    serializeJson(doc, output);

    // Your Domain name with URL path or IP address with path
    http.begin(this->server + "/api/client/register");

    // If you need an HTTP request with a content type: application/json, use the following:
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(output);

    DEBUGPRINTFV("Trying to register client, response: %d\n", httpResponseCode);
    if (httpResponseCode != 202) {
        DEBUGPRINTFE("Failed to register client!\n");
        return;
    }

    this->fetchFeatures();
}

void Unleash::fetchFeatures() {
    http.begin(this->server + "/api/client/features");
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0) {
        DEBUGPRINTFV("Got list of features from the server.\n");
        payload = http.getString();
    } else {
        DEBUGPRINTFE("Error getting feature list: %d!\n", httpResponseCode);
    }
    http.end();

    DeserializationError error = deserializeJson(serverFeatures, payload);

    if (error) {
        DEBUGPRINTFE("deserializeJson() failed: %s", error.f_str());        
        return;
    }

    lastFeatureFetch = millis();
}

bool Unleash::isEnabled(const char* featureName, bool value) {
    if (serverFeatures.isNull())
        this->fetchFeatures();

    JsonArray features = serverFeatures["features"];
    for (size_t i = 0; i < features.size(); i++) {
        JsonObject feat = serverFeatures["features"][i];

        String name = feat["name"];
        bool enabled = feat["enabled"];

        JsonArray strategies = feat["strategies"];
        if (name == String(featureName)) {
            if (!enabled) {
                return false;
            } else {
                for (size_t j = 0; j < strategies.size(); j++) {
                    String name = strategies[j]["name"];

                    if (name == "default") {
                        return true;
                    } else {
                        DEBUGPRINTFV("Unsupported strategy %s for %s toggle, returning default.\n", name.c_str(), featureName);
                        return value;
                    }
                }
            }
        }
    }

    return value;
}

void Unleash::loop() {
    if (millis() - lastFeatureFetch > this->pollingInterval) {
        this->fetchFeatures();
    }
}

void Unleash::setPollingInterval(long interval) {
    this->pollingInterval = interval;
}

#endif